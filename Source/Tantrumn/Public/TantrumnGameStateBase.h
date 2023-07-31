// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TantrumnGameStateBase.generated.h"

class ATantrumnPlayerState;
class UTantrumnGameWidget;
class ATantrumnCharacterBase;

// ENUM to track the current state of the game
UENUM(BlueprintType)
enum class ETantrumnGameState : uint8
{
	None		UMETA(DisplayName = "None"),
	Waiting		UMETA(DisplayName = "Waiting"),
	Playing		UMETA(DisplayName = "Playing"),
	Paused		UMETA(DisplayName = "Paused"),
	GameOver	UMETA(DisplayName = "GameOver"),
};

// ENUM to track the current type of game
UENUM(BlueprintType)
enum class ETantrumnGameType : uint8
{
	None		UMETA(DisplayName = "None"),
	Start		UMETA(DisplayName = "Start"),
	Racing		UMETA(DisplayName = "Racing"),
	DodgeBall		UMETA(DisplayName = "DodgeBall"),
	Battle		UMETA(DisplayName = "Battle"),
};

USTRUCT(BlueprintType)
struct FGameResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString Name = "Null";

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float Time = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bIsWinner = false;
};

/** Begin Delegates */

/** Notify listeners such as player controller that game type has changed */
DECLARE_MULTICAST_DELEGATE_OneParam(FGameTypeUpdateDelegate, ETantrumnGameType);
/** Notify listeners match has started with the match start time */
DECLARE_MULTICAST_DELEGATE_OneParam(FStartMatch, const float);

/*
 * Inherits from Actor so can replicate data
 */

UCLASS()
class TANTRUMN_API ATantrumnGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void SetGameState(const ETantrumnGameState InGameState);
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	ETantrumnGameState GetGameState() const { return TantrumnGameState; };
	/** Quick Check to Determine if Game State is Playing */
	UFUNCTION(BlueprintPure)
	bool IsGameInPlay() const { return TantrumnGameState == ETantrumnGameState::Playing;}

	/** Game Type Public Accessors */
	/** Set the Game Type - Should Correspond with GameMode */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void SetGameType(const ETantrumnGameType InGameType);
	/** Get the Game Type - Should Correspond with GameMode */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	ETantrumnGameType GetGameType() const { return TantrumnGameType;}
	
	/** TODO This will only be called on a system which has Authority, review this
	 * Should be in game mode */
	void OnPlayerReachedEnd(ATantrumnCharacterBase* TantrumnCharacter);

	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	const TArray<FGameResult>& GetResults() { return Results; }
	UFUNCTION()
	void ClearResults();
	
	/** Methods relating to Match Start and Time Management */
	UFUNCTION(NetMulticast, Reliable, Category = "Tantrumn")
	void NM_MatchStart();
	FStartMatch OnStartMatchDelegate;
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	float GetMatchStartTime() const { return MatchStartTime; }
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	float GetMatchDeltaTime() const { return GetServerWorldTimeSeconds() - MatchStartTime; }
	
	FGameTypeUpdateDelegate OnGameTypeUpdateDelegate;
	
private:

	/** Game Results */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_ResultsUpdated, Category = "Tantrumn States")
	TArray<FGameResult> Results;
	UFUNCTION()
	void OnRep_ResultsUpdated();
	/** Can be called during and after play */
	void PlayerRequestSubmitResults(const ATantrumnCharacterBase* InTantrumnCharacter);
	/** Check if all results are in then let clients know the final results */
	void TryFinaliseScoreBoard();
	bool CheckAllResultsIn() const ;
	
	/** The State of the Game */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_GameState, Category = "Tantrumn States")
	ETantrumnGameState TantrumnGameState = ETantrumnGameState::None;
	UPROPERTY()
	ETantrumnGameState OldTantrumnGameState = ETantrumnGameState::None;
	UFUNCTION()
	void OnRep_GameState() const;

	/** The type of game being played - is correlated to which gamemode is selected */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_GameType, Category = "Tantrumn States")
	ETantrumnGameType TantrumnGameType = ETantrumnGameType::None;
	UFUNCTION()
	void OnRep_GameType() const;

	/** Game Time Values */
	UPROPERTY(VisibleAnywhere, Category = "Tantrumn States")
	float CountDownStartTime;
	/** Server - Client Time Sync handled by Player Controllers */ 
	UPROPERTY(VisibleAnywhere, Category = "Tantrumn States")
	float MatchStartTime;
};
