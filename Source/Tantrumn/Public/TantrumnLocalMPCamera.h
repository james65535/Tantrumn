// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TantrumnLocalMPCamera.generated.h"

class ATantrumnGameModeBase;
UCLASS()
class TANTRUMN_API ATantrumnLocalMPCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATantrumnLocalMPCamera(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** The camera component for this camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CamActor, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CamActor, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;
	UPROPERTY(EditAnywhere, Category = "Spring Arm", meta = (ClampMin = "0.0"))
	float MinArmLength = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Spring Arm", meta = (ClampMin = "0.0"))
	float MaxArmLength = 500.0f;
	UPROPERTY(EditAnywhere, Category = "Player Distance", meta = (ClampMin = "0.0"))
	float MinPlayerDistance = 100.0f;
	UPROPERTY(EditAnywhere, Category = "Player Distance", meta = (ClampMin = "0.0"))
	float MaxPlayerDistance = 1000.0f;

	ATantrumnGameModeBase* TantrumnGameModeBase;

};
