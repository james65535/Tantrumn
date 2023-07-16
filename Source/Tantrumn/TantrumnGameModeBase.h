// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TantrumnGameStateBase.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumnGameModeBase.generated.h"

enum class EPlayerGameState : uint8;
class ATantrumnPlayerController;
class UTantrumnGameWidget;

/*
 *  Note Game Mode is not replicated
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

	/** Check if Game is In Start Menu */
	UFUNCTION(BlueprintPure, Category = "Tantrumn")
	bool IsInStartMenu() const { return bToggleInitialMainMenu; }

	void RestartGame();

private:

	// TODO this can be dropped in favour of DesiredGameType
	/** Is this game mode used for the initial main menu screen */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn")
	bool bToggleInitialMainMenu = true;

	/** Determine what type of game this mode should communicate to game state and downstream clients */
	UPROPERTY(EditDefaultsOnly, NoClear, Category = "Tantrumn")
	ETantrumnGameType DesiredGameType = ETantrumnGameType::None;

	/** Used to offset start to avoid race conditions as game loads up */
	float DelayStartDuration = 0.5f;
	FTimerHandle DelayStartTimerHandle;

	/** Game Mode will poll for all players ready and if not then try again in a specified time */
	float MatchTryStartWaitDuration = 2.0f;
	FTimerHandle MatchTryStartTimerHandle;
	
	/** Countdown before the gameplay state begins.  Exposed for BPs to change in editor */
	UPROPERTY(EditAnywhere, Category = "Tantrumn")
	int32 GameCountDownDuration = 1;
	FTimerHandle CountdownTimerHandle;
	
	/** Allows Game Mode to determine if a single or multiplayer game is intended */
	UPROPERTY(EditAnywhere, Category = "Tantrumn")
	int32 NumExpectedPlayers = 1;
	/** Setter to specify if a single or multiplayer game is intended */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void SetNumExpectedPlayers(const int32 InNumExpectedPlayers) { NumExpectedPlayers = InNumExpectedPlayers; }
	
	void AttemptStartGame();
	void DisplayCountDown();
	void StartGame();

	bool CheckAllPlayersStatus(const EPlayerGameState StateToCheck) const;
};
