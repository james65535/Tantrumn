// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TantrumnCharMovementComponent.generated.h"

// Stun delegates
DECLARE_MULTICAST_DELEGATE(FOnStunStart);
DECLARE_MULTICAST_DELEGATE(FOnStunEnd);


// TODO determine whether to go with custom movement mode or bool for stun and other things
UENUM(BlueprintType)
enum ECustomMovementMode
{
	TANTRUMNMOVE_None			UMETA(Hidden),
	TANTRUMNMOVE_Stun			UMETA(DisplayName = "Stunned"),
};

UCLASS()
class TANTRUMN_API UTantrumnCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// Set default values
	UTantrumnCharMovementComponent();

	// General Movement
	UFUNCTION(Category = "Movement")
	void DoSprintStart();
	UFUNCTION(Category = "Movement")
	void DoSprintEnd();
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Movement")
	float SprintModifier = 2.0f;
	UPROPERTY(BlueprintReadWrite,  EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 600.0f;

	// Stun Methods
	UFUNCTION(BlueprintCallable, meta=(ClampMin="0.1", UIMin="0.1"), Category = "Stun")
	void DoStun(const float DurationMultiplier);
	UFUNCTION(BlueprintPure, Category = "Stun")
	bool IsStunned() const {return bIsStunned;}

	// Stun delegates
	FOnStunStart OnStunStart;
	FOnStunEnd OnStunEnd;

	// Stun Defaults
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float StunDuration = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float StunDurationMultiplier = 1.0f;

protected:
	virtual void InitializeComponent() override;

	// Stun Internals
	void StunEnd();
	FTimerHandle StunTimer;
	UPROPERTY()
	bool bIsStunned = false;
};
