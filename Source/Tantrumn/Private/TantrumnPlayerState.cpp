// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "TantrumnGeneralSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Tantrumn/TantrumnGameModeBase.h"
#include "Tantrumn/TantrumnPlayerController.h"

void ATantrumnPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams PushedRepNotifyParams;
	PushedRepNotifyParams.bIsPushBased = true;
	PushedRepNotifyParams.RepNotifyCondition = REPNOTIFY_OnChanged;
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnPlayerState, CurrentState, PushedRepNotifyParams);
}

void ATantrumnPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (ATantrumnPlayerController* TantrumnPlayerController =  Cast<ATantrumnPlayerController>(GetPlayerController()))
	{
		if (!TantrumnPlayerController->TantrumnPlayerState)
		{
			TantrumnPlayerController->SetTantrumnPlayerState(this);
			if(!IsRunningDedicatedServer())
			{
				LoadSavedPlayerInfo();
			}
		}
	}
}

void ATantrumnPlayerState::SetCurrentState(const EPlayerGameState PlayerGameState)
{
	if (HasAuthority())
	{
		CurrentState = PlayerGameState;
		MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnPlayerState, CurrentState, this);

		/** If Player is Ready then notify game mode */
		if (PlayerGameState == EPlayerGameState::Ready)
		{
			ATantrumnGameModeBase* TantrumnGameMode = Cast<ATantrumnGameModeBase>(GetWorld()->GetAuthGameMode());
			check(TantrumnGameMode);
			TantrumnGameMode->PlayerNotifyIsReady(this);
		}
	}
}

void ATantrumnPlayerState::OnRep_CurrentState()
{
	UE_LOG(LogTemp, Warning, TEXT("Player %s State Updated to: %s"), *GetPlayerName() ,*UEnum::GetDisplayValueAsText(CurrentState).ToString());
}

void ATantrumnPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (const ATantrumnPlayerController* TantrumnPlayerController =  Cast<ATantrumnPlayerController>(GetPlayerController()))
	{
		TantrumnPlayerController->OnPlayerStateReceived.Broadcast();
	}
}

void ATantrumnPlayerState::SavePlayerInfo()
{
	if (IsRunningDedicatedServer())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player save called on server or client with no authority"));
		return;
	}
	
	if (UTantrumnGeneralSaveGame* SaveGameInstance =
		Cast<UTantrumnGeneralSaveGame>(UGameplayStatics::CreateSaveGameObject(UTantrumnGeneralSaveGame::StaticClass())))
	{
		FAsyncSaveGameToSlotDelegate OnSavedToSlot;
		OnSavedToSlot.BindUObject(this, &ATantrumnPlayerState::SavePlayerDelegate);

		/** Assign data to be saved */
		SaveGameInstance->TantrumnPlayerName = GetPlayerName();
		SaveGameInstance->UserIndex = SaveUserIndex;
		
		UGameplayStatics::AsyncSaveGameToSlot(SaveGameInstance, SaveSlotName, SaveUserIndex, OnSavedToSlot);
	}
}

void ATantrumnPlayerState::SavePlayerDelegate(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("Save Process: %s"), bSuccess ? *FString("Successful") : *FString("Failed"));
}

void ATantrumnPlayerState::LoadSavedPlayerInfo_Implementation()
{
	if (IsRunningDedicatedServer()) { return; }
	
	FAsyncLoadGameFromSlotDelegate OnLoadSaveFromSlot;
	OnLoadSaveFromSlot.BindUObject(this, &ATantrumnPlayerState::LoadPlayerSaveDelegate);
	
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, SaveUserIndex, OnLoadSaveFromSlot);
}

void ATantrumnPlayerState::LoadPlayerSaveDelegate(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	if (const UTantrumnGeneralSaveGame* LoadedPlayerInfo =
		Cast<UTantrumnGeneralSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
	{
		GetPlayerController()->SetName(LoadedPlayerInfo->TantrumnPlayerName);
		OnSaveGameLoad.Broadcast();
	}
}
