// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnCharacterBase.h"

#include "Kismet/GameplayStatics.h"
#include "TantrumnPlayerController.h"
#include "TantrumnCharMovementComponent.h"
#include "ThrowableActor.h"
#include "DrawDebugHelpers.h"
#include "TantrumnPlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "VisualLogger/VisualLogger.h"

constexpr int CVSphereCastPlayerView = 0;
constexpr int CVSphereCastActorTransform = 1;
constexpr int CVLineCastActorTransform = 2;

// CVars for visual debugging pulling action in runtime
static TAutoConsoleVariable<int> CVarTraceMode(
	TEXT("Tantrumn.Character.Debug.TraceMode"),
	0,
	TEXT("	0: Sphere cast PlayerView is used for direction/rotation (default).\n")
	TEXT("	1: Sphere cast using ActorTransform \n")
	TEXT("	2: Line cast using ActorTransform \n"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarDisplayTrace(
	TEXT("Tantrumn.Character.Debug.DisplayTrace"),
	false,
	TEXT("Display Trace"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarDisplayThrowVelocity(
	TEXT("Tantrumn.Character.Debug.DisplayThrowVelocity"),
	false,
	TEXT("Display Throw Velocity"),
	ECVF_Default);

// For Visual Logger
DEFINE_LOG_CATEGORY_STATIC(LogTantrumnChar, Verbose, Verbose)

// Instantiate character defaults
ATantrumnCharacterBase::ATantrumnCharacterBase(const FObjectInitializer& ObjectInitializer) : Super(
		ObjectInitializer.SetDefaultSubobjectClass<UTantrumnCharMovementComponent>(
			ACharacter::CharacterMovementComponentName ) )
{
	// Allow network replication of data
	bReplicates = true;
	
	// Setup reference for custom character movement component
	TantrumnCharMoveComp = Cast<UTantrumnCharMovementComponent>(GetCharacterMovement());
	
 	// Set this character to call Tick() every frame.
 	// You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ATantrumnCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams. Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnCharacterBase, CharacterThrowState, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnCharacterBase, bIsStunned, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnCharacterBase, bBeingRescued, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnCharacterBase, LastGroundPosition, SharedParams);
}

// Called when the game starts or when spawned
void ATantrumnCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	EffectCooldown = DefaultEffectCooldown;
	bIsStunned = false;
}

// Called every frame
void ATantrumnCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// This is done on the clients to ensure the anim looks correct
	// No need to spam network traffic with curve value
	if (CharacterThrowState == ECharacterThrowState::Throwing)
	{
		UpdateThrowMontagePlayRate();
		return;
	}
	
	if (IsBeingRescued() && HasAuthority() && GetLocalRole() != ROLE_AutonomousProxy)
	{
		UpdateRescue(DeltaTime);
		return;
	}

	// TODO review this
	// Exit before processing tick if this is a replica on a remote system
	// if (!IsLocallyControlled())
	// {
	// 	return;
	// }

	// Check for stun as it will impact character movement
	// if (TantrumnCharMoveComp->IsStunned())
	// {
	// 	return;
	// }

	// Check for effect status
	if(bIsUnderEffect)
	{
		if (EffectCooldown > 0)
		{
			EffectCooldown -= DeltaTime;
		}
		else
		{
			bIsUnderEffect = false;
			EffectCooldown = DefaultEffectCooldown;
			EndEffect();
		}
	}

	// If not throwing then let player perform a hit check to see if they can pull an object
	else if (CharacterThrowState == ECharacterThrowState::None ||
		CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		switch (CVarTraceMode->GetInt())
		{
		case CVSphereCastPlayerView:
			SphereCastPlayerView();
			break;
		case CVSphereCastActorTransform:
			SphereCastActorTransform();
			break;
		case CVLineCastActorTransform:
			LineCastActorTransform();
			break;
		default:
			SphereCastPlayerView();
			break;
		}
	}
}

// Called to bind functionality to input
void ATantrumnCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

#pragma region PullCastTracing

void ATantrumnCharacterBase::SphereCastActorTransform()
{
	const FVector StartPos = GetActorLocation();
	const FVector EndPos = StartPos + (GetActorForwardVector() * 1000.0f);

	// Sphere trace
	const EDrawDebugTrace::Type DebugTrace = CVarDisplayTrace->GetBool() ?
		EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	FHitResult HitResult;
	UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		StartPos,
		EndPos,
		80.0f,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		false,
		TArray<AActor*>(),
		DebugTrace,
		HitResult,
		true);
	ProcessTraceResult(HitResult);
}

void ATantrumnCharacterBase::LineCastActorTransform()
{
	const FVector StartPos = GetActorLocation();
	const FVector EndPos = StartPos + (GetActorForwardVector() * 1000.0f);
	FHitResult HitResult;
	GetWorld() ? GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartPos,
		EndPos,
		ECollisionChannel::ECC_Visibility) : false;
#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool())
	{
		DrawDebugLine(
			GetWorld(),
			StartPos,
			EndPos,
			HitResult.bBlockingHit ? FColor::Red : FColor::Green,
			false,-1,0,4);
	}
#endif
	ProcessTraceResult(HitResult);
}

void ATantrumnCharacterBase::ProcessTraceResult(const FHitResult& HitResult, bool bHighlight /* = true */)
{
	// Check if there was an existing throwable object
	// Remove the highlight to avoid wrong feedback
	AThrowableActor* HitThrowableActor = HitResult.bBlockingHit ? Cast<AThrowableActor>(HitResult.GetActor()) : nullptr;
	const bool IsSameActor = (ThrowableActor == HitThrowableActor);
	const bool IsValidTarget = HitThrowableActor && HitThrowableActor->IsIdle();

	// No need to process further if target is invalid
	if (ThrowableActor && (!IsValidTarget || !IsSameActor))
	{
		ThrowableActor->ToggleHighlight(false);
		ThrowableActor = nullptr;
	}

	if (!IsValidTarget)
	{
		return;
	}

	if (!IsSameActor)
	{
		ThrowableActor = HitThrowableActor;
		if (bHighlight)
		{
			ThrowableActor->ToggleHighlight(true);
		}
	}

	// Perform pull
	if (ThrowableActor && CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		// Do not allow for pulling objects while running / jogging
		if (GetVelocity().SizeSquared() < 100.0f)
		{
			ServerPullObject(ThrowableActor);
			CharacterThrowState = ECharacterThrowState::Pulling;
			ThrowableActor->ToggleHighlight(false);
		}
	}
}

void ATantrumnCharacterBase::SphereCastPlayerView()
{
	FVector Location;
	FRotator Rotation;
	
	if (!GetController())
	{
		return;
	}
	
	GetController()->GetPlayerViewPoint(Location, Rotation);
	const FVector PlayerViewForward = Rotation.Vector();
	const float AdditionalDistance = (Location - GetActorLocation()).Size();
	FVector EndPos = Location + (PlayerViewForward * (1000.0f + AdditionalDistance));

	const FVector CharacterForward = GetActorForwardVector();
	const float DotResult = FVector::DotProduct(PlayerViewForward, CharacterForward);
	// Prevent picking up objects behind us, this is when the camera is looking directly at the character's front side
	if(DotResult < -0.23f)
	{
		if (ThrowableActor)
		{
			ThrowableActor->ToggleHighlight(false);
			ThrowableActor = nullptr;
		}
		return;
	}

	FHitResult HitResult;
	EDrawDebugTrace::Type DebugTrace = CVarDisplayTrace->GetBool() ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		Location, EndPos,
		70.0f,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		false, ActorsToIgnore,
		DebugTrace,
		HitResult,
		true);
	ProcessTraceResult(HitResult);

#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool())
	{
		static float FovDeg = 90.0f;
		DrawDebugCamera(GetWorld(), Location, Rotation, FovDeg);
		DrawDebugLine(GetWorld(), Location, EndPos, HitResult.bBlockingHit ? FColor::Red : FColor::White);
		DrawDebugPoint(GetWorld(), EndPos, 70.0f, HitResult.bBlockingHit ? FColor::Red : FColor::White);
	}
#endif

}

#pragma endregion PullCastTracing

#pragma region Movement

void ATantrumnCharacterBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (!IsBeingRescued() &&
		(PrevMovementMode == MOVE_Walking && TantrumnCharMoveComp->MovementMode == MOVE_Falling))
	{
		LastGroundPosition =
			GetActorLocation() +(GetActorForwardVector() * -100.0f) +(GetActorUpVector() * 100.0f);
	
		// Ensure LastGroundPosition is a valid static location and not something which moves
		FVector ValidLoc;
		if(IsLandingValid(LastGroundPosition,ValidLoc,200.0f, 3))
		{
			LastGroundPosition = ValidLoc;
		}
		else
		{
			LastGroundPosition =
				GetActorLocation() +(GetActorForwardVector() * -600.0f) +(GetActorUpVector() * 100.0f);	
		}
	}

	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}

void ATantrumnCharacterBase::FellOutOfWorld(const UDamageType& dmgType)
{
	if (HasAuthority())
	{
		StartRescue();
	}
}

// TODO check if we do not already handle sprint replication
void ATantrumnCharacterBase::RequestSprintStart()
{
	if (!bIsStunned)
	{
		TantrumnCharMoveComp->DoSprintStart();
		ServerSprintStart();
	}
}

void ATantrumnCharacterBase::RequestSprintEnd()
{
	TantrumnCharMoveComp->DoSprintEnd();
	ServerSprintEnd();
}

void ATantrumnCharacterBase::ServerSprintStart_Implementation()
{
	TantrumnCharMoveComp->DoSprintStart();
}

void ATantrumnCharacterBase::ServerSprintEnd_Implementation()
{
	TantrumnCharMoveComp->DoSprintEnd();
}

void ATantrumnCharacterBase::RequestStunStart(const float DurationMultiplier)
{
	// Let character play with existing stun instead of stacking or resetting stun timers.
	if (bIsStunned)
	{
		return;
	}

	// Drop throwable object if it is held
	if (CharacterThrowState == ECharacterThrowState::Attached || CharacterThrowState == ECharacterThrowState::Aiming)
	{
		ResetThrowableObject();
	}

	// TODO check if we need to implement stop sprint state
	TantrumnCharMoveComp->RequestStun(false, DurationMultiplier);
}

void ATantrumnCharacterBase::OnRep_IsStunned()
{
	if (TantrumnCharMoveComp)
	{
		// Update simulated proxy clients
		if (bIsStunned)
		{
			TantrumnCharMoveComp->RequestStun(true, MaxImpactStunMultiplier);
		}
		else if (!bIsStunned)
		{
			TantrumnCharMoveComp->RequestStunEnd(true);
		}
	}
}

void ATantrumnCharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Apply Controller Force FeedBack and stun player if impact speed exceeds threshold
	if(ATantrumnPlayerController* PlayerController = GetController<ATantrumnPlayerController>())
	{
		// Determine effects of fall based upon speed
		const float FallImpactSpeed = FMath::Abs(GetVelocity().Z);
		UE_LOG(LogTemp, Warning, TEXT("Character Landed with Fall Impact Speed: %f"), FallImpactSpeed);
		if (FallImpactSpeed < MinImpactSpeed)
		{
			// Nothing to do, light fall
			return;
		}
		else if (FallImpactSpeed >= MinImpactSpeed && FallImpactSpeed < MaxImpactSpeed)
		{
			// Stun the player to affect movement for a duration
			const float Duration = FMath::Clamp(MinImpactSpeed / FallImpactSpeed, 0.0f, 1.0f);
			RequestStunStart(Duration * MinImpactStunMultiplier);
		}
		else if (FallImpactSpeed >= MaxImpactSpeed)
		{
			// Stun the player to affect movement for a duration
			RequestStunStart(MaxImpactStunMultiplier);
			
			// Soundcue triggers
			if (HeavyLandSound && GetOwner())
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HeavyLandSound,
					GetOwner()->GetActorLocation());
			}
		}

		// Controller Force Feedback
		// Calculate Severity of Impact
		const float DeltaImpact = MaxImpactSpeed - MinImpactSpeed;
		const float FallRatio = FMath::Clamp((FallImpactSpeed - MinImpactSpeed) / DeltaImpact, 0.0f, 1.0f);

		// Apply force feedback
		const bool bAffectSmall = FallRatio <= 0.5;
		const bool bAffectLarge = FallRatio > 0.5;
		PlayerController->PlayDynamicForceFeedback(
			FallRatio,
			0.5f,
			bAffectLarge, bAffectSmall,
			bAffectLarge, bAffectSmall);
	}
}

void ATantrumnCharacterBase::UpdateRescue(float DeltaTime)
{
	CurrentRescueTime += DeltaTime;
	const float Alpha = FMath::Clamp(CurrentRescueTime / TimeToRescuePlayer, 0.0f, 1.0f);
	const FVector NewPlayerLocation = UKismetMathLibrary::VLerp(FallOutOfWorldPosition, LastGroundPosition, Alpha);
	SetActorLocation(NewPlayerLocation);

	if (HasAuthority() && Alpha >= 1.0f)
	{
			EndRescue();
	}
}

void ATantrumnCharacterBase::StartRescue()
{
	// This will be broadcasted, don't want to potentially start moving to a bad location
	FallOutOfWorldPosition = GetActorLocation();
	bBeingRescued = true;
	OnBeingRescuedEvent.Broadcast();
	CurrentRescueTime = 0.0f;
	TantrumnCharMoveComp->Deactivate();
	SetActorEnableCollision(false);
}

void ATantrumnCharacterBase::OnRep_IsBeingRescued()
{
	if (bBeingRescued)
	{
		StartRescue();
	}
	else
	{
		EndRescue();
	}
}

void ATantrumnCharacterBase::EndRescue()
{
	// Authority will dictate when this is over
	bBeingRescued = false;
	TantrumnCharMoveComp->Activate();
	SetActorEnableCollision(true);
	CurrentRescueTime = 0.0f;
}

bool ATantrumnCharacterBase::IsHovering() const
{
	if (const ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
	{
		return TantrumnPlayerState->GetCurrentState() == EPlayerGameState::Waiting;
	}

	return false;
}

#pragma endregion Movement

#pragma region PullAction

void ATantrumnCharacterBase::RequestPullObjectStart()
{
	// Ensure we are in idle
	if (!bIsStunned && CharacterThrowState == ECharacterThrowState::None)
	{
		CharacterThrowState = ECharacterThrowState::RequestingPull;
		ServerRequestPullObject(true);
	}
}

void ATantrumnCharacterBase::RequestPullObjectStop()
{
	// If we were pulling an object, drop it
	if (CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		CharacterThrowState = ECharacterThrowState::None;
		ServerRequestPullObject(false);
	}
}

void ATantrumnCharacterBase::ServerRequestPullObject_Implementation(bool bIsPulling)
{
	CharacterThrowState = bIsPulling ? ECharacterThrowState::RequestingPull : ECharacterThrowState::None;
}

void ATantrumnCharacterBase::ServerPullObject_Implementation(AThrowableActor* InThrowableActor)
{
	if (InThrowableActor && InThrowableActor->Pull(this))
	{
		CharacterThrowState = ECharacterThrowState::Pulling;
		ThrowableActor = InThrowableActor;
		ThrowableActor->ToggleHighlight(false);
	}
}

void ATantrumnCharacterBase::OnThrowableAttached(AThrowableActor* InThrowableActor)
{
	CharacterThrowState = ECharacterThrowState::Attached;
	ThrowableActor = InThrowableActor;
	MoveIgnoreActorAdd(ThrowableActor);
	ClientThrowableAttached(InThrowableActor);
}

void ATantrumnCharacterBase::ClientThrowableAttached_Implementation(AThrowableActor* InThrowableActor)
{
	CharacterThrowState = ECharacterThrowState::Attached;
	ThrowableActor = InThrowableActor;
	MoveIgnoreActorAdd(ThrowableActor);
}

bool ATantrumnCharacterBase::AttemptPullObjectAtLocation(const FVector& InLocation)
{
	if (CharacterThrowState != ECharacterThrowState::None && CharacterThrowState != ECharacterThrowState::RequestingPull)
	{
		return false;
	}

	const FVector StartPos = GetActorLocation();
	//FVector EndPos = InLocation;
	FHitResult HitResult;
	GetWorld() ? GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartPos,
		InLocation,
		ECollisionChannel::ECC_Visibility) : false;
#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool())
	{
		DrawDebugLine(
			GetWorld(),
			StartPos,
			InLocation,
			HitResult.bBlockingHit ? FColor::Red : FColor::White,
			false);
	}
#endif

	CharacterThrowState = ECharacterThrowState::RequestingPull;

	// This will alter CharacterThrowState
	ProcessTraceResult(HitResult, false);
	
	if (CharacterThrowState == ECharacterThrowState::Pulling)
	{
		return true;
	}
	CharacterThrowState = ECharacterThrowState::None;
	return false;
}

#pragma endregion PullAction

#pragma region AimAction

void ATantrumnCharacterBase::RequestAim()
{
	if (!bIsStunned && CharacterThrowState == ECharacterThrowState::Attached)
	{
		CharacterThrowState = ECharacterThrowState::Aiming;
		ServerRequestToggleAim(true);
	}
}

void ATantrumnCharacterBase::ServerRequestToggleAim_Implementation(bool IsAiming)
{
	CharacterThrowState = IsAiming ? ECharacterThrowState::Aiming : ECharacterThrowState::Attached;
}

void ATantrumnCharacterBase::RequestStopAim()
{
	if (CharacterThrowState == ECharacterThrowState::Aiming)
	{
		CharacterThrowState = ECharacterThrowState::Attached;
		ServerRequestToggleAim(false);
	}
}

#pragma endregion AimAction

#pragma region ThrowAction

void ATantrumnCharacterBase::RequestThrowObject()
{
	if (CanThrowObject())
	{
		if (PlayThrowMontage())
		{
			CharacterThrowState = ECharacterThrowState::Throwing;

			// For network play
			ServerRequestThrowObject();
		}
		else
		{
			ResetThrowableObject();
		}
	}
}

bool ATantrumnCharacterBase::ServerRequestThrowObject_Validate()
{
	// Check the state or if the throwable actor exists etc to prevent this from being broadcast
	return true;
}

void ATantrumnCharacterBase::ServerRequestThrowObject_Implementation()
{
	// Server needs to call the multicast
	MulticastRequestThrowObject();
}

void ATantrumnCharacterBase::MulticastRequestThrowObject_Implementation()
{
	// Locally controlled actor already has the binding and played the montage
	if (IsLocallyControlled())
	{
		return;
	}

	PlayThrowMontage();
	CharacterThrowState = ECharacterThrowState::Throwing;
}

void ATantrumnCharacterBase::ServerBeginThrow_Implementation()
{
	// Ignore collisions otherwise the throwable object hits the player capsule
	// and doesn't travel in the desired direction
	if (ThrowableActor->GetRootComponent())
	{
		if (UPrimitiveComponent* RootPrimitiveComponent =
			Cast<UPrimitiveComponent>(ThrowableActor->GetRootComponent()))
		{
			RootPrimitiveComponent->IgnoreActorWhenMoving(this, true);
		}
	}
	
	const FVector& Direction = GetActorForwardVector() * ThrowSpeed;
	ThrowableActor->Launch(Direction);

	if (CVarDisplayThrowVelocity->GetBool())
	{
		const FVector& Start = GetMesh()->GetSocketLocation(TEXT("ObjectAttach"));
		DrawDebugLine(GetWorld(), Start, Start + Direction, FColor::Red, false, 5.0f);
	}

	const FVector& Start = GetMesh()->GetSocketLocation(TEXT("ObjectAttach"));
	UE_VLOG_ARROW(this, LogTantrumnChar, Verbose, Start, Start + Direction, FColor::Red, TEXT("Throw Direction"));
}

void ATantrumnCharacterBase::ServerFinishThrow_Implementation()
{
	CharacterThrowState = ECharacterThrowState::None;
	// This only happens on the locally controlled actor
	MoveIgnoreActorRemove(ThrowableActor);
	if (ThrowableActor->GetRootComponent())
	{
		if (UPrimitiveComponent* RootPrimitiveComponent =
			Cast<UPrimitiveComponent>(ThrowableActor->GetRootComponent()))
		{
			RootPrimitiveComponent->IgnoreActorWhenMoving(this, false);
		}
	}
	ThrowableActor = nullptr;
}

// TODO Implement Replication
void ATantrumnCharacterBase::ResetThrowableObject()
{
	// Drop object
	if (ThrowableActor)
	{
		ThrowableActor->Drop();
	}
	CharacterThrowState = ECharacterThrowState::None;
	ThrowableActor = nullptr;
}

// TODO remove logs
void ATantrumnCharacterBase::OnRep_CharacterThrowState(const ECharacterThrowState& OldCharacterThrowState)
{
	if (CharacterThrowState != OldCharacterThrowState)
	{
		if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("Simultated proxy throw"));
		}
		else if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("Autonomous proxy throw"));
		}
		else if (GetNetMode() == ENetMode::NM_DedicatedServer)
		{
			UE_LOG(LogTemp, Warning, TEXT("Dedicated Authority proxy throw"));
		}
		UE_LOG(LogTemp, Warning, TEXT("OldThrowState: %s"),*UEnum::GetDisplayValueAsText(OldCharacterThrowState).ToString());
		UE_LOG(LogTemp, Warning, TEXT("CharacterThrowState: %s"), *UEnum::GetDisplayValueAsText(CharacterThrowState).ToString());
	}
}

#pragma endregion ThrowAction

#pragma region Montage

// Configure delegates for montage play states: start/end to correlate behavior to animations
bool ATantrumnCharacterBase::PlayThrowMontage()
{
	const bool bPlayedSuccessfully = PlayAnimMontage(ThrowMontage, 1.0f) > 0.0f;
	if (bPlayedSuccessfully)
	{
		// Run this for local system
		if (IsLocallyControlled())
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			if (!BlendingOutDelegate.IsBound())
			{
				BlendingOutDelegate.BindUObject(this, &ATantrumnCharacterBase::OnMontageBlendingOut);
			}
			AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ThrowMontage);

			if (!MontageEndedDelegate.IsBound())
			{
				MontageEndedDelegate.BindUObject(this, &ATantrumnCharacterBase::OnMontageEnded);
			}
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ThrowMontage);

			AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ATantrumnCharacterBase::OnNotifyBeginReceived);
			AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ATantrumnCharacterBase::OnNotifyEndReceived);
		}
	}
	return bPlayedSuccessfully;
}

bool ATantrumnCharacterBase::PlayCelebrateMontage()
{
	const bool bPlayedSuccessfully = PlayAnimMontage(CelebrateMontage, 1.0f) > 0.f;
	if (bPlayedSuccessfully)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (!MontageEndedDelegate.IsBound())
		{
			MontageEndedDelegate.BindUObject(this, &ATantrumnCharacterBase::OnMontageEnded);
		}

		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, CelebrateMontage);
	}

	return bPlayedSuccessfully;
}

void ATantrumnCharacterBase::NM_FinishedMatch_Implementation()
{
	if (ATantrumnPlayerController* TantrumnPlayerController = Cast<ATantrumnPlayerController>(GetController()))
	{
		TantrumnPlayerController->FinishedMatch();
	}
	GetCharacterMovement()->DisableMovement();
	PlayCelebrateMontage();
}

void ATantrumnCharacterBase::UpdateThrowMontagePlayRate()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (const UAnimMontage* CurrentAnimMontage = AnimInstance->GetCurrentActiveMontage())
		{
			// Speed up the playrate when at the throwing part of the animation.
			// The initial interaction animation wasn't intended as a throw so it is slow
			const float PlayRate = AnimInstance->GetCurveValue(TEXT("ThrowCurve"));
			AnimInstance->Montage_SetPlayRate(CurrentAnimMontage, PlayRate);
		}
	}
}

void ATantrumnCharacterBase::UnbindMontage()
{
	if (IsLocallyControlled())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ATantrumnCharacterBase::OnNotifyBeginReceived);
			AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ATantrumnCharacterBase::OnNotifyEndReceived);
		}
	}
}

void ATantrumnCharacterBase::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{

}

void ATantrumnCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (IsLocallyControlled())
	{
		UnbindMontage();
	}

	if (Montage == ThrowMontage)
	{
		if (IsLocallyControlled())
		{
			CharacterThrowState = ECharacterThrowState::None;
			ServerFinishThrow();
			ThrowableActor = nullptr;
		}
	}
	// TODO Re-implement winner celebration
	else if (Montage == CelebrateMontage)
	{
		if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
		{
			if (TantrumnPlayerState->IsWinner())
			{
				PlayAnimMontage(CelebrateMontage, 1.0f, TEXT("Winner"));
			}
		}
	}
}

// Prep and call launch on throwable object when character attempts to throw
void ATantrumnCharacterBase::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	ServerBeginThrow();
}

void ATantrumnCharacterBase::OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{

}

#pragma endregion Montage

void ATantrumnCharacterBase::ApplyEffect_Implementation(EEffectType EffectType, bool bIsBuff)
{
	// Prevents multiple buffs stacking
	if (bIsUnderEffect) return;

	CurrentEffect = EffectType;
	bIsUnderEffect = true;
	bIsEffectBuff = bIsBuff;

	switch (CurrentEffect)
	{
	case EEffectType::SPEED :
		if(bIsEffectBuff)
		{
			TantrumnCharMoveComp->MaxWalkSpeed *= 2;
		} else
		{
			TantrumnCharMoveComp->DisableMovement();
		}

	default:
		break;
	}
}

void ATantrumnCharacterBase::EndEffect()
{
	bIsUnderEffect = false;

	switch(CurrentEffect)
	{
		case EEffectType::SPEED :
		{
			if(bIsEffectBuff)
			{
				TantrumnCharMoveComp->MaxWalkSpeed /= 2, RequestSprintEnd();
			}
				else
			{
				TantrumnCharMoveComp->SetMovementMode(MOVE_Walking);
			}
			break;
		}
		default: break;
	}
}

void ATantrumnCharacterBase::NotifyHitByThrowable(AThrowableActor* InThrowable)
{
	RequestStunStart(MaxImpactStunMultiplier);
	ResetThrowableObject();
}

void ATantrumnCharacterBase::RequestUseObject()
{
	ApplyEffect(ThrowableActor->GetEffectType(), true);
	ThrowableActor->Destroy();
	ResetThrowableObject();
}

bool ATantrumnCharacterBase::IsLandingValid(FVector StartLoc, FVector& ValidLoc, float SearchRadius , uint8 NumRetries)
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
			return true;
		}
	}

	// Found nothing so we give up and bail out
	if (NumRetries <= 0)
	{
		return false;
	}

	// Prep for next search
	SearchRadius *= 2.0f;
	NumRetries--;
	
	return(IsLandingValid(StartLoc, ValidLoc, SearchRadius, NumRetries));
}