// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TantrumnPlayerController.h"
#include "TantrumnCharMovementComponent.h"
#include "ThrowableActor.h"
#include "DrawDebugHelpers.h"

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

// Instantiate character defaults
ATantrumnCharacterBase::ATantrumnCharacterBase(
	const FObjectInitializer& ObjectInitializer) : Super(
		ObjectInitializer.SetDefaultSubobjectClass<UTantrumnCharMovementComponent>(
			ACharacter::CharacterMovementComponentName ) )
{
	// Setup reference for custom character movement component
	TantrumnCharMoveComp = Cast<UTantrumnCharMovementComponent>(GetCharacterMovement());
	
 	// Set this character to call Tick() every frame.
 	// You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATantrumnCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATantrumnCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check for stun as it will impact character movement
	if (TantrumnCharMoveComp->IsStunned())
	{
		return;
	}

	// Check for throw state in order to use appropriate animation montage
	if (CharacterThrowState == ECharacterThrowState::Throwing)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (UAnimMontage* CurrentAnimMontage = AnimInstance->GetCurrentActiveMontage())
			{
				const float PlayRate = AnimInstance->GetCurveValue(TEXT("ThrowCurve"));
				AnimInstance->Montage_SetPlayRate(CurrentAnimMontage, PlayRate);
			}
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

void ATantrumnCharacterBase::RequestSprintStart()
{
	TantrumnCharMoveComp->DoSprintStart();
	
}

void ATantrumnCharacterBase::RequestSprintEnd()
{
	TantrumnCharMoveComp->DoSprintEnd();
}

void ATantrumnCharacterBase::RequestStunStart(const float DurationMultiplier)
{
	if(!TantrumnCharMoveComp->OnStunStart.IsBoundToObject(this))
	{
		OnStunEndHandle = TantrumnCharMoveComp->OnStunEnd.AddUObject(this, &ATantrumnCharacterBase::OnStunEnd);
	}
	TantrumnCharMoveComp->DoStun(DurationMultiplier);
}

void ATantrumnCharacterBase::OnStunEnd() const
{
	TantrumnCharMoveComp->OnStunEnd.Remove(OnStunEndHandle);
	// TODO Add stun end logic here
	UE_LOG(LogTemp, Warning, TEXT("Tantrumn Character Stun Ended"))
}

// Called when landed from fall or jump
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
			RequestStunStart(MinImpactStunMultiplier);
		}
		else if (FallImpactSpeed >= MaxImpactSpeed)
		{
			// Stun the player to affect movement for a duration
			RequestStunStart(MaxImpactStunMultiplier);
			
			// Soundcue triggers
			if (HeavyLandSound && GetOwner())
			{
				FVector CharacterLocation = GetOwner()->GetActorLocation();
				UGameplayStatics::PlaySoundAtLocation(this, HeavyLandSound, CharacterLocation);
			}
		}

		// Controller Force Feedback
		// Calculate Severity of Impact
		const float DeltaImpact = MaxImpactSpeed - MinImpactSpeed;
		const float FallRatio = FMath::Clamp((FallImpactSpeed - MinImpactSpeed) / DeltaImpact, 0.0f, 1.0f);
		// Apply force feedback
		const bool bAffectSmall = FallRatio <= 0.5;
		const bool bAffectLarge = FallRatio > 0.5;
		PlayerController->PlayDynamicForceFeedback(FallRatio,
			0.5f,
			bAffectLarge, bAffectSmall,
			bAffectLarge, bAffectSmall);
	}
}

void ATantrumnCharacterBase::RequestThrowObject()
{
	if (CanThrowObject())
	{
		if (PlayThrowMontage())
		{
			CharacterThrowState = ECharacterThrowState::Throwing;
		}
		else
		{
			ResetThrowableObject();
		}
	}
}

void ATantrumnCharacterBase::RequestPullObjectStart()
{
	if (!IsStunned() && CharacterThrowState == ECharacterThrowState::None)
	{
		CharacterThrowState = ECharacterThrowState::RequestingPull;
	}
}

void ATantrumnCharacterBase::RequestPullObjectStop()
{
	// If we were pulling an object, drop it
	if (CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		CharacterThrowState = ECharacterThrowState::None;
		//ResetThrowableObject();
	}
}

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

void ATantrumnCharacterBase::OnThrowableAttached(AThrowableActor* InThrowableActor)
{
	CharacterThrowState = ECharacterThrowState::Attached;
	ThrowableActor = InThrowableActor;
	MoveIgnoreActorAdd(ThrowableActor);
}

void ATantrumnCharacterBase::SphereCastPlayerView()
{
	FVector Location;
	FRotator Rotation;
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

void ATantrumnCharacterBase::SphereCastActorTransform()
{
	FVector StartPos = GetActorLocation();
	FVector EndPos = StartPos + (GetActorForwardVector() * 1000.0f);

	// Sphere trace
	EDrawDebugTrace::Type DebugTrace = CVarDisplayTrace->GetBool() ?
		EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	FHitResult HitResult;
	UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		StartPos,
		EndPos,
		70.0f,
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
	FVector StartPos = GetActorLocation();
	FVector EndPos = StartPos + (GetActorForwardVector() * 1000.0f);
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
			HitResult.bBlockingHit ? FColor::Red : FColor::White,
			false);
	}
#endif
	ProcessTraceResult(HitResult);
}

void ATantrumnCharacterBase::ProcessTraceResult(const FHitResult& HitResult)
{
	// Check if there was an existing throwable object
	// Remove the highlight to avoid wrong feedback
	AThrowableActor* HitThrowableActor = HitResult.bBlockingHit ?
		Cast<AThrowableActor>(HitResult.GetActor()) : nullptr;
	const bool IsSameActor = (ThrowableActor == HitThrowableActor);
	const bool IsValidTarget = HitThrowableActor && HitThrowableActor->IsIdle();

	// Clean up previous target reference
	if (ThrowableActor)
	{
		if (!IsValidTarget || !IsSameActor)
		{
			ThrowableActor->ToggleHighlight(false);
			ThrowableActor = nullptr;
		}
	}

	// Check if hit object can be pulled
	if (IsValidTarget)
	{
		if (!ThrowableActor)
		{
			ThrowableActor = HitThrowableActor;
			ThrowableActor->ToggleHighlight(true);
		}
	}

	// Perform pull
	if (CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		// Don't allow for pulling objects while running/jogging
		if (GetVelocity().SizeSquared() < 100.0f)
		{
			if (ThrowableActor && ThrowableActor->Pull(this))
			{
				CharacterThrowState = ECharacterThrowState::Pulling;
				ThrowableActor = nullptr;
			}
		}
	}
}

// Configure delegates for montage play states: start/end to correlate behavior to animations
bool ATantrumnCharacterBase::PlayThrowMontage()
{
	// TODO implement if we decide not to allow movement during throw animation
	// if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	// {
	// 	Movement->SetMovementMode(EMovementMode::MOVE_None);
	// }
	const float PlayRate = 1.0f;
	bool bPlayedSuccessfully = PlayAnimMontage(ThrowMontage, PlayRate) > 0.0f;
	if (bPlayedSuccessfully)
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

	return bPlayedSuccessfully;
}

void ATantrumnCharacterBase::UnbindMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ATantrumnCharacterBase::OnNotifyBeginReceived);
		AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ATantrumnCharacterBase::OnNotifyEndReceived);
	}
}

void ATantrumnCharacterBase::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{

}

void ATantrumnCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UnbindMontage();
	CharacterThrowState = ECharacterThrowState::None;
	MoveIgnoreActorRemove(ThrowableActor);
	if (ThrowableActor->GetRootComponent())
	{
		UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(ThrowableActor->GetRootComponent());
		if (RootPrimitiveComponent)
		{
			RootPrimitiveComponent->IgnoreActorWhenMoving(this, false);
		}
	}
	ThrowableActor = nullptr;
}

void ATantrumnCharacterBase::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	// Ignore collisions
	// otherwise the throwable object hits the player capsule and doesn't travel in the desired direction
	if (ThrowableActor->GetRootComponent())
	{
		UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(ThrowableActor->GetRootComponent());
		if (RootPrimitiveComponent)
		{
			RootPrimitiveComponent->IgnoreActorWhenMoving(this, true);
		}
	}

	// TODO implement if we decide not to allow movement during throw animation
	// if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	// {
	// 	Movement->SetMovementMode(EMovementMode::MOVE_Walking);
	// }
	
	const FVector& Direction = GetActorForwardVector() * ThrowSpeed;
	ThrowableActor->Launch(Direction);

	if (CVarDisplayThrowVelocity->GetBool())
	{
		const FVector& Start = GetMesh()->GetSocketLocation(TEXT("ObjectAttach"));
		UE_LOG(LogTemp, Warning,
			TEXT("Velocity X: %f Velocity Y: %f Velocity Z: %f"), Direction.X, Direction.Y, Direction.Z);
		DrawDebugLine(GetWorld(), Start, Start + Direction, FColor::Red, false, 5.0f);
	}
}

void ATantrumnCharacterBase::OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{

}
