// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FogOfWarManager.h"

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FogOfWarComponent.generated.h"

/*
* Handles registration of actors with the FOWManager as well as keeping track of individual FOW settings.
*
* The component location is used as the starting point for LOS test
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FOGOFWAR_API UFogOfWarComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	// Sets default values for this component's properties
	UFogOfWarComponent();

	/* Should this component register itself with the FOWManager at game start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOW")
	bool bRegisterAtBeginPlay;

	/* Tell the manager to include this component in the FOW updates */
	UFUNCTION(BlueprintCallable, Category = "FOW")
	void RegisterFOW() { Manager->RegisterComponent(this); }
	/* Tell the manager ignore this component in the FOW updates */
	UFUNCTION(BlueprintCallable, Category = "FOW")
	void DeRegisterFOW() { Manager->DeRegisterComponent(this); }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	AFogOfWarManager *Manager;
};
