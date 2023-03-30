// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

UENUM(BlueprintType)
enum class EEffectType : uint8
{
	NONE	UMETA(DisplayName = "None"),
	SPEED	UMETA(DisplayName = "SpeedBuff"),
	JUMP	UMETA(DisplayName = "JumpBuff"),
	POWER	UMETA(DisplayName = "PowerBuff"),
};
class TANTRUMN_API IInteractInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// *** EXAMPLES *** //
	// BlueprintCallable so that we can implement it via BlueprintNativeEvent so we can implement base functionality
	// into a C++ class
	// Typed function so we can implement it into a function in the BP class rather than an Event in EventGraph
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	const bool TypedInteract();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	void NonTypedInteract();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	void SignatureInteract(bool &Return);

	// *** Implementations *** //
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	void ApplyEffect(EEffectType EffectType, bool bIsBuff);
	// Lesson removes this to simplify work
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	//EEffectType UseEffect();
};
