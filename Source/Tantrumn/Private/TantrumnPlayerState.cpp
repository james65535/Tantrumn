// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "TantrumnGeneralSaveGame.h"
#include "TantrumnHUD.h"
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
	
	if (ATantrumnPlayerController* TantrumnPlayerController = Cast<ATantrumnPlayerController>(GetPlayerController()))
	{
		if (!IsValid(TantrumnPlayerController->TantrumnPlayerState))
		{
			TantrumnPlayerController->SetTantrumnPlayerState(this);
			if(!IsRunningDedicatedServer())
			{ LoadSavedPlayerInfo(); }
		}
		SetCurrentState(EPlayerGameState::Unready);
	}
}

void ATantrumnPlayerState::SetCurrentState(const EPlayerGameState PlayerGameState)
{
	if (!HasAuthority())
	{ return; }
	
	CurrentState = PlayerGameState;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CurrentState, this);
	
	/** If Player is Ready then notify game mode */
	if (PlayerGameState == EPlayerGameState::Waiting)
	{
		ATantrumnGameModeBase* TantrumnGameMode = Cast<ATantrumnGameModeBase>(GetWorld()->GetAuthGameMode());
		check(TantrumnGameMode);
		TantrumnGameMode->PlayerNotifyIsReady(this);
	}

	/** For local only non-network mode */
	if (!IsRunningDedicatedServer())
	{
		if(const ATantrumnPlayerController* TantrumnPlayerController = Cast<ATantrumnPlayerController>(GetOwner()))
		{
			if(const ATantrumnHUD* TantrumnHUD = Cast<ATantrumnHUD>(TantrumnPlayerController->GetHUD()))
			{ TantrumnHUD->UpdateDisplayedPlayerState(CurrentState); }
		}
	}
}

void ATantrumnPlayerState::OnRep_CurrentState()
{
	if (!IsValid(GetPlayerController()))
	{ return; }
	
	if(const ATantrumnHUD* PlayerHud = Cast<ATantrumnHUD>(GetPlayerController()->GetHUD()))
	{ PlayerHud->UpdateDisplayedPlayerState(CurrentState); }
}

void ATantrumnPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if (const ATantrumnPlayerController* TantrumnPlayerController =  Cast<ATantrumnPlayerController>(GetPlayerController()))
	{ TantrumnPlayerController->OnPlayerStateReceived.Broadcast(); }
}

void ATantrumnPlayerState::SavePlayerInfo()
{
	if (IsRunningDedicatedServer())
	{ return; }
	
	if (UTantrumnGeneralSaveGame* SaveGameInstance =
		Cast<UTantrumnGeneralSaveGame>(UGameplayStatics::CreateSaveGameObject(UTantrumnGeneralSaveGame::StaticClass())))
	{
		FAsyncSaveGameToSlotDelegate OnSavedToSlot;
		OnSavedToSlot.BindUObject(this, &ATantrumnPlayerState::SavePlayerDelegate);

		/** Assign data to be saved */
		SaveGameInstance->TantrumnPlayerName = *GetPlayerName();
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
