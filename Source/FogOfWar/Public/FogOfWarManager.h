#pragma once

#include "GameFramework/Actor.h"
#include "FogOfWarWorker.h"
#include "FogOfWarManager.generated.h"

/**
*
*/
class UMaterialInstanceDynamic;
class UPostProcessComponent;
class UFogOfWarComponent;

UCLASS()
class FOGOFWAR_API AFogOfWarManager : public AActor
{
	GENERATED_BODY()
		AFogOfWarManager(const FObjectInitializer & FOI);
	virtual ~AFogOfWarManager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform & Transform) override;
protected:
	void OnFowTextureUpdated(UTexture2D* currentTexture, UTexture2D* lastTexture);
public:
	//Register an actor to influence the FOW-texture
	void RegisterFowActor(AActor* Actor);

	//Stolen from https://wiki.unrealengine.com/Dynamic_Textures
	void UpdateTextureRegions(
		UTexture2D* Texture,
		int32 MipIndex,
		uint32 NumRegions,
		FUpdateTextureRegion2D* Regions,
		uint32 SrcPitch,
		uint32 SrcBpp,
		uint8* SrcData,
		bool bFreeData);

	//How far will an actor be able to see
	//CONSIDER: Place it on the actors to allow for individual sight-radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SightRange = 9.0f;

	//The number of samples per 100 unreal units
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SamplesPerMeter = 2.0f;

	//If the last texture blending is done
	UPROPERTY(BlueprintReadWrite)
	bool bIsDoneBlending;

	//Should we blur? It takes up quite a lot of CPU time...
	UPROPERTY(EditAnywhere)
	bool bIsBlurEnabled = true;

	//The size of our textures
	uint32 TextureSize = 1024;

	//Array containing what parts of the map we've unveiled.
	UPROPERTY()
	TArray<bool> UnfoggedData;

	//Temp array for horizontal blur pass
	UPROPERTY()
	TArray<uint8> HorizontalBlurData;

	//Our texture data (result of vertical blur pass)
	UPROPERTY()
	TArray<FColor> TextureData;

	//Our texture data from the last frame
	UPROPERTY()
	TArray<FColor> LastFrameTextureData;

	//Check to see if we have a new FOW-texture.
	bool bHasFOWTextureUpdate = false;

	//Blur size
	uint8 blurKernelSize = 15;

	//Blur kernel
	UPROPERTY()
	TArray<float> blurKernel;
	/* Color of unexplored areas. Fog transparancy is set with the alpha channel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor FogColor;
	/* Color of areas that are explored, but that are currently unseen. Only used if blurring is disabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 ShroudOpacity;

protected:
	UPROPERTY()
	TArray<UFogOfWarComponent *> FowComponents;
	FCriticalSection FowComponents_mutex;
public:
	/* register a component for FOW-texture calculations */
	UFUNCTION(BlueprintCallable)
	void RegisterComponent(UFogOfWarComponent *Comp);
	/* de register a component for FOW-texture calculations */
	UFUNCTION(BlueprintCallable)
	void DeRegisterComponent(UFogOfWarComponent *Comp);
	/* Get an array with the currently registerd components */
	UFUNCTION(BlueprintCallable)
	void GetFowComponents(TArray<UFogOfWarComponent *> &OutFowComponents);

	//DEBUG: Time it took to update the fow texture
	UPROPERTY(VisibleAnywhere)
	float fowUpdateTime = 0;

	//Getter for the working thread
	bool GetIsBlurEnabled();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UPostProcessComponent *PostProcessComponent;

	UPROPERTY(BlueprintReadWrite)
	UMaterialInstanceDynamic *FOWMaterial;

protected:
	void UpdateFowTexture();

	UPROPERTY(BlueprintReadWrite)
	float FOWTextureBlend;

	//Triggers the start of a new FOW-texture-update
	void StartFOWTextureUpdate();

	//Our dynamically updated texture
	UPROPERTY()
		UTexture2D* FOWTexture;

	//Texture from last update. We blend between the two to do a smooth unveiling of newly discovered areas.
	UPROPERTY()
		UTexture2D* LastFOWTexture;

	//Texture regions
	FUpdateTextureRegion2D* textureRegions;

	//Our fowupdatethread
	AFogOfWarWorker* FowThread;
};
