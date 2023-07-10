// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TantrumnGameWidget.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUMN_API UTantrumnGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void InitiateGameTimer(float CountDownTime, class ATantrumnPlayerController* TantrumnPlayerController);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void LevelComplete();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void DisplayResults();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void RemoveResults();
};
