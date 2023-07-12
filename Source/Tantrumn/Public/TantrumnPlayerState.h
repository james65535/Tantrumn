// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TantrumnPlayerState.generated.h"

// Enum to track the current state of the game
UENUM()
enum class EPlayerGameState : uint8
{
	None		UMETA(DisplayName = "None"),
	Waiting		UMETA(DisplayName = "Waiting"),
	Ready		UMETA(DisplayName = "Ready"),
	Playing		UMETA(DisplayName = "Playing"),
	Finished	UMETA(DisplayName = "Finished"),
};
UCLASS()
class TANTRUMN_API ATantrumnPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure)
	EPlayerGameState GetCurrentState() const { return CurrentState; }

	UFUNCTION()
	void SetCurrentState(const EPlayerGameState PlayerGameState);
	

	UFUNCTION(BlueprintPure)
	bool IsWinner() const { return bIsWinner; }

	void SetIsWinner(const bool IsWinner) { bIsWinner = IsWinner; }

private:

	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	EPlayerGameState CurrentState = EPlayerGameState::None;
	UFUNCTION()
	virtual void OnRep_CurrentState();

	UPROPERTY(Replicated)
	bool bIsWinner = false;
	
};
