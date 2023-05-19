// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "Sound/SoundCue.h"
#include "TantrumnPlayerController.generated.h"

class UTantrumnGameWidget;
class UUserWidget;
class ATantrumnGameStateBase;

UCLASS()
class TANTRUMN_API ATantrumnPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	// In Local MP it is needed to ensure the controller has receive the player in order to correctly set up the HUD
	virtual void ReceivedPlayer() override;

	// Called from GameMode, only on Authority will get get these calls
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Called by Game Widget
	UFUNCTION(BlueprintCallable)
	void OnRetrySelected();

	UFUNCTION(Client, Reliable)
	void ClientDisplayCountDown(float GameCountDownDuration, TSubclassOf<UTantrumnGameWidget> InGameWidgetClass);

	UFUNCTION(Client, Reliable)
	void ClientRestartGame();

	/*
	 * This needs to be named better, it's just displayer the end screen
	 * this will be seperate, as it will come after the montage.
	 * Client gets HUD Authority needs to replicate the montage
	 */
	UFUNCTION(Client, Reliable)
	void ClientReachedEnd();

	UFUNCTION(Server, Reliable)
	void ServerRestartLevel();

protected:

	// Game play start and cleanup
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
	ATantrumnGameStateBase* TantrumnGameState;
	bool CanProcessRequest() const;

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
	void RequestHoldObject();
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestStopHoldObject();

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
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UUserWidget> HUDClass;
	UPROPERTY()
	UUserWidget* HUDWidget;

	// Player Game Widget
	UPROPERTY()
	UTantrumnGameWidget* TantrumnGameWidget;
	
};
