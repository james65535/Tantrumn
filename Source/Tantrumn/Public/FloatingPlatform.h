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

	AFloatingPlatform();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tantrumn")
	USceneComponent* SceneRootComponent;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tantrumn")
	UStaticMeshComponent* PlatformMeshComponent;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tantrumn")
	UNetTimeInterpToMovementComponent* NetInterpToMovementComponent;

};
