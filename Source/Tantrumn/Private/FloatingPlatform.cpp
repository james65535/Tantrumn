// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingPlatform.h"

#include "NetTimeInterpToMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Sets default values
AFloatingPlatform::AFloatingPlatform()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bNetLoadOnClient = true;
	//SetReplicatingMovement(true);
	//bOnlyRelevantToOwner = false;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneCompoent"));
	check(SceneRootComponent);
	//SceneRootComponent->SetIsReplicated(true);
	RootComponent = SceneRootComponent;
	PlatformMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	//PlatformMeshComponent->SetIsReplicated(true);
	PlatformMeshComponent->SetupAttachment(RootComponent);
	NetInterpToMovementComponent = CreateDefaultSubobject<UNetTimeInterpToMovementComponent>(TEXT("NetworkInterpMovementComponent"));
	NetInterpToMovementComponent->SetIsReplicated(true);

}

void AFloatingPlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = false;
	//SharedParams.RepNotifyCondition = REPNOTIFY_;
	//DOREPLIFETIME_WITH_PARAMS_FAST(AFloatingPlatform, CurrentState, SharedParams);
}

// Called when the game starts or when spawned
void AFloatingPlatform::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AFloatingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
