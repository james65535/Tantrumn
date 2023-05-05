// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TantrumnGameInstance.generated.h"

class ATantrumnPlayerController;
class ATantrumnGameStateBase;
class UTantrumnGameWidget;

/*
 * Contain game information which should exist globally.  Each client has a GameStateInstance so is useful for HUD
 * and related items which aren't handled by PlayerController.  GameInstance is not a replicated class.
 * Note: A game only has one GameInstance so this will not work for local splitscreen games, only one screen gets the HUD
 */
UCLASS()
class TANTRUMN_API UTantrumnGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "TantrumnRace")
	bool IsPlayableGame = false;
	
	void DisplayCountDown(float GameCountDownDuration, ATantrumnPlayerController* TantrumnPlayerController);
	void DisplayLevelComplete(ATantrumnPlayerController* TantrumnPlayerController);

	// TODO This is never used
	UFUNCTION(BlueprintPure)
	ATantrumnGameStateBase* GetGameState() const { return TantrumnGameStateBase; }

	UFUNCTION(BlueprintCallable)
	void OnRetrySelected(ATantrumnPlayerController* TantrumnPlayerController);

	UFUNCTION()
	void RestartGame(ATantrumnPlayerController* TantrumnPlayerController);

protected:

	UFUNCTION()
	void OnGameStateSet(AGameStateBase* GameStateBase);
	
private:

	// Exposed Class to check the type of widget to display
	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnGameWidget> GameWidgetClass;

	UPROPERTY()
	TMap<APlayerController*, UTantrumnGameWidget*> GameWidgets;

	UPROPERTY()
	ATantrumnGameStateBase* TantrumnGameStateBase;
	
};
