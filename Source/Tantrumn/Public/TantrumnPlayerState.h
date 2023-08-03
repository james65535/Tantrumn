// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TantrumnPlayerState.generated.h"

class USaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveGameUpdate);

// Enum to track the current state of the player
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

	virtual void BeginPlay() override;
	UFUNCTION(BlueprintPure, Category = "Tantrumn")
	EPlayerGameState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void SetCurrentState(const EPlayerGameState PlayerGameState);

	UFUNCTION(BlueprintPure, Category = "Tantrumn")
	bool IsWinner() const { return bIsWinner; }

	void SetIsWinner(const bool IsWinner) { bIsWinner = IsWinner; }

	UFUNCTION(BlueprintCallable, Category = "Tantrumn Save Info")
	void SavePlayerInfo();
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Tantrumn Save Info")
	void LoadSavedPlayerInfo();
	UPROPERTY(BlueprintAssignable, Category = "Tantrumn Save Info")
	FOnSaveGameUpdate OnSaveGameLoad;
	
private:

	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	EPlayerGameState CurrentState = EPlayerGameState::None;
	UFUNCTION()
	virtual void OnRep_CurrentState();

	virtual void OnRep_PlayerName() override;

	/** Save and Load Delegates */
	void SavePlayerDelegate(const FString& SlotName, const int32 UserIndex, bool bSuccess);
	void LoadPlayerSaveDelegate(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData);
	// TODO Build these properties out instead of being static
	const FString SaveSlotName = TEXT("GeneralSaveSlot");
	const uint32 SaveUserIndex = 0;
	
	bool bIsWinner = false;
	
};
