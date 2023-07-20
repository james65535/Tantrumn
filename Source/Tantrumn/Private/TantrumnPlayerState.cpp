// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

void ATantrumnPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams PushedRepNotifyParams;
	PushedRepNotifyParams.bIsPushBased = true;
	PushedRepNotifyParams.RepNotifyCondition = REPNOTIFY_OnChanged;
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnPlayerState, CurrentState, PushedRepNotifyParams);
}

void ATantrumnPlayerState::SetCurrentState(const EPlayerGameState PlayerGameState)
{
	if (HasAuthority())
	{
		CurrentState = PlayerGameState;
		MARK_PROPERTY_DIRTY_FROM_NAME(ATantrumnPlayerState, CurrentState, this);
	}
}

void ATantrumnPlayerState::OnRep_CurrentState()
{
	UE_LOG(LogTemp, Warning, TEXT("Player %s State Updated to: %s"), *GetPlayerName() ,*UEnum::GetDisplayValueAsText(CurrentState).ToString());
}
