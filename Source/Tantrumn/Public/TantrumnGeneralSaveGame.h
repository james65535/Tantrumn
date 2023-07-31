// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TantrumnGeneralSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUMN_API UTantrumnGeneralSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UTantrumnGeneralSaveGame();

	/** Player Details to Save */
	UPROPERTY(VisibleAnywhere, Category = "Tantrumn SaveInfo")
	FString TantrumnPlayerName;

	/** Pre-reqs for Saving */
	UPROPERTY(VisibleAnywhere, Category = "Tantrumn SaveInfo")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "Tantrumn SaveInfo")
	uint32 UserIndex;
	
};
