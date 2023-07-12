// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumnGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Tantrumn/TantrumnCharacterBase.h"
#include "Tantrumn/TantrumnPlayerController.h"
#include "TantrumnPlayerState.h"
#include "TantrumnCharMovementComponent.h"
#include "TantrumnAIController.h"

void ATantrumnGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	FDoRepLifetimeParams RepNotifyParams;
	RepNotifyParams.bIsPushBased = true;
	RepNotifyParams.RepNotifyCondition = REPNOTIFY_OnChanged;
	RepNotifyParams.Condition = COND_SkipOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, TantrumnGameState, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, Results, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, TantrumnGameType, RepNotifyParams);
}

void ATantrumnGameStateBase::OnRep_GameState()
{
	UE_LOG(LogTemp, Warning, TEXT("OldGameState: %s"), *UEnum::GetDisplayValueAsText(OldTantrumnGameState).ToString());
	UE_LOG(LogTemp, Warning, TEXT("TantrumnGameState: %s"), *UEnum::GetDisplayValueAsText(TantrumnGameState).ToString());
}

void ATantrumnGameStateBase::OnRep_GameType()
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

	if (ATantrumnPlayerController* TantrumnPlayerController = TantrumnCharacter->GetController<ATantrumnPlayerController>())
	{
		TantrumnPlayerController->C_FinishedRound();
		TantrumnCharacter->GetCharacterMovement()->DisableMovement();
		
		if (ATantrumnPlayerState* PlayerState = TantrumnPlayerController->GetPlayerState<ATantrumnPlayerState>())
		{
			UpdateResults(PlayerState, TantrumnCharacter);
			UE_LOG(LogTemp, Warning, TEXT("Play state has %i results and %i players"), Results.Num(), PlayerArray.Num());
		}
	}
	else if (ATantrumnAIController* TantrumnAIController = TantrumnCharacter->GetController<ATantrumnAIController>())
	{
		if (ATantrumnPlayerState* PlayerState = TantrumnAIController->GetPlayerState<ATantrumnPlayerState>())
		{
			UpdateResults(PlayerState, TantrumnCharacter);
			TantrumnAIController->OnReachedEnd();
		}	
	}
	
	/** All Players Should Be Finished */
	if (Results.Num() >= PlayerArray.Num()-1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game State Found All Players Finished"));
		TantrumnGameState = ETantrumnGameState::GameOver;
		for (const TObjectPtr<APlayerState> PlayerState : PlayerArray)
		{
			if(ATantrumnPlayerController* PlayerController =
				Cast<ATantrumnPlayerController>(PlayerState->GetPlayerController()))
			{
				UE_LOG(LogTemp, Warning, TEXT("Game State requesting results"));
				PlayerController->C_RequestFinalResults();
			}
		}
	}
}

void ATantrumnGameStateBase::ClearResults()
{
	Results.Empty();
}

void ATantrumnGameStateBase::UpdateResults(ATantrumnPlayerState* PlayerState, ATantrumnCharacterBase* TantrumnCharacter)
{
	if (!PlayerState || !TantrumnCharacter)
	{
		return;
	}
	
	const bool IsWinner = Results.Num() == 0;
	PlayerState->SetIsWinner(IsWinner);
	PlayerState->SetCurrentState(EPlayerGameState::Finished);
	
	FGameResult Result;
	Result.Name = TantrumnCharacter->GetName();
	Result.Time = 5.0f;
	Results.Add(Result);
}
