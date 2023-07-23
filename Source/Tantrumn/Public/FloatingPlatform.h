// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FloatingPlatform.generated.h"

class UNetTimeInterpToMovementComponent;

UCLASS()
class TANTRUMN_API AFloatingPlatform : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFloatingPlatform();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tantrumn")
	USceneComponent* SceneRootComponent;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tantrumn")
	UStaticMeshComponent* PlatformMeshComponent;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tantrumn")
	UNetTimeInterpToMovementComponent* NetInterpToMovementComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
