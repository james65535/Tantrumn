// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TantrumnGameWidget.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumnGameModeBase.generated.h"

class ATantrumnPlayerController;
class UTantrumnGameWidget;

// Enum to track the current state of the game
UENUM(BlueprintType)
enum class ETantrumnGameState : uint8
{
	NONE		UMETA(DisplayName = "NONE"),
	WAITING		UMETA(DisplayName = "Waiting"),
	PLAYING		UMETA(DisplayName = "Playing"),
	PAUSED		UMETA(DisplayName = "Paused"),
	GAMEOVER	UMETA(DisplayName = "GameOver"),
};

UCLASS()
class TANTRUMN_API ATantrumnGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	// --- FUNCTIONS --- //
	
	ATantrumnGameModeBase();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	ETantrumnGameState GetCurrentGameState() const;
	void PLayerReachedEnd();

private:

	// --- VARS --- //

	// Create and sset CurrentGameState to NONE. This will be tracked in the code file
	UPROPERTY(VisibleAnywhere, Category = "States")
	ETantrumnGameState CurrentGameState = ETantrumnGameState::NONE;
	// Countdown before the gameplay state begins.  Exposed for BPs to change in editor
	UPROPERTY(EditAnywhere, Category = "Game Details")
	float GameCountDownDuration = 4.0f;

	FTimerHandle CountdownTimerHandle;

	// Game UI Widget
	UPROPERTY()
	UTantrumnGameWidget* GameWidget;
	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnGameWidget> GameWidgetClass;

	ATantrumnPlayerController* PC = nullptr;

	// --- FUNCTIONS --- //

	void DisplayCountdown();
	void StartGame();
	
};
