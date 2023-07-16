// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumnGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Tantrumn/TantrumnCharacterBase.h"
#include "Tantrumn/TantrumnPlayerController.h"
#include "TantrumnPlayerState.h"

void ATantrumnGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/** Simple Rep */
	FDoRepLifetimeParams SharedParams;
	SharedParams.Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, TantrumnGameState, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, MatchStartTime, SharedParams)

	/** Rep with notify */
	FDoRepLifetimeParams RepNotifyParams;
	RepNotifyParams.RepNotifyCondition = REPNOTIFY_OnChanged;
	RepNotifyParams.Condition = COND_SkipOwner;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, TantrumnGameType, RepNotifyParams);

	/** Pushed Rep with Notify - MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnGameStateBase, TestVar, this); */
	FDoRepLifetimeParams PushedRepNotifyParams;
	PushedRepNotifyParams.RepNotifyCondition = REPNOTIFY_OnChanged;
	PushedRepNotifyParams.bIsPushBased = true;
	PushedRepNotifyParams.Condition = COND_SkipOwner;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, Results, PushedRepNotifyParams);
	
}

void ATantrumnGameStateBase::OnRep_GameState() const
{
	UE_LOG(LogTemp, Warning, TEXT("OldGameState: %s"), *UEnum::GetDisplayValueAsText(OldTantrumnGameState).ToString());
	UE_LOG(LogTemp, Warning, TEXT("TantrumnGameState: %s"), *UEnum::GetDisplayValueAsText(TantrumnGameState).ToString());
}

void ATantrumnGameStateBase::OnRep_GameType() const
{
	OnGameTypeUpdateDelegate.Broadcast(TantrumnGameType);
}

void ATantrumnGameStateBase::SetGameState(const ETantrumnGameState InGameState)
{
	OldTantrumnGameState = TantrumnGameState;
	TantrumnGameState = InGameState;
}

void ATantrumnGameStateBase::SetGameType(const ETantrumnGameType InGameType)
{
	TantrumnGameType = InGameType;
}

void ATantrumnGameStateBase::OnPlayerReachedEnd(ATantrumnCharacterBase* TantrumnCharacter)
{
	ensureMsgf(HasAuthority(), TEXT("ATantrumnGameStateBase::OnPlayerReachedEnd being called from Non Authority!"));
	TantrumnCharacter->NM_FinishedMatch();
	UpdateResults(TantrumnCharacter);
	TryFinaliseScoreBoard();
}

bool ATantrumnGameStateBase::CheckAllResultsIn() const
{
	const uint8 FinalNumPlayers = PlayerArray.Num();
	if (FinalNumPlayers > 0 && Results.Num() == FinalNumPlayers)
	{
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("GameState CheckAllResults has %i Players and %i Results."), FinalNumPlayers, Results.Num());
	return false;
}

void ATantrumnGameStateBase::TryFinaliseScoreBoard()
{
	if(CheckAllResultsIn())
	{
		/** Results Replication Is Pushed to Mark Dirty */
		MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnGameStateBase, Results, this);
		TantrumnGameState = ETantrumnGameState::GameOver;
		
		/** if running a local game then need to call the OnRep function manually */
		if (HasAuthority() && !IsRunningDedicatedServer())
		{
			OnRep_Results();
		}
	}
}

void ATantrumnGameStateBase::ClearResults()
{
	Results.Empty();
}

void ATantrumnGameStateBase::NM_MatchStart_Implementation()
{
	MatchStartTime = FGenericPlatformTime::ToSeconds(FPlatformTime::Cycles());
	UE_LOG(LogTemp, Warning, TEXT("Gamestate match start time: %f"), MatchStartTime);
	SetGameState(ETantrumnGameState::Playing);
	ClearResults();
	OnStartMatchDelegate.Broadcast(GetMatchDeltaTime());
}

void ATantrumnGameStateBase::OnRep_Results()
{
	for (const TObjectPtr<APlayerState> PlayerState : PlayerArray)
	{
		if(const ATantrumnPlayerController* PlayerController =
			Cast<ATantrumnPlayerController>(PlayerState->GetPlayerController()))
		{
			PlayerController->RequestDisplayFinalResults();
		}
	}
}

void ATantrumnGameStateBase::UpdateResults(const ATantrumnCharacterBase* InTantrumnCharacter)
{
	if (ATantrumnPlayerState* TantrumnPlayerState = Cast<ATantrumnPlayerState>(InTantrumnCharacter->GetPlayerState()))
	{
		if (TantrumnPlayerState->GetCurrentState() != EPlayerGameState::Finished)
		{
			FGameResult Result;
			Result.Time = FGenericPlatformTime::ToSeconds(FPlatformTime::Cycles()) - MatchStartTime;
	
			const bool IsWinner = Results.Num() == 0;
			TantrumnPlayerState->SetIsWinner(IsWinner);
			TantrumnPlayerState->SetCurrentState(EPlayerGameState::Finished);

			Result.Name = InTantrumnCharacter->GetName();
			Result.bIsWinner = IsWinner;
			Results.Add(Result);
		}
	}
}
