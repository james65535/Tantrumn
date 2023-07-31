// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnAIController.h"

#include "TantrumnGameStateBase.h"
#include "TantrumnPlayerState.h"
#include "BehaviorTree/BlackboardComponent.h"

void ATantrumnAIController::BeginPlay()
{
	Super::BeginPlay();
	if(ATantrumnGameStateBase* TantrumnGameState = GetWorld()->GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->OnStartMatchDelegate.AddUObject(this, &ATantrumnAIController::SetIsPlaying);
	}
}

void ATantrumnAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Clear Delegates
	if(ATantrumnGameStateBase* TantrumnGameState = GetWorld()->GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->OnStartMatchDelegate.RemoveAll(this);
	}
}

void ATantrumnAIController::SetIsPlaying(const float InMatchStartTime)
{
	UE_LOG(LogTemp, Warning, TEXT("%f"), InMatchStartTime);
	if(UBlackboardComponent* BlackBoard = GetBlackboardComponent())
	{
		BlackBoard->SetValueAsBool(FName("IsPlaying"), true);
	}
}

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