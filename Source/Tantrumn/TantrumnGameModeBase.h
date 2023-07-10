// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TantrumnGameWidget.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumnGameModeBase.generated.h"

class ATantrumnPlayerController;
class UTantrumnGameWidget;

/*
 *  Note Gamemode is not replicated
 */

UCLASS()
class TANTRUMN_API ATantrumnGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	
	// --- Overrides --- //
	ATantrumnGameModeBase();
	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	/** Primary Widget Class to be displayed for this game mode */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	TSubclassOf<UTantrumnGameWidget> GetWidgetClass() const { return GameWidgetClass; }

	/** Primary Widget Class to be displayed for this game mode */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	TSubclassOf<UTantrumnGameWidget> GetGameMenuWidgetClass() const { return GameMenuWidgetClass; }

	/** Check if Game is In Start Menu */
	UFUNCTION(BlueprintPure, Category = "Tantrumn")
	bool IsInStartMenu() const { return bToggleInitialMainMenu; }

	void RestartGame();

private:

	// --- VARS --- //

	/** Is this gamemode used for the initial main menu screen */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn")
	bool bToggleInitialMainMenu = true;
	
	/** Countdown before the gameplay state begins.  Exposed for BPs to change in editor */
	UPROPERTY(EditAnywhere, Category = "Tantrumn")
	float GameCountDownDuration = 3.0f;

	/** Widget to display for this Game Mode */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn")
	TSubclassOf<UTantrumnGameWidget> GameWidgetClass;

	/** Widget to display for Game Menu */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn")
	TSubclassOf<UTantrumnGameWidget> GameMenuWidgetClass;

	/** Allows gamemode to determine if a single or multiplayer game is intended */
	UPROPERTY(EditAnywhere, Category = "Tantrumn")
	uint8 NumExpectedPlayers = 1u;

	float DelayStartDuration = 0.5f;
	FTimerHandle DelayStartTimerHandle;
	FTimerHandle CountdownTimerHandle;
	
	// --- FUNCTIONS --- //
	
	/** Setter to specify if a single or multiplayer game is intended */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void SetNumExpectedPlayers(uint8 InNumExpectedPlayers) { NumExpectedPlayers = InNumExpectedPlayers; }
	
	void AttemptStartGame();
	void DisplayCountDown();
	void StartGame();
	
};
