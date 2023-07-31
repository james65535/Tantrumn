// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumnGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Tantrumn/TantrumnCharacterBase.h"
#include "Tantrumn/TantrumnPlayerController.h"
#include "TantrumnPlayerState.h"

void ATantrumnGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/** Pushed Rep with Notify - MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnGameStateBase, TestVar, this); */
	FDoRepLifetimeParams PushedRepNotifyParams;
	PushedRepNotifyParams.RepNotifyCondition = REPNOTIFY_Always;
	PushedRepNotifyParams.bIsPushBased = true;
	PushedRepNotifyParams.Condition = COND_SkipOwner;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, Results, PushedRepNotifyParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, TantrumnGameState, PushedRepNotifyParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, TantrumnGameType, PushedRepNotifyParams);
	
}

void ATantrumnGameStateBase::SetGameState(const ETantrumnGameState InGameState)
{
	OldTantrumnGameState = TantrumnGameState;
	TantrumnGameState = InGameState;
	MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnGameStateBase, TantrumnGameState, this);
	// Manual invocation of OnRep_GameState so server will also run the method
	if (HasAuthority())
	{
		OnRep_GameState();
	}
}

void ATantrumnGameStateBase::OnRep_GameState() const
{
}

void ATantrumnGameStateBase::SetGameType(const ETantrumnGameType InGameType)
{
	TantrumnGameType = InGameType;
	MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnGameStateBase, TantrumnGameType, this);
	// Manual invocation of OnRep_GameState so server will also run the method
	if (HasAuthority())
	{
		OnRep_GameType();
	}
}

void ATantrumnGameStateBase::OnRep_GameType() const
{
	// If Replicated with COND_SkipOwner then Authority will need to run this manually
	OnGameTypeUpdateDelegate.Broadcast(TantrumnGameType);
}

void ATantrumnGameStateBase::ClearResults()
{
	Results.Empty();
}

void ATantrumnGameStateBase::NM_MatchStart_Implementation()
{
	ClearResults();
	MatchStartTime = GetServerWorldTimeSeconds();
	SetGameState(ETantrumnGameState::Playing);
	OnStartMatchDelegate.Broadcast(MatchStartTime);
	UE_LOG(LogTemp, Warning, TEXT("Gamestate match start time: %f"), MatchStartTime);
}

void ATantrumnGameStateBase::PlayerRequestSubmitResults(const ATantrumnCharacterBase* InTantrumnCharacter)
{
	if (ATantrumnPlayerState* TantrumnPlayerState = Cast<ATantrumnPlayerState>(InTantrumnCharacter->GetPlayerState()))
	{
		if (TantrumnPlayerState->GetCurrentState() != EPlayerGameState::Finished)
		{
			TantrumnPlayerState->SetCurrentState(EPlayerGameState::Finished);
			
			FGameResult Result;
			Result.Time = GetServerWorldTimeSeconds() - MatchStartTime;
			Result.Name = TantrumnPlayerState->GetPlayerName();
			const bool IsWinner = Results.Num() == 0;
			TantrumnPlayerState->SetIsWinner(IsWinner);
			Result.bIsWinner = IsWinner;
			Results.Add(Result);
		}
	}
}

void ATantrumnGameStateBase::OnRep_ResultsUpdated()
{
	for (const TObjectPtr<APlayerState> PlayerState : PlayerArray)
	{
		if(const ATantrumnPlayerController* PlayerController = Cast<ATantrumnPlayerController>(
			PlayerState->GetPlayerController()))
		{
			PlayerController->RequestDisplayFinalResults();
		}
	}
}

void ATantrumnGameStateBase::OnPlayerReachedEnd(ATantrumnCharacterBase* TantrumnCharacter)
{
	PlayerRequestSubmitResults(TantrumnCharacter);
	TantrumnCharacter->NM_FinishedMatch();
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
		TantrumnGameState = ETantrumnGameState::GameOver;
		MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnGameStateBase, Results, this);
		MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnGameStateBase, TantrumnGameState, this);
		
		/** if running a local game then need to call the OnRep function manually */
		if (HasAuthority() && !IsRunningDedicatedServer())
		{
			OnRep_ResultsUpdated();
		}
	}
}
