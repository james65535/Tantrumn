// Fill out your copyright notice in the Description page of Project Settings.


#include "NetTimeInterpToMovementComponent.h"

#include "TantrumnGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UNetTimeInterpToMovementComponent::UNetTimeInterpToMovementComponent()
{
	bUpdateOnlyIfRendered = false;
	bForceSubStepping = false;

	bWantsInitializeComponent = true;
	bAutoRegisterPhysicsVolumeUpdates = false;
	bComponentShouldUpdatePhysicsVolume = false;

	MaxSimulationTimeStep = 0.05f;
	MaxSimulationIterations = 8;

	bIsWaiting = false;
	TimeMultiplier = 1.0f;	
	Duration = 1.0f;
	CurrentDirection = 1.0f;
	CurrentTime = 0.0f;
	bStopped = false;
	bPointsFinalized = false;

}

void UNetTimeInterpToMovementComponent::BeginPlay()
{
	GameState = CastChecked<ATantrumnGameStateBase>(GetWorld()->GetGameState());
	CurrentTime = GameState->GetServerWorldTimeSeconds(); // 0.0f
	UE_LOG(LogTemp, Warning, TEXT("Got start time: %f"), NetworkStartTimeSeconds);
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().SetTimer(SlowTickTimerHandle,
				this, &UNetTimeInterpToMovementComponent::SlowTick,
				SlowTickerFrequencySeconds,
				true);
	}
	
	MARK_PROPERTY_DIRTY_FROM_NAME(UNetTimeInterpToMovementComponent, CurrentTime, this);
	MARK_PROPERTY_DIRTY_FROM_NAME(UNetTimeInterpToMovementComponent, CurrentDirection, this);
}
void UNetTimeInterpToMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams PushedParams;
	PushedParams.bIsPushBased = true;
	PushedParams.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS_FAST(UNetTimeInterpToMovementComponent, CurrentTime, PushedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UNetTimeInterpToMovementComponent, CurrentDirection, PushedParams);
}

// void UNetTimeInterpToMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
// {
// 	UE_LOG(LogTemp, Warning, TEXT("handleimpact, current time: %f"), CurrentTime);
// 	if( bPauseOnImpact == false )
// 	{
// 		switch(BehaviourType )
// 		{
// 		case EInterpToBehaviourType::OneShot:
// 			OnInterpToStop.Broadcast(Hit, TimeSlice);
// 			bStopped = true;
// 			StopSimulating(Hit);
// 			return;
// 		case EInterpToBehaviourType::OneShot_Reverse:
// 			if( CurrentDirection == -1.0f)
// 			{
// 				OnInterpToStop.Broadcast(Hit, TimeSlice);
// 				bStopped = true;
// 				StopSimulating(Hit);
// 				return;
// 			}
// 			else
// 			{
// 				ReverseDirection(Hit, TimeSlice, true);
// 			}
// 			break;
// 		case EInterpToBehaviourType::Loop_Reset:
// 			{
// 				CurrentTime = GameState->GetServerWorldTimeSeconds(); // TODO 0.0f
// 				OnResetDelegate.Broadcast(Hit, CurrentTime);
// 			}
// 			break;
// 		default:
// 			ReverseDirection(Hit, TimeSlice, true);
// 			break;
// 		}		
// 	}
// 	else
// 	{
// 		if( bIsWaiting == false )
// 		{
// 			OnWaitBeginDelegate.Broadcast(Hit, TimeSlice);
// 			bIsWaiting = true;
// 		}
// 	}
// }

// void UNetTimeInterpToMovementComponent::ReverseDirection(const FHitResult& Hit, float Time, bool InBroadcastEvent)
// {
// 	// Invert the direction we are moving 
// 	if (InBroadcastEvent == true)
// 	{
// 		OnInterpToReverse.Broadcast(Hit, Time);
// 	}
// 	// flip dir
// 	CurrentDirection = -CurrentDirection;
// 	MARK_PROPERTY_DIRTY_FROM_NAME(UNetTimeInterpToMovementComponent, CurrentDirection, this);
// 	MARK_PROPERTY_DIRTY_FROM_NAME(UNetTimeInterpToMovementComponent, CurrentTime, this);
// 	UE_LOG(LogTemp, Warning, TEXT("Platform reverse direction"));
// }

// void UNetTimeInterpToMovementComponent::RestartMovement(float InitialDirection)
// {
// 	CurrentDirection = InitialDirection;
// 	CurrentTime = 0.0f;
// 	bIsWaiting = false;
// 	bStopped = false;
// 	MARK_PROPERTY_DIRTY_FROM_NAME(UNetTimeInterpToMovementComponent, CurrentDirection, this);
// 	MARK_PROPERTY_DIRTY_FROM_NAME(UNetTimeInterpToMovementComponent, CurrentTime, this);
// }

void UNetTimeInterpToMovementComponent::SlowTick()
{
	UE_LOG(LogTemp, Warning, TEXT("SlowTick, current time: %f"), CurrentTime);
	MARK_PROPERTY_DIRTY_FROM_NAME(UNetTimeInterpToMovementComponent, CurrentTime, this);
	MARK_PROPERTY_DIRTY_FROM_NAME(UNetTimeInterpToMovementComponent, CurrentDirection, this);
}

void UNetTimeInterpToMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction* ThisTickFunction)
{
	// if (GetOwnerRole() != ROLE_Authority)
	// {
	// 	return;
	// }
	QUICK_SCOPE_CYCLE_COUNTER(STAT_InterpToMovementComponent_TickComponent);
	// skip if don't want component updated when not rendered or updated component can't move
	if (!UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	AActor* ActorOwner = UpdatedComponent->GetOwner();
	if (!ActorOwner || !CheckStillInWorld())
	{
		return;
	}

	if (UpdatedComponent->IsSimulatingPhysics())
	{
		return;
	}
	if((bStopped == true ) || ( !IsValid(ActorOwner) ) )
	{
		return;
	}
	if( ControlPoints.Num()== 0 ) 
	{
		return;
	}

	// This will update any control points coordinates that are linked to actors.
	UpdateControlPoints(false);

	float RemainingTime = DeltaTime;
	int32 NumBounces = 0;
	int32 Iterations = 0;
	FHitResult Hit(1.f);

	FVector WaitPos = FVector::ZeroVector;
	if (bIsWaiting == true)
	{
		WaitPos = UpdatedComponent->GetComponentLocation(); //-V595
	}
	while (RemainingTime >= MIN_TICK_TIME && (Iterations < MaxSimulationIterations) && IsValid(ActorOwner) && UpdatedComponent && IsActive())
	{
		Iterations++;

		const float TimeTick = ShouldUseSubStepping() ? GetSimulationTimeStep(RemainingTime, Iterations) : RemainingTime;
		RemainingTime -= TimeTick;

		// Calculate the current alpha with this tick iteration
		const float TargetTime = FMath::Clamp(CurrentTime + ((TimeTick*TimeMultiplier)*CurrentDirection), 0.0f, 1.0f);		
		FVector MoveDelta = ComputeMoveDelta(TargetTime);
		
		// Update velocity
		Velocity = MoveDelta / TimeTick;

		// Update the rotation on the spline if required
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); //-V595
		
		// Move the component
 		if ((bPauseOnImpact == false ) && (BehaviourType != EInterpToBehaviourType::OneShot))
 		{
 			// If we can bounce, we are allowed to move out of penetrations, so use SafeMoveUpdatedComponent which does that automatically.
 			SafeMoveUpdatedComponent(MoveDelta, CurrentRotation, bSweep, Hit, TeleportType);
 		}
 		else
		{
			// If we can't bounce, then we shouldn't adjust if initially penetrating, because that should be a blocking hit that causes a hit event and stop simulation.
			TGuardValue<EMoveComponentFlags> ScopedFlagRestore(MoveComponentFlags, MoveComponentFlags | MOVECOMP_NeverIgnoreBlockingOverlaps);
			MoveUpdatedComponent(MoveDelta, CurrentRotation, bSweep, &Hit, TeleportType);
		}
		//DrawDebugPoint(GetWorld(), UpdatedComponent->GetComponentLocation(), 16, FColor::White,true,5.0f);
		// If we hit a trigger that destroyed us, abort.
		if (!IsValid(ActorOwner) || !UpdatedComponent || !IsActive())
		{
			return;
		}

		// Update current time
		float AlphaRemainder = 0.0f;
		if (bIsWaiting == false)
		{
			// Compute time used out of tick time to get to the hit
			const float TimeDeltaAtHit = TimeTick * Hit.Time;
			// Compute new time lerp alpha based on how far we moved
			CurrentTime = CalculateNewTime(CurrentTime, TimeDeltaAtHit, Hit, true, bStopped, AlphaRemainder);
		}

		// See if we moved at all
		if (Hit.Time != 0.f)
		{
			// If we were 'waiting' we are not any more - broadcast we are moving again
			if (bIsWaiting == true)
			{
				OnWaitEndDelegate.Broadcast(Hit, CurrentTime);
				bIsWaiting = false;
			}
		}

		// Handle hit result after movement
		float SubTickTimeRemaining = 0.0f;
		if (!Hit.bBlockingHit)
		{
			if (bStopped == true)
			{
				Velocity = FVector::ZeroVector;
				break;
			}

			// Handle remainder of alpha after it goes off the end, for instance if ping-pong is set and it hit the end,
			// continue with the time remaining off the end but in the reverse direction. It is similar to hitting an object in this respect.
			if (AlphaRemainder != 0.0f)
			{
				NumBounces++;
				SubTickTimeRemaining = (AlphaRemainder * Duration);
			}
		}
		else
		{
			if (HandleHitWall(Hit, TimeTick, MoveDelta))
			{
				break;
			}

			NumBounces++;
			SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);
		}

		// A few initial bounces should add more time and iterations to complete most of the simulation.
		if (NumBounces <= 2 && SubTickTimeRemaining >= MIN_TICK_TIME)
		{
			RemainingTime += SubTickTimeRemaining;
			Iterations--;
		}
	}

	UpdateComponentVelocity();
}


