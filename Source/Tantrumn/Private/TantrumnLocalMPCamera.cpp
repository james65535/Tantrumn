// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnLocalMPCamera.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Tantrumn/TantrumnGameModeBase.h"

static TAutoConsoleVariable<bool> CVarDrawMidPoint(
	TEXT("Tantrumn.Camera.Debug.DrawMidPoint"),
	true,
	TEXT("Draw midPoint Between Players"),
	ECVF_Default);

// Sets default values
ATantrumnLocalMPCamera::ATantrumnLocalMPCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	// Make the scene component the root
	RootComponent = SpringArmComponent;

	// Camera Defaults
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->FieldOfView = 90.0f;
	CameraComponent->bConstrainAspectRatio = true;
	CameraComponent->AspectRatio = 1.77778f;
	CameraComponent->PostProcessBlendWeight = 1.0f;
	CameraComponent->SetupAttachment(SpringArmComponent);
}

// Called when the game starts or when spawned
void ATantrumnLocalMPCamera::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(GetWorld(), TEXT("ATantrumnLocalMPCamera::BeginPlay() Missing World!"));
	TantrumnGameModeBase = Cast<ATantrumnGameModeBase>(GetWorld()->GetAuthGameMode());
	
}

// Called every frame
void ATantrumnLocalMPCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Allows Local Multiplayer Camera to zoom out to cover all players
	float MaxDistanceSq = 0.0f;
	FVector MidPoint = FVector::ZeroVector;
	FVector LastPosition = FVector::ZeroVector;
	int NumPlayers = 0;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState)
		{
			FVector PawnPosition = PlayerController->GetPawn()->GetActorLocation();
			if (!LastPosition.IsNearlyZero())
			{
				const float DistanceSq = (PawnPosition - LastPosition).SizeSquared();
				if (DistanceSq > MaxDistanceSq)
				{
					MaxDistanceSq = DistanceSq;
				}
			}
			MidPoint += PawnPosition;
			LastPosition = PawnPosition;
			++NumPlayers;
		}
	}

	MidPoint /= NumPlayers > 0 ? (float)NumPlayers : 1.0f;

	if (CVarDrawMidPoint->GetBool())
	{
		DrawDebugSphere(GetWorld(), MidPoint, 25.0f, 10, FColor::Blue);
	}
	
	const float MaxDistance = MaxDistanceSq > KINDA_SMALL_NUMBER ?
		FMath::Min(sqrtf(MaxDistanceSq), MaxPlayerDistance) : 0.0f;
	const float DistanceRatio = MaxDistance > MinPlayerDistance ?
		(MaxDistance - MinPlayerDistance) / (MaxPlayerDistance - MinPlayerDistance): 0.0f;
	SpringArmComponent->TargetArmLength = FMath::Lerp(MinArmLength, MaxArmLength, DistanceRatio);
	UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), MidPoint);
	
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), MidPoint));
}

