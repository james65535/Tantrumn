// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnAIController.h"

#include "TantrumnCharMovementComponent.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnPlayerState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Tantrumn/TantrumnCharacterBase.h"

ATantrumnAIController::ATantrumnAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("Perception Component");
}

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
	if(UBlackboardComponent* BlackBoard = GetBlackboardComponent())
	{
		BlackBoard->SetValueAsBool(FName("IsPlaying"), true);
		if(ATantrumnCharacterBase* TantrumnPawn = Cast<ATantrumnCharacterBase>(GetPawn()))
		{
			TantrumnPawn->RequestSprintStart();
			TantrumnPawn->bUseControllerRotationYaw = false;
			if(UTantrumnCharMovementComponent* TantrumnCharMovementComponent = Cast<UTantrumnCharMovementComponent>(TantrumnPawn->GetMovementComponent()))
			{
				TantrumnCharMovementComponent->bOrientRotationToMovement = false;
				TantrumnCharMovementComponent->bUseControllerDesiredRotation= true;
			}
		}
	}
}

void ATantrumnAIController::RespondToBeingRescued(bool bIsBeingRescued)
{
	if (UBlackboardComponent* BlackBoard = GetBlackboardComponent())
	{ BlackBoard->SetValueAsBool(FName("IsBeingRescued"), bIsBeingRescued); }
}

void ATantrumnAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
	{ TantrumnPlayerState->SetCurrentState(EPlayerGameState::Waiting); }

	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(InPawn))
	{
		TantrumnCharacter->OnBeingRescuedEvent.AddDynamic(this, &ThisClass::RespondToBeingRescued);
	}
}

void ATantrumnAIController::OnUnPossess()
{

	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetPawn()))
	{ TantrumnCharacter->OnBeingRescuedEvent.RemoveAll(this);}
	
	Super::OnUnPossess();
}