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
class ATantrumnHUD;

UCLASS()
class TANTRUMN_API ATantrumnPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** In Local MP it is needed to ensure the controller has receive the player in order to correctly set up the HUD */
	// TODO test this
	//virtual void ReceivedPlayer() override;

	/** Called from GameMode, only on Authority will get get these calls */
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** Called by Game Widget */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void OnRetrySelected();

	UFUNCTION(Client, Reliable, Category = "Tantrumn")
	void ClientRestartGame();
	
	/*
	 * TODO This needs to be named better, it just displays the end screen
	 * this will be separate, as it will come after the montage.
	 * Client gets HUD Authority needs to replicate the montage
	 */
	UFUNCTION(Client, Reliable, Category = "Tantrumn")
	void ClientReachedEnd();
	
	UFUNCTION(Server, Reliable, Category = "Tantrumn")
	void ServerRestartLevel();

protected:
	
	/** Class overrides */
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	
	/** Player HUD */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	ATantrumnHUD* PlayerHUD;
	
	/** Level Menu Display Requests */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void RequestDisplayLevelMenu();
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void RequestRemoveLevelMenu();

	UPROPERTY()
	ATantrumnGameStateBase* TantrumnGameState;
	bool CanProcessRequest() const;

	/** Enhanced Input Setup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enhanced Input", meta = (AllowPrivateAccess))
	TSoftObjectPtr<UInputMappingContext> GameInputMapping;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enhanced Input", meta = (AllowPrivateAccess))
	TSoftObjectPtr<UInputMappingContext> MenuInputMapping;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess))
	class UTantrumnInputConfigRegistry* InputActions;
	/** Call to change Input Mapping Contexts for Controller */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void SetInputContext(TSoftObjectPtr<UInputMappingContext> InMappingContext);
	
	/** Character Movement Requests */
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
	/** Character Look Inputs */
	UPROPERTY(EditAnywhere,Category = "CharacterMovement")
	float BaseLookPitchRate = 90.0f;
	UPROPERTY(EditAnywhere,Category = "CharacterMovement")
	float BaseLookYawRate = 90.0f;
	/** Base lookup rate, in deg/sec.  Other scaling may affect final lookup rate */
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookUpRate = 90.0f;
	/** Base look right rate, in deg/sec.  Other scaling may affect final lookup rate */
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookRightRate = 90.0f;

	/**
	 * Flick setup for throw action
	 * Used to determine flick of axis
	 * float LastDelta = 0.0f;
	 */
	float LastAxis = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Input")
	float FlickThreshold = 0.75;

	/** Sound Cue for Jumping */
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* JumpSound = nullptr;
};
