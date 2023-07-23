// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InterpToMovementComponent.h"
#include "NetTimeInterpToMovementComponent.generated.h"

class ATantrumnGameStateBase;
/**
 * 
 */
UCLASS(ClassGroup=Movement, meta=(BlueprintSpawnableComponent),HideCategories=Velocity)
class TANTRUMN_API UNetTimeInterpToMovementComponent : public UInterpToMovementComponent
{
	GENERATED_BODY()

	UNetTimeInterpToMovementComponent();

public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;

protected:

	//virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
	//void ReverseDirection(const FHitResult& Hit, float Time, bool InBroadcastEvent);
	//void RestartMovement(float InitialDirection);

	UPROPERTY(ReplicatedUsing=OnRep_CurrentTime)
	float CurrentTime;
	UPROPERTY(ReplicatedUsing=OnRep_CurrentDirection)
	float CurrentDirection;

	UFUNCTION()
	void OnRep_CurrentTime() { UE_LOG(LogTemp, Warning, TEXT("OnRepCurrentTime: %f"), CurrentTime); }
	UFUNCTION()
	void OnRep_CurrentDirection() { UE_LOG(LogTemp, Warning, TEXT("OnRepCurrentTime: %f"), CurrentTime); }
	

private:

	FTimerHandle SlowTickTimerHandle;
	float SlowTickerFrequencySeconds = 2.0f;

	UFUNCTION()
	void SlowTick();
	
	UPROPERTY()
	ATantrumnGameStateBase* GameState;
	float NetworkStartTimeSeconds;
	float NetworkDeltaTimeSeconds;

	/** Shadows Parent Class Private vars */
	float TotalDistance;
	FVector StartLocation;
	bool bPointsFinalized;
	
};
