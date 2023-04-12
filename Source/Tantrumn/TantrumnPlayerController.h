// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "Sound/SoundCue.h"
#include "TantrumnPlayerController.generated.h"

/**
 * 
 */

class ATantrumnGameModeBase;
class UUserWidget;

UCLASS()
class TANTRUMN_API ATantrumnPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void ReceivedPlayer() override; 

protected:

	// Game play start and cleanup
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
	ATantrumnGameModeBase* GameModeRef;

	// Enhanced Input Setup
	UPROPERTY(BlueprintReadOnly, Category = "Enhanced Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	class UTantrumnInputConfigRegistry* InputActions;

	// Character Movement Requests
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestJump();
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestStopJump();
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestCrouch();
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestStopCrouch();
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestSprint();
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestStopSprint();
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestMove(const FInputActionValue& ActionValue);
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestLook(const FInputActionValue& ActionValue);
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestThrowObject(const FInputActionValue& ActionValue);
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestPullObject();
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestStopPullObject();

	// Character Look Inputs
	UPROPERTY(EditAnywhere,Category = "CharacterMovement")
	float BaseLookPitchRate = 90.0f;
	UPROPERTY(EditAnywhere,Category = "CharacterMovement")
	float BaseLookYawRate = 90.0f;
	/** Base lookup rate, in deg/sec.  Other scaling may affect final lookup rate */
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookUpRate = 90.0f;
	/** Base lookright rate, in deg/sec.  Other scaling may affect final lookup rate */
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookRightRate = 90.0f;

	// Flick setup for throw action
	// Used to determine flick of axis
	// float LastDelta = 0.0f;
	float LastAxis = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Input")
	float FlickThreshold = 0.75;

	// Sound Cue for Jumping
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* JumpSound = nullptr;

	// Player HUD
	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<class UUserWidget> HUDClass;
	UPROPERTY()
	UUserWidget* HUDWidget;
	
};
