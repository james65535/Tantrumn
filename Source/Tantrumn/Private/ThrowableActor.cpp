// Fill out your copyright notice in the Description page of Project Settings.

#include "ThrowableActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "InteractInterface.h"
#include "Tantrumn/TantrumnCharacterBase.h"

// Sets default values
AThrowableActor::AThrowableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Allow network replication of movement data
	bReplicates = true;
	SetReplicateMovement(true);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));  // Ignores camera
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->SetUpdatedComponent(GetRootComponent());
		ProjectileMovementComponent->MaxSpeed = 3000.0f;
		ProjectileMovementComponent->bShouldBounce = true;
		ProjectileMovementComponent->Bounciness = 0.3f;
		ProjectileMovementComponent->bInterpMovement = true;
	}
}

bool AThrowableActor::IsIdle() const
{
	if (State == EState::Idle || State == EState::Dropped)
	{
		return true;
	}
	return false;
}

// Called when the game starts or when spawned
void AThrowableActor::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AThrowableActor::ProjectileStop);
}

void AThrowableActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	ProjectileMovementComponent->OnProjectileStop.RemoveDynamic(this, &AThrowableActor::ProjectileStop);
	Super::EndPlay(EndPlayReason);
}

// If object hits the pulling actor then attach it to the actor
void AThrowableActor::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	if (State == EState::Idle || State == EState::Attached || State == EState::Dropped)
	{ return; }

	// TODO implement different effects
	if (State == EState::Launch)
	{
		if (IInteractInterface* InteractObject = Cast<IInteractInterface>(Other))
		{ InteractObject->Execute_ApplyEffect(Other, EffectType, false); }

		const AActor* CurrentOwner = GetOwner();
		if (CurrentOwner && CurrentOwner != Other)
		{
			if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(Other))
			{ TantrumnCharacter->NotifyHitByThrowable(this); }
		}
	}

	
	// Ignore all other hits
	// This will wait until the projectile comes to a natural stop before returning it to idle
	if (PullActor && State == EState::Pull)
	{
		if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(PullActor))
		{
			if (Other == PullActor)
			{
				AttachToComponent(
					TantrumnCharacter->GetMesh(),
					FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					TEXT("ObjectAttach"));
				SetOwner(TantrumnCharacter);
				ProjectileMovementComponent->Deactivate();
				State = EState::Attached;
				TantrumnCharacter->OnThrowableAttached(this);
			}
			else
			{
				TantrumnCharacter->ResetThrowableObject();
				State = EState::Dropped;
			}
		}
	}

	ProjectileMovementComponent->HomingTargetComponent = nullptr;
	PullActor = nullptr;
}

// Return object state to idle when not doing anything
void AThrowableActor::ProjectileStop(const FHitResult& ImpactResult)
{
	if (State == EState::Launch || State == EState::Dropped)
	{ State = EState::Idle; }
}

// Configure object to home as a projectile to the pulling actor as a target
bool AThrowableActor::SetHomingTarget(AActor* Target)
{
	if (Target)
	{
		// Character should only have one ItemReceiver scene component
		if (USceneComponent* SceneComponent =
			Cast<USceneComponent>(
				Target->GetComponentsByTag(
					USceneComponent::StaticClass(),
				"ItemReceiver")[0]))
		{
			ProjectileMovementComponent->SetUpdatedComponent(GetRootComponent());
			ProjectileMovementComponent->Activate(true);
			ProjectileMovementComponent->HomingTargetComponent = TWeakObjectPtr<USceneComponent>(SceneComponent);
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = 2500.f; // Needed otherwise homing doesn't work

			// Set object into motion so homing can do it's thing
			// TODO replace velocity with static value with calculation based on distance to target
			// As currently the visual aspect is not smooth at distance extremities
			ProjectileMovementComponent->Velocity = FVector(0.0f, 0.0f, 500.0f);
			
			return true;
		}
	}

	return false;
}

bool AThrowableActor::Pull(AActor* InActor)
{
	// Do not allow pull if object is already doing something else
	if (State != EState::Idle)
	{ return false; }

	// If we can home to the puller then proceed with pull
	if (SetHomingTarget(InActor))
	{
		ToggleHighlight(false);
		State = EState::Pull;
		PullActor = InActor;
		return true;
	}

	// We have not met any conditions required to pull object
	return false;
}

// Throw the object
void AThrowableActor::Launch(const FVector& InitialVelocity, AActor* Target)
{
	// Launch object if not detached or idle
	if (State == EState::Pull || State == EState::Attached)
	{
		// Configure object to function as a projectile and detach from holding actor
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ProjectileMovementComponent->Activate(true);
		ProjectileMovementComponent->HomingTargetComponent = nullptr;

		State = EState::Launch;

		// If object has been called to be thrown at something in particular
		if (Target)
		{
			if (USceneComponent* SceneComponent = Cast<USceneComponent>(
				Target->GetComponentByClass(USceneComponent::StaticClass())))
			{
				ProjectileMovementComponent->HomingTargetComponent = TWeakObjectPtr<USceneComponent>(SceneComponent);
				return;
			}
		}
		ProjectileMovementComponent->Velocity = InitialVelocity;
	}
}

void AThrowableActor::Drop()
{
	if (State == EState::Attached || State == EState::Pull)
	{ DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); }

	ProjectileMovementComponent->Activate(true);
	ProjectileMovementComponent->Velocity = FVector(0.0f, 0.0f, ProjectileMovementComponent->GetGravityZ());
	ProjectileMovementComponent->HomingTargetComponent = nullptr;
	State = EState::Dropped;
}

void AThrowableActor::ToggleHighlight(bool bIsOn)
{
	if (StaticMeshComponent)
	{ StaticMeshComponent->SetRenderCustomDepth(bIsOn); }
}

EEffectType AThrowableActor::GetEffectType() const
{
	return EffectType;
}
