// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnCharacterBase.h"
#include "TantrumnPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ATantrumnCharacterBase::ATantrumnCharacterBase()
{
 	// Set this character to call Tick() every frame.
 	// You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATantrumnCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATantrumnCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATantrumnCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Called when landed
void ATantrumnCharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Apply Controller Force FeedBack and stun player if impact speed exceeds threshold
	if(ATantrumnPlayerController* PlayerController = GetController<ATantrumnPlayerController>())
	{
		const float FallImpactSpeed = FMath::Abs(GetVelocity().Z);
		if (FallImpactSpeed > MinImpactSpeed)
		{
			// Stun the player to affect movement for a duration
			OnStunBegin(StunDuration);

			// Calculate Severity of Impact
			const float DeltaImpact = MaxImpactSpeed - MinImpactSpeed;
			const float FallRatio = FMath::Clamp((FallImpactSpeed - MinImpactSpeed) / DeltaImpact, 0.0f, 1.0f);

			// Apply force feedback
			const bool bAffectSmall = FallRatio <= 0.5;
			const bool bAffectLarge = FallRatio > 0.5;
			PlayerController->PlayDynamicForceFeedback(FallRatio,
				0.5f,
				bAffectLarge, bAffectSmall,
				bAffectLarge, bAffectSmall);
		}
	}
}

// Stuns player by disabling movement for a specified duration
void ATantrumnCharacterBase::OnStunBegin(float StunLength)
{
	// TODO Check into custom movement mode enum
	bIsStunned = true;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(EMovementMode::MOVE_None);
	}
	GetWorldTimerManager().SetTimer(StunTimer, this, &ATantrumnCharacterBase::OnStunEnd, 1.0f,false);
}

// Removes Stun and restores movement to player
void ATantrumnCharacterBase::OnStunEnd()
{
	bIsStunned = false;

	// Gradually release stun movement constraints
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}



