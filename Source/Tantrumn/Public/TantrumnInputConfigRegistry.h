// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnhancedInput/Public/InputAction.h"
#include "TantrumnInputConfigRegistry.generated.h"

/**
 * Enhanced Input Action Registry
 */

UCLASS()
class TANTRUMN_API UTantrumnInputConfigRegistry : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputLook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputJump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputCrouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputSprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputThrowObject;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputPullObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputOpenMenu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputCloseMenu;
};
