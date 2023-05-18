// Fill out your copyright notice in the Description page of Project Settings.


#include "TBT_TaskRequestSprintStart.h"

#include "TantrumnAIController.h"
#include "Tantrumn/TantrumnCharacterBase.h"

EBTNodeResult::Type UTBT_TaskRequestSprintStart::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	const ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(OwnerComp.GetOwner());
	ATantrumnCharacterBase* TantrumnCharacter = TantrumnAIController ?
		TantrumnAIController->GetPawn<ATantrumnCharacterBase>() : nullptr;
	if (TantrumnCharacter)
	{
		TantrumnCharacter->RequestSprintStart();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
