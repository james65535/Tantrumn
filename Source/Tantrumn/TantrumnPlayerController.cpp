// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerController.h"
#include "GameFramework/Character.h"
#include "TantrumnInputConfigRegistry.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput//Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/InputActionValue.h"


void ATantrumnPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(InputComponent)
	{
		// Prep Enhanced Input subsystem for mappings
		if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (!InputMapping.IsNull())
				{
					InputSystem->ClearAllMappings();
					InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
				}
			}
		}

		// Get Input Component as Enhanced Input Component
		if (UEnhancedInputComponent* EIPlayerComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// Enhanced Input Action Bindings
			EIPlayerComponent->BindAction(InputActions->InputMove,
				ETriggerEvent::Triggered,
				this,
				&ATantrumnPlayerController::RequestMove);
			
			EIPlayerComponent->BindAction(InputActions->InputLook,
				ETriggerEvent::Triggered,
				this, &
				ATantrumnPlayerController::RequestLook);
			
			EIPlayerComponent->BindAction(InputActions->InputJump,
				ETriggerEvent::Triggered,
				this,
				&ATantrumnPlayerController::RequestJump);
		}
	}
}

void ATantrumnPlayerController::RequestMove(const FInputActionValue& ActionValue)
{
	const FVector2d MoveAction = ActionValue.Get<FVector2d>();
	const FRotator MoveRotation(0, GetControlRotation().Yaw, 0);

	// Movement for forwards and backwards
	if (MoveAction.Y != 0.f)
	{
		if (ACharacter* ControlledChar = GetCharacter())
		{
			const FVector Direction = MoveRotation.RotateVector(FVector::ForwardVector);
			ControlledChar->AddMovementInput(Direction, MoveAction.Y);
		}
	}

	// Movement for left and right
	if (MoveAction.X != 0.f)
	{
		if (ACharacter* ControlledChar = GetCharacter())
		{
			const FVector Direction = MoveRotation.RotateVector(FVector::RightVector);
			ControlledChar->AddMovementInput(Direction, MoveAction.X);
		}
	}
}

void ATantrumnPlayerController::RequestLook(const FInputActionValue& ActionValue)
{
	const FVector2d LookAction = ActionValue.Get<FVector2d>();

	// Look up and down
	if (LookAction.Y != 0.f)
	{
		if (ACharacter* ControlledChar = GetCharacter())
		{
			ControlledChar->AddControllerPitchInput(LookAction.Y);
		}
	}

	// Look left and right
	if (LookAction.X != 0.f)
	{
		if (ACharacter* ControlledChar = GetCharacter())
		{
			ControlledChar->AddControllerYawInput(LookAction.X);
		}
	}
}

void ATantrumnPlayerController::RequestJump()
{
	ACharacter* ControlledChar = GetCharacter();
	if (ControlledChar)
	{
		ControlledChar->Jump();
	}
}

void ATantrumnPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//TODO update for enhanced input
	if (InputComponent)
	{
		InputComponent->ClearActionBindings();
	}
}


/*


void ATantrumnPlayerController::RequestMoveX(float AxisValue)
{
	if  (AxisValue != 0.f)
	{
		if (GetCharacter())
		{
			GetCharacter()->AddMovementInput(GetCharacter()->GetActorForwardVector(),AxisValue);
		}
	}
}

void ATantrumnPlayerController::RequestMoveY(float AxisValue)
{
	if  (AxisValue != 0.f)
	{
		if (GetCharacter() && AxisValue != 0.f)
		{
			GetCharacter()->AddMovementInput(GetCharacter()->GetActorRightVector(),AxisValue);
		}
	}
}

void ATantrumnPlayerController::RequestLookPitch(float AxisValue)
{
	if  (AxisValue != 0.f)
	{
		AddPitchInput(AxisValue * BaseLookPitchRate * GetWorld()->DeltaTimeSeconds);
	}
}

void ATantrumnPlayerController::RequestLookYaw(float AxisValue)
{
	if  (AxisValue != 0.f)
	{
		AddYawInput(AxisValue * BaseLookYawRate * GetWorld()->DeltaTimeSeconds);
	}
}
*/
