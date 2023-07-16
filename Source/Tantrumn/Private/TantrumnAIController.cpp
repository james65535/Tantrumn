// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnAIController.h"
#include "TantrumnPlayerState.h"

void ATantrumnAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
		if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
		{
			TantrumnPlayerState->SetCurrentState(EPlayerGameState::Waiting);
		}
}

void ATantrumnAIController::OnUnPossess()
{
	Super::OnUnPossess();
}