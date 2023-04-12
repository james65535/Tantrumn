// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumnCharMovementComponent.h"
#include "Tantrumn/TantrumnCharacterBase.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


UTantrumnCharMovementComponent::UTantrumnCharMovementComponent()
{
	
}

#pragma region SavedMove

UTantrumnCharMovementComponent::FSavedMove_Tantrumn::FSavedMove_Tantrumn()
{
	Saved_bWantsToStun = 0;
}

bool UTantrumnCharMovementComponent::FSavedMove_Tantrumn::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_Tantrumn* NewTantrumnMove = static_cast<FSavedMove_Tantrumn*>(NewMove.Get());

	if (Saved_bWantsToStun != NewTantrumnMove->Saved_bWantsToStun)
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UTantrumnCharMovementComponent::FSavedMove_Tantrumn::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToStun = 0;
}

uint8 UTantrumnCharMovementComponent::FSavedMove_Tantrumn::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	if (Saved_bWantsToStun) Result |= FLAG_Custom_0;

	return Result;
}

void UTantrumnCharMovementComponent::FSavedMove_Tantrumn::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UTantrumnCharMovementComponent* CharacterMovement = Cast<UTantrumnCharMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToStun = CharacterMovement->Safe_bWantsToStun;
}

void UTantrumnCharMovementComponent::FSavedMove_Tantrumn::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UTantrumnCharMovementComponent* CharacterMovement = Cast<UTantrumnCharMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToStun = Saved_bWantsToStun;
}

#pragma endregion SavedMove

#pragma region CMC

void UTantrumnCharMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetOwner());
}

void UTantrumnCharMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToStun = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UTantrumnCharMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr)
	{
		UTantrumnCharMovementComponent* MutableThis = const_cast<UTantrumnCharMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Tantrumn(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

bool UTantrumnCharMovementComponent::IsStunned() const
{
	//return Safe_bWantsToStun; // TODO is it right to call value from the var for compressed flag operations?  Perhaps: Saved_bWantsToStun?
	return (Safe_bWantsToStun) && UpdatedComponent;
}

#pragma endregion CMC

#pragma region Movement

void UTantrumnCharMovementComponent::DoSprintStart()
{
	MaxWalkSpeed = WalkSpeed * SprintModifier;
}

void UTantrumnCharMovementComponent::DoSprintEnd()
{
	MaxWalkSpeed = WalkSpeed;
}

void UTantrumnCharMovementComponent::RequestStun(const bool bClientSimulation, const float DurationMultiplier)
{

	if (!HasValidData())
	{
		return;
	}

	// No need to run this on simulation proxies
	if( !bClientSimulation )
	{
		// Set timer to call StunEnd after duration
		Safe_bWantsToStun = true;
		TantrumnCharacter->bIsStunned = true;
		const float CurrentTime = GetWorld()->GetTimeSeconds();
		StunStartTime = GetWorld()->GetTimeSeconds();
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_EnterStun,
			this,
			&UTantrumnCharMovementComponent::OnStunCoolDownFinished,
			StunDuration * DurationMultiplier - (CurrentTime - StunStartTime));
		UE_LOG(LogTemp, Warning, TEXT("Player %s entered Stun at: %f"), *TantrumnCharacter->GetName(), StunStartTime);
		
		// Prevent character from moving while stunned
		SetMovementMode(MOVE_None);

		// Broadcast to allow other methods to trigger from stun status
		StunStartDelegate.Broadcast();
	}
	else
	{
		TantrumnCharacter->bIsStunned = true;
	}
}

void UTantrumnCharMovementComponent::RequestStunEnd(bool bClientSimulation)
{
	if (bClientSimulation)
	{
		TantrumnCharacter->bIsStunned = false;
	}
}


void UTantrumnCharMovementComponent::OnStunCoolDownFinished()
{
	if (!HasValidData())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	UE_LOG(LogTemp, Warning, TEXT("Player %s exited Stun at: %f with duration: %f"),
		*TantrumnCharacter->GetName(), CurrentTime, CurrentTime-StunStartTime);

	SetMovementMode(MOVE_Walking);
	Safe_bWantsToStun = false;
	TantrumnCharacter->bIsStunned = false;
	StunStartTime = 0.0f;
}

void UTantrumnCharMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

#pragma endregion Movement

#pragma region Network

#pragma region ClientNetworkPredictionData

UTantrumnCharMovementComponent::FNetworkPredictionData_Client_Tantrumn::FNetworkPredictionData_Client_Tantrumn(
	const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr UTantrumnCharMovementComponent::FNetworkPredictionData_Client_Tantrumn::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Tantrumn());
}

#pragma endregion ClientNetworkPredictionData

#pragma region Replication

void UTantrumnCharMovementComponent::OnRep_Stun()
{
	// TODO play montage?
	StunStartDelegate.Broadcast();
}

#pragma endregion Replication

#pragma endregion Network
