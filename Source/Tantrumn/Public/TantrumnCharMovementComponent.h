// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TantrumnCharMovementComponent.generated.h"

/**
 *  TODO Update for multiplayer replication
 */
UCLASS()
class TANTRUMN_API UTantrumnCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	// Set default values
	UTantrumnCharMovementComponent();

	UFUNCTION(BlueprintPure)
	bool IsStunned() const;

	

protected:
	virtual void InitializeComponent() override;
	
};
