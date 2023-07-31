// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_JumpToPlatform.h"

#include "TantrumnAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

static TAutoConsoleVariable<bool> CVarDisplayTrace(
	TEXT("Tantrumn.AI.Debug.DisplayTrace"),
	false,
	TEXT("Display AI Task Trace"),
	ECVF_Default);

UBTTask_JumpToPlatform::UBTTask_JumpToPlatform()
{
	NodeName = "Task Jump To Platform";
	bNotifyTick = false;
	bNotifyTaskFinished = true;

	// Accept only vectors
	BlackboardKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UBTTask_JumpToPlatform, BlackboardKey));
}

EBTNodeResult::Type UBTTask_JumpToPlatform::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if(const ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(OwnerComp.GetOwner()))
	{
		if (const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent())
		{
			const FVector JumpToLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(
				BlackboardKey.GetSelectedKeyID());
			UE_LOG(LogTemp, Warning, TEXT("BTTask Vector - x: %f y: %f z: %f"), JumpToLocation.X, JumpToLocation.Y, JumpToLocation.Z);
		}
	}
	FVector TempVector = FVector::ZeroVector;
	JumpToPlatformLocation(FVector::ZeroVector, TempVector, 0.0f,1);
	return EBTNodeResult::Succeeded;
}

FVector UBTTask_JumpToPlatform::JumpToPlatformLocation(FVector StartLoc, FVector& ValidLoc, float SearchRadius , uint8 NumRetries)
{
	// No need for multiple traces in circumference when radius is similar to sphere size
	uint8 NumTraces = SearchRadius <= 20.0f ? 1 : 3;
	
	// Setup trace vars
	FVector TraceStartLoc = StartLoc;
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceForObject;
	TraceForObject.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	float InnerSearchRad = SearchRadius;

	// Whether to show visual trace for debugging
	EDrawDebugTrace::Type DebugTrace = CVarDisplayTrace->GetBool() ?
		EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	// Perform the series of traces
	for (uint8 i = 0; i < NumTraces; i++)
	{
		// Find vector on a circle
		// TODO look into switching this for ukismet
		float RadianCalcAlt = FMath::GetMappedRangeValueClamped(
			UE::Math::TVector2<float>( 0.0f, 3.0f),
			UE::Math::TVector2<float>(0.0f, 4.7f),
			i);
		
		float RadianCalc = UKismetMathLibrary::MapRangeClamped(i,
			0.0f, 3.0f,
			0.0f, 4.7f );
		
		TraceStartLoc.X = StartLoc.X + InnerSearchRad * FMath::Cos(RadianCalc);  
		TraceStartLoc.Y = StartLoc.Y + InnerSearchRad * FMath::Sin(RadianCalc);
		FVector TraceEndLoc = TraceStartLoc - FVector(0.0f, 0.0f, 400.0f);
		FHitResult HitResult;

		UKismetSystemLibrary::SphereTraceSingleForObjects(
			GetWorld(),
			TraceStartLoc,
			TraceEndLoc,
			10.0f,
			TraceForObject,
			false,
			TArray<AActor*>(),
			DebugTrace,
			HitResult,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			10.0f);

		// Got one, time to go to the pub
		if (HitResult.IsValidBlockingHit())
		{
			ValidLoc = TraceStartLoc;
			return FVector::ZeroVector;
		}
	}
	return FVector::ZeroVector;
}