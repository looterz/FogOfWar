// Fill out your copyright notice in the Description page of Project Settings.

#include "FogOfWarComponent.h"

#include "EngineUtils.h"

// Sets default values for this component's properties
UFogOfWarComponent::UFogOfWarComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bRegisterAtBeginPlay = true;
}


// Called when the game starts
void UFogOfWarComponent::BeginPlay()
{
	Super::BeginPlay();

	// grab the first FOWManager we can find in the world
	for (TActorIterator<AFogOfWarManager> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		Manager = *ActorItr;
		break;
	}
	check(Manager);
	if (bRegisterAtBeginPlay)
	{
		RegisterFOW();
	}
}
