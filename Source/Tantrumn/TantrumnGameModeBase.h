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
	None		UMETA(DisplayName = "None"),
	Waiting		UMETA(DisplayName = "Waiting"),
	Playing		UMETA(DisplayName = "Playing"),
	Paused		UMETA(DisplayName = "Paused"),
	Gameover	UMETA(DisplayName = "GameOver"),
};

UCLASS()
class TANTRUMN_API ATantrumnGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	// --- FUNCTIONS --- //
	ATantrumnGameModeBase();
	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	ETantrumnGameState GetCurrentGameState() const;
	UFUNCTION(BlueprintCallable)
	void AttemptStartGame();
	void PLayerReachedEnd(APlayerController* PlayerController);
	
	void ReceivePlayer(APlayerController* PlayerController);

private:

	// --- VARS --- //

	// Create and sset CurrentGameState to NONE. This will be tracked in the code file
	UPROPERTY(VisibleAnywhere, Category = "States")
	ETantrumnGameState CurrentGameState = ETantrumnGameState::None;
	// Countdown before the gameplay state begins.  Exposed for BPs to change in editor
	UPROPERTY(EditAnywhere, Category = "Game Details")
	float GameCountDownDuration = 4.0f;

	FTimerHandle CountdownTimerHandle;

	// Local player split screen
	UFUNCTION(BlueprintCallable, Category = "Game Details")
	void SetNumExpectedPlayers(uint8 InNumExpectedPlayers) { NumExpectedPlayers = InNumExpectedPlayers; }
	UPROPERTY(EditAnywhere, Category = "Game Details")
	uint8 NumExpectedPlayers = 1u;

	// Game UI Widget
	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnGameWidget> GameWidgetClass;
	UPROPERTY()
	TMap<APlayerController*, UTantrumnGameWidget*> GameWidgets; // Use map instead of array due to >1 players

	ATantrumnPlayerController* PC = nullptr;

	// --- FUNCTIONS --- //
	void DisplayCountdown();
	void StartGame();
	
};
