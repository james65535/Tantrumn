// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TantrumnAIController.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUMN_API ATantrumnAIController : public AAIController
{
	GENERATED_BODY()

public:

	ATantrumnAIController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess), Category = "Tantrumn")
	class UAIPerceptionComponent* AIPerceptionComponent;

	// Delegate for Game State broadcast
	void SetIsPlaying(const float InMatchStartTime);

	UFUNCTION()
	void RespondToBeingRescued(bool bIsBeingRescued);
};
