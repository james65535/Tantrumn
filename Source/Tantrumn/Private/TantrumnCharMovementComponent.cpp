// Fill out your copyright notice in the Description page of Project Settings.

#include "TantrumnCharMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UTantrumnCharMovementComponent::UTantrumnCharMovementComponent()
{
	
}

void UTantrumnCharMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UTantrumnCharMovementComponent::DoSprintStart()
{
	MaxWalkSpeed = WalkSpeed * SprintModifier;
}

void UTantrumnCharMovementComponent::DoSprintEnd()
{
	MaxWalkSpeed = WalkSpeed;
}

// Stuns player by disabling movement for a specified duration
void UTantrumnCharMovementComponent::DoStun(const float DurationMultiplier)
{
	// TODO Check into custom movement mode enum
	bIsStunned = true;

	// Disable movement
	SetMovementMode(EMovementMode::MOVE_None);
	OnStunStart.Broadcast();

	// Set timer to call StunEnd after duration
	const float StunTime = StunDuration * DurationMultiplier;
	GetWorld()->GetTimerManager().SetTimer(
		StunTimer,
		this,
		&UTantrumnCharMovementComponent::StunEnd,
		StunTime,false);
}

// Removes Stun and restores movement to player
void UTantrumnCharMovementComponent::StunEnd()
{
	bIsStunned = false;

	//Enable Movement
	SetMovementMode(EMovementMode::MOVE_Walking);
	
	OnStunEnd.Broadcast();
}