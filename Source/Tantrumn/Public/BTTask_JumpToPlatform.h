// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_JumpToPlatform.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUMN_API UBTTask_JumpToPlatform : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:

	UBTTask_JumpToPlatform();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:

	FVector JumpToPlatformLocation(FVector StartLoc, FVector& ValidLoc, float SearchRadius , uint8 NumRetries);
	
};
