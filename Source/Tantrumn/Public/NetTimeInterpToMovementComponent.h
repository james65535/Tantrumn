// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InterpToMovementComponent.h"
#include "NetTimeInterpToMovementComponent.generated.h"

class ATantrumnGameStateBase;

UCLASS(ClassGroup=Movement, meta=(BlueprintSpawnableComponent),HideCategories=Velocity)
class TANTRUMN_API UNetTimeInterpToMovementComponent : public UInterpToMovementComponent
{
	GENERATED_BODY()

	UNetTimeInterpToMovementComponent();

public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;

protected:
	
	UPROPERTY(Replicated)
	float CurrentTime;
	UPROPERTY(Replicated)
	float CurrentDirection;

private:

	FTimerHandle SlowTickTimerHandle;
	float SlowTickerFrequencySeconds = 2.0f;

	/** Used to casually push replicate location and time on spline */
	UFUNCTION()
	void SlowTick() const;

	/** Used for GetServerWorldTime for consistent replication */
	UPROPERTY()
	ATantrumnGameStateBase* GameState;
	float NetworkStartTimeSeconds;
	float NetworkDeltaTimeSeconds;
	
};
