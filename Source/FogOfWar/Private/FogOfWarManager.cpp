// Fill out your copyright notice in the Description page of Project Settings.

#include "FogOfWarManager.h"

#include "Components/PostProcessComponent.h"
#include "ConstructorHelpers.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"


AFogOfWarManager::AFogOfWarManager(const FObjectInitializer &FOI) : Super(FOI) {
	PrimaryActorTick.bCanEverTick = true;
	textureRegions = new FUpdateTextureRegion2D(0, 0, 0, 0, TextureSize, TextureSize);

	//15 Gaussian samples. Sigma is 2.0.
	//CONSIDER: Calculate the kernel instead, more flexibility...
	blurKernel.Init(0.0f, blurKernelSize);
	blurKernel[0] = 0.000489f;
	blurKernel[1] = 0.002403f;
	blurKernel[2] = 0.009246f;
	blurKernel[3] = 0.02784f;
	blurKernel[4] = 0.065602f;
	blurKernel[5] = 0.120999f;
	blurKernel[6] = 0.174697f;
	blurKernel[7] = 0.197448f;
	blurKernel[8] = 0.174697f;
	blurKernel[9] = 0.120999f;
	blurKernel[10] = 0.065602f;
	blurKernel[11] = 0.02784f;
	blurKernel[12] = 0.009246f;
	blurKernel[13] = 0.002403f;
	blurKernel[14] = 0.000489f;

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>("Post-process Component");

	FOWTextureBlend = 0;
}

AFogOfWarManager::~AFogOfWarManager() {
	if (FowThread) {
		FowThread->ShutDown();
	}
}

void AFogOfWarManager::BeginPlay() {
	Super::BeginPlay();
	bIsDoneBlending = true;
	AFogOfWarManager::StartFOWTextureUpdate();
}

void AFogOfWarManager::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (FOWTexture && LastFOWTexture && bHasFOWTextureUpdate && bIsDoneBlending) {
		LastFOWTexture->UpdateResource();
		UpdateTextureRegions(LastFOWTexture, (int32)0, (uint32)1, textureRegions, (uint32)(4 * TextureSize), (uint32)4, (uint8*)LastFrameTextureData.GetData(), false);
		FOWTexture->UpdateResource();
		UpdateTextureRegions(FOWTexture, (int32)0, (uint32)1, textureRegions, (uint32)(4 * TextureSize), (uint32)4, (uint8*)TextureData.GetData(), false);
		bHasFOWTextureUpdate = false;
		bIsDoneBlending = false;
		//Trigger the blueprint update
		OnFowTextureUpdated(FOWTexture, LastFOWTexture);
	}

	// Set blending factor between FOW-textures
	if (FOWTextureBlend < 1)
	{
		FOWTextureBlend = FMath::Clamp(FOWTextureBlend * 10 + DeltaSeconds, 0.0f, 1.0f);
		FOWMaterial->SetScalarParameterValue(FName("Blend"), FOWTextureBlend);
	}
	else if (!bIsDoneBlending)
	{
		bIsDoneBlending = true;
	}

}

void AFogOfWarManager::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	// set up the FOW material
	UMaterial *BaseMaterial = LoadObject<UMaterial>(this, TEXT("/FogOfWar/Materials/PP_Fow_Mat.PP_Fow_Mat"));
	check(BaseMaterial);
	FOWMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	PostProcessComponent->AddOrUpdateBlendable(FOWMaterial, 1);
}

void AFogOfWarManager::StartFOWTextureUpdate() {
	if (!FOWTexture) {
		FOWTexture = UTexture2D::CreateTransient(TextureSize, TextureSize);
		LastFOWTexture = UTexture2D::CreateTransient(TextureSize, TextureSize);
		int arraySize = TextureSize * TextureSize;
		TextureData.Init(FColor(0, 0, 0, 255), arraySize);
		LastFrameTextureData.Init(FColor(0, 0, 0, 255), arraySize);
		HorizontalBlurData.Init(0, arraySize);
		UnfoggedData.Init(false, arraySize);
		FowThread = new AFogOfWarWorker(this);
	}
}

void AFogOfWarManager::OnFowTextureUpdated(UTexture2D* currentTexture, UTexture2D* lastTexture) {
	FOWMaterial->SetTextureParameterValue(FName("FOWTexture"), currentTexture);
	FOWMaterial->SetTextureParameterValue(FName("LastFOWTexture"), lastTexture);
	FOWMaterial->SetScalarParameterValue(FName("Blend"), 0.0f);
	FOWTextureBlend = 0;
}

void AFogOfWarManager::RegisterComponent(UFogOfWarComponent *Comp)
{
	// make sure to lock as this array is accessed both from the game thread and the FOWWorker thread
	FowComponents_mutex.Lock();
	FowComponents.AddUnique(Comp);
	FowComponents_mutex.Unlock();
}

void AFogOfWarManager::DeRegisterComponent(UFogOfWarComponent *Comp)
{
	// make sure to lock as this array is accessed both from the game thread and the FOWWorker thread
	FowComponents_mutex.Lock();
	FowComponents.Remove(Comp);
	FowComponents_mutex.Unlock();
}

void AFogOfWarManager::GetFowComponents(TArray<UFogOfWarComponent *> &OutFowComponents)
{
	// make sure to lock as this array is accessed both from the game thread and the FOWWorker thread
	FowComponents_mutex.Lock();
	OutFowComponents = FowComponents;
	FowComponents_mutex.Unlock();
}

bool AFogOfWarManager::GetIsBlurEnabled() {
	return bIsBlurEnabled;
}

void AFogOfWarManager::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture && Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
	}
}
