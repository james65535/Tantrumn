// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "TantrumnLevelEndTrigger.generated.h"

class ATantrumnGameModeBase;
class ATantrumnGameStateBase;

UCLASS(Blueprintable, BlueprintType)
class TANTRUMN_API ATantrumnLevelEndTrigger : public ATriggerVolume
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

private:

	// Custom Overlap function to ovveride the actor BeginOverLap
	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	UPROPERTY()
	ATantrumnGameModeBase* GameModeRef;
	UPROPERTY()
	ATantrumnGameStateBase* TantrumnGameState;
	
};
