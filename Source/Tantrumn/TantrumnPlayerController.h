// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "Sound/SoundCue.h"
#include "TantrumnPlayerController.generated.h"

enum class ETantrumnGameType : uint8;
class UTantrumnGameElementsRegistry;
class UTantrumnGameWidget;
class ATantrumnGameStateBase;
class ATantrumnPlayerState;
class ATantrumnHUD;

/** To Specify What type of InputMode to Request */
UENUM(BlueprintType)
enum class ETantrumnInputMode : uint8
{
	GameOnly		UMETA(DisplayName = "GameOnly No Cursor"),
	GameAndUI		UMETA(DisplayName = "Game and UI With Cursor"),
	UIOnly		UMETA(DisplayName = "UI With Cursor"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStateReceived);

UCLASS()
class TANTRUMN_API ATantrumnPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** Called from GameMode, only on Authority will get get these calls */
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY()
	ATantrumnPlayerState* TantrumnPlayerState;
	UPROPERTY(BlueprintAssignable, Category = "Tantrumn")
	FOnPlayerStateReceived OnPlayerStateReceived;
	void SetTantrumnPlayerState(ATantrumnPlayerState* InPlayerState) { TantrumnPlayerState = InPlayerState; }
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	ATantrumnPlayerState* GetTantrumnPlayerState() const { return TantrumnPlayerState; }

	/** Called by Game Widget */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void OnRetrySelected();

	/** Called by Game Widget */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void OnReadySelected();
	/** Called by Client Ready */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Tantrumn")
	void S_OnReadySelected();

	/** Restart the level on client */
	UFUNCTION(Client, Reliable, Category = "Tantrumn")
	void C_ResetPlayer();
	UFUNCTION(Client, Reliable, Category = "Tantrumn")
	void C_StartGameCountDown(const float InCountDownDuration);
	
	void FinishedMatch();
	
	/** Get the final results and call hud to display */
	void RequestDisplayFinalResults() const;
	
	/** Called by Server Authority to restart level */
	UFUNCTION(Server, Reliable, Category = "Tantrumn")
	void S_RestartLevel();

	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void ConnectToServer(const FString InServerAddress);

	/**
	 * Set the controller input mode and cursor show
	 * @param InRequestedInputMode GameOnly / ShowCursor False  GameAndUI, UIOnly / Show Cursor True
	 */
	UFUNCTION(NetMulticast, Reliable, Category = "Tantrumn")
	void NM_SetControllerGameInputMode(const ETantrumnInputMode InRequestedInputMode);

	/** UFUNCTION Wrapper for parent class SetName method */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void SetPlayerName(const FString& InPlayerName);

protected:
	
	/** Class overrides */
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	
	UPROPERTY()
	ATantrumnGameStateBase* TantrumnGameState;

	
	/** Values Used for Display Match Time to the Player */
	FTimerHandle MatchClockDisplayTimerHandle;
	const float MatchClockDisplayRateSeconds = 0.1f;
	float CachedMatchStartTime = 0.0f;
	void HUDDisplayGameTimeElapsedSeconds() const;
	
	/** Player HUD */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	ATantrumnHUD* PlayerHUD;
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	UTantrumnGameElementsRegistry* GameElementsRegistry;
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void UpdateHUDWithGameUIElements(ETantrumnGameType InGameType);

	void SetControllerGameInputMode(const ETantrumnInputMode InRequestedInputMode);
	
	/** Level Menu Display Requests */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void RequestDisplayLevelMenu();
	UFUNCTION(BlueprintCallable, Category = "Tantrumn")
	void RequestHideLevelMenu();

	/** Delegate related to Game State match start of play */
	void StartMatchForPlayer(const float InMatchStartTime);

	/** Checks if player is allowed to input movement commands given current state of play */
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
	 * Flick setup for throw action.  Used to determine flick of axis
	 * float LastDelta = 0.0f;
	 */
	float LastAxis = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Input")
	float FlickThreshold = 0.75;

	/** Sound Cue for Jumping */
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* JumpSound = nullptr;
};
