// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

void ATantrumnPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.RepNotifyCondition = REPNOTIFY_OnChanged;

	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnPlayerState, CurrentState, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnPlayerState, bIsWinner, SharedParams);
}

void ATantrumnPlayerState::SetCurrentState(const EPlayerGameState PlayerGameState)
{
	if (HasAuthority())
	{
		CurrentState = PlayerGameState;
	}
}

void ATantrumnPlayerState::OnRep_CurrentState()
{
	UE_LOG(LogTemp, Warning, TEXT("Player State Updated to: %hhd"), CurrentState);
}
