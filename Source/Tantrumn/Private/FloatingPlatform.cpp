// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingPlatform.h"

#include "NetTimeInterpToMovementComponent.h"

// Sets default values
AFloatingPlatform::AFloatingPlatform()
{
	bReplicates = true;
	bNetLoadOnClient = true;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneCompoent"));
	check(SceneRootComponent);

	RootComponent = SceneRootComponent;
	PlatformMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	PlatformMeshComponent->SetupAttachment(RootComponent);
	NetInterpToMovementComponent = CreateDefaultSubobject<UNetTimeInterpToMovementComponent>(TEXT("NetworkInterpMovementComponent"));
	NetInterpToMovementComponent->SetIsReplicated(true);

}

