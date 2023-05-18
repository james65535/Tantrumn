// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Tantrumn/TantrumnCharacterBase.h"
#include "TantrumnGameStateBase.generated.h"

class ATantrumnPlayerState;
// ENUM to track the current state of the game
UENUM(BlueprintType)
enum class EGameState : uint8
{
	None		UMETA(DisplayName = "None"),
	Waiting		UMETA(DisplayName = "Waiting"),
	Playing		UMETA(DisplayName = "Playing"),
	Paused		UMETA(DisplayName = "Paused"),
	GameOver	UMETA(DisplayName = "GameOver"),
};

USTRUCT()
struct FGameResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name = "Null";

	UPROPERTY()
	float Time = 0.0f;;
};

class ATantrumnCharacterBase;

/*
 * Inherits from Actor so can replicate data
 */
UCLASS()
class TANTRUMN_API ATantrumnGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void SetGameState(EGameState InGameState) { GameState = InGameState;}

	UFUNCTION(BlueprintPure)
	bool IsPlaying() const { return GameState == EGameState::Playing;}

	// This will only be called on a system which has Authority
	void OnPlayerReachedEnd(ATantrumnCharacterBase* TantrumnCharacter);
	
	UFUNCTION()
	void ClearResults();

protected:

	void UpdateResults(ATantrumnPlayerState* PlayerState, ATantrumnCharacterBase* TantrumnCharacter);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_GameState, Category = "States")
	EGameState GameState = EGameState::None;

	UFUNCTION()
	void OnRep_GameState(const EGameState& OldGameState);

	UPROPERTY(VisibleAnywhere, Replicated, Category = "States")
	TArray<FGameResult> Results;
};
