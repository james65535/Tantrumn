// Fill out your copyright notice in the Description page of Project Settings.


#include "TBT_TaskAttemptPullObject.h"
#include "TantrumnAIController.h"
#include "Tantrumn/TantrumnCharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UTBT_TaskAttemptPullObject::UTBT_TaskAttemptPullObject(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Attempt Pull Object";
	bNotifyTick = false;
	bNotifyTaskFinished = true;

	// Accept only vectors
	BlackboardKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UTBT_TaskAttemptPullObject, BlackboardKey));
	
}

EBTNodeResult::Type UTBT_TaskAttemptPullObject::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) 
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	const ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(OwnerComp.GetOwner());
	ATantrumnCharacterBase* TantrumnCharacter = TantrumnAIController ?
		TantrumnAIController->GetPawn<ATantrumnCharacterBase>() : nullptr;
	if (TantrumnCharacter)
	{
		if (UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent())
		{
			
			const FVector TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(
				BlackboardKey.GetSelectedKeyID());
			MyBlackboard->ClearValue("Destination");
			if (TantrumnCharacter->AttemptPullObjectAtLocation(TargetLocation))
			{
				MyBlackboard->SetValueAsBool("HasBall", true);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}
