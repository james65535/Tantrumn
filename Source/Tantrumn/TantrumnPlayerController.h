// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "TantrumnPlayerController.generated.h"

/**
 * 
 */


UCLASS()
class TANTRUMN_API ATantrumnPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestJump();

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestMove(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	void RequestLook(const FInputActionValue& ActionValue);
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

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;	

	//class UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	class UTantrumnInputConfigRegistry* InputActions;
	
	
};
