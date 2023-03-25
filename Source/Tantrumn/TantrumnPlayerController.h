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

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	void RequestThrowObject(const FInputActionValue& ActionValue);  // TODO update these
	
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestPullObject();  // TODO update these

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestStopPullObject();  // TODO update these

	UPROPERTY(BlueprintReadWrite, Category = "CharacterMovement")
	float SprintModifier = 2.0f;

	UPROPERTY(BlueprintReadWrite, Category = "CharacterMovement")
	float WalkSpeed = 600.0f;
/*
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestMoveX(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestMoveY(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestLookYaw(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestLookPitch(float AxisValue);
	*/

	UPROPERTY(EditAnywhere,Category = "CharacterMovement")
	float BaseLookPitchRate = 90.0f;

	UPROPERTY(EditAnywhere,Category = "CharacterMovement")
	float BaseLookYawRate = 90.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Enhanced Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;	

	//class UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	class UTantrumnInputConfigRegistry* InputActions;
	
	// UPROPERTY(EditAnywhere, Category = "HUD")
	// TSubclassOf<class UUserWidget> HUDClass;
	
	//UPROPERTY()
	//UUserWidget* HUDWidget;

	/** Base lookup rate, in deg/sec.  Other scaling may affect final lookup rate */
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookUpRate = 90.0f;

	/** Base lookright rate, in deg/sec.  Other scaling may affect final lookup rate */
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookRightRate = 90.0f;

	/** Sound Cue for Jumping Sound */
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* JumpSound = nullptr;

	//ATantrumnGameModeBase* GameModeRef;

	// Used to determine flick of axis
	// float LastDelta = 0.0f;
	float LastAxis = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float FlickThreshold = 0.75;
	
};
