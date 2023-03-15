// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerController.h"
#include "GameFramework/Character.h"
#include "TantrumnInputConfigRegistry.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput//Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"


void ATantrumnPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (InputComponent)
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

			// Character locomotion
			EIPlayerComponent->BindAction(InputActions->InputMove,
				ETriggerEvent::Triggered,
				this,
				&ATantrumnPlayerController::RequestMove);

			// Character Look
			EIPlayerComponent->BindAction(InputActions->InputLook,
				ETriggerEvent::Triggered,
				this, &
				ATantrumnPlayerController::RequestLook);

			// Character Jump
			EIPlayerComponent->BindAction(InputActions->InputJump,
				ETriggerEvent::Triggered,
				this,
				&ATantrumnPlayerController::RequestJump);
			EIPlayerComponent->BindAction(InputActions->InputJump,
				ETriggerEvent::Completed,
				this,
				&ATantrumnPlayerController::EndRequestJump);

			// Character Crouch
			EIPlayerComponent->BindAction(InputActions->InputCrouch,
                ETriggerEvent::Triggered,
                this,
                &ATantrumnPlayerController::RequestCrouch);

			EIPlayerComponent->BindAction(InputActions->InputCrouch,
				ETriggerEvent::Completed,
				this,
				&ATantrumnPlayerController::EndRequestCrouch);

			// Character Sprint
			EIPlayerComponent->BindAction(InputActions->InputSprint,
				ETriggerEvent::Triggered,
				this,
				&ATantrumnPlayerController::RequestSprint);

			EIPlayerComponent->BindAction(InputActions->InputSprint,
				ETriggerEvent::Completed,
				this,
				&ATantrumnPlayerController::EndRequestSprint);
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
	if (ACharacter* ControlledChar = GetCharacter())
	{
		ControlledChar->Jump();
	}
}

void ATantrumnPlayerController::EndRequestJump()
{
	if (ACharacter* ControlledChar = GetCharacter())
	{
		ControlledChar->StopJumping();
	}
}

void ATantrumnPlayerController::RequestCrouch()
{
	if (ACharacter* ControlledChar = GetCharacter())
	{
		ControlledChar->Crouch();
	}
}

void ATantrumnPlayerController::EndRequestCrouch()
{
	if (ACharacter* ControlledChar = GetCharacter())
	{
		ControlledChar->UnCrouch();
	}
}

void ATantrumnPlayerController::RequestSprint()
{
	if (ACharacter* ControlledChar = GetCharacter())
	{
		ControlledChar->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SprintModifier;
	}
}

void ATantrumnPlayerController::EndRequestSprint()
{
	if (ACharacter* ControlledChar = GetCharacter())
	{
		ControlledChar->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void ATantrumnPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InputComponent)
	{
		// Prep Enhanced Input subsystem for mappings
		if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (!InputMapping.IsNull())
				{
					InputSystem->ClearAllMappings();
				}
			}
		}
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
