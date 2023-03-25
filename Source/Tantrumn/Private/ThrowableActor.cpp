// Fill out your copyright notice in the Description page of Project Settings.

#include "ThrowableActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Tantrumn/TantrumnCharacterBase.h"

// Sets default values
AThrowableActor::AThrowableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
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

void AThrowableActor::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	if (State == EState::Idle || State == EState::Attached || State == EState::Dropped)
	{
		return;
	}

	// Three options when hit
	// If in attached ignore

	// If not our actor is being pulled or launched
	// pulled we want to check that the hit is of the actor pulling
	// in which case it's a successful attach

	// If launched and hit a character that is not the launcher
	// do damage of whatever we want

	// Ignore all other hits
	
	// This will wait until the projectile comes to a natural stop before returning it to idle


	if (PullActor && State == EState::Pull)
	{
		if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(PullActor))
		{
			if (Other == PullActor)
			{
				AttachToComponent(TantrumnCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ObjectAttach"));
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

void AThrowableActor::ProjectileStop(const FHitResult& ImpactResult)
{
	if (State == EState::Launch || State == EState::Dropped)
	{
		State = EState::Idle;
	}
}

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
			ProjectileMovementComponent->HomingAccelerationMagnitude = 2500.f;
			ProjectileMovementComponent->Velocity = FVector(0.0f, 0.0f, 500.0f);
			//ProjectileMovementComponent->UpdateComponentVelocity(); // TODO added while troubleshooting lack of movement
			return true;
		}
	}

	return false;
}

bool AThrowableActor::Pull(AActor* InActor)
{
	if (State != EState::Idle)
	{
		return false;
	}

	if (SetHomingTarget(InActor))
	{
		ToggleHighlight(false);
		State = EState::Pull;
		PullActor = InActor;
		return true;
	}

	return false;
}

void AThrowableActor::Launch(const FVector& InitialVelocity, AActor* Target)
{
	if (State == EState::Pull || State == EState::Attached)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ProjectileMovementComponent->Activate(true);
		ProjectileMovementComponent->HomingTargetComponent = nullptr;

		State = EState::Launch;

		if (Target)
		{
			if (USceneComponent* SceneComponent = Cast<USceneComponent>(Target->GetComponentByClass(USceneComponent::StaticClass())))
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
	if (State == EState::Attached)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	ProjectileMovementComponent->Activate(true);
	ProjectileMovementComponent->HomingTargetComponent = nullptr;
	State = EState::Dropped;
}

void AThrowableActor::ToggleHighlight(bool bIsOn)
{
	StaticMeshComponent->SetRenderCustomDepth(bIsOn);
}
