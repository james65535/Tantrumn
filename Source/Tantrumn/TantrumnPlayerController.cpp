// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerController.h"
#include "TantrumnCharacterBase.h"
#include "TantrumnInputConfigRegistry.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput//Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/InputActionValue.h"

static TAutoConsoleVariable<bool> CVarDisplayLaunchInputDelta(
	TEXT("Tantrumn.Character.Debug.DisplayLaunchInputdelta"),
	false,
	TEXT("Display Launch Input Delta"),
	ECVF_Default);

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
				&ATantrumnPlayerController::RequestStopJump);

			// Character Crouch
			EIPlayerComponent->BindAction(InputActions->InputCrouch,
                ETriggerEvent::Triggered,
                this,
                &ATantrumnPlayerController::RequestCrouch);

			EIPlayerComponent->BindAction(InputActions->InputCrouch,
				ETriggerEvent::Completed,
				this,
				&ATantrumnPlayerController::RequestStopCrouch);

			// Character Sprint
			EIPlayerComponent->BindAction(InputActions->InputSprint,
				ETriggerEvent::Triggered,
				this,
				&ATantrumnPlayerController::RequestSprint);

			EIPlayerComponent->BindAction(InputActions->InputSprint,
				ETriggerEvent::Completed,
				this,
				&ATantrumnPlayerController::RequestStopSprint);

			// Character Throw
			EIPlayerComponent->BindAction(InputActions->InputThrowObject,
				ETriggerEvent::Triggered,
				this,
				&ATantrumnPlayerController::RequestThrowObject);

			// Character Pull
			EIPlayerComponent->BindAction(InputActions->InputPullObject,
				ETriggerEvent::Triggered,
				this,
				&ATantrumnPlayerController::RequestPullObject);  // TODO Update

			EIPlayerComponent->BindAction(InputActions->InputPullObject,
				ETriggerEvent::Completed,
				this,
				&ATantrumnPlayerController::RequestStopPullObject);  // TODO Update
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
		if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
		{
			const FVector Direction = MoveRotation.RotateVector(FVector::ForwardVector);
			TantrumnCharacter->AddMovementInput(Direction, MoveAction.Y);
		}
	}

	// Movement for left and right
	if (MoveAction.X != 0.f)
	{
		if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
		{
			const FVector Direction = MoveRotation.RotateVector(FVector::RightVector);
			TantrumnCharacter->AddMovementInput(Direction, MoveAction.X);
		}
	}
}

void ATantrumnPlayerController::RequestLook(const FInputActionValue& ActionValue)
{
	const FVector2d LookAction = ActionValue.Get<FVector2d>();

	// Look up and down
	if (LookAction.Y != 0.f)
	{
		if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
		{
			TantrumnCharacter->AddControllerPitchInput(LookAction.Y);
		}
	}

	// Look left and right
	if (LookAction.X != 0.f)
	{
		if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
		{
			TantrumnCharacter->AddControllerYawInput(LookAction.X);
		}
	}
}

void ATantrumnPlayerController::RequestJump()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->Jump();
	}
}

void ATantrumnPlayerController::RequestStopJump()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->StopJumping();
	}
}

void ATantrumnPlayerController::RequestCrouch()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->Crouch();
	}
}

void ATantrumnPlayerController::RequestStopCrouch()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->UnCrouch();
	}
}

void ATantrumnPlayerController::RequestSprint()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->RequestSprintStart();
	}
}

void ATantrumnPlayerController::RequestStopSprint()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->RequestSprintEnd();
	}
}

void ATantrumnPlayerController::RequestThrowObject(const FInputActionValue& ActionValue)
{
	const float AxisValue = ActionValue.Get<float>();  // TODO check this works as intended
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		if (TantrumnCharacter->CanThrowObject())
		{
			float CurrentDelta = AxisValue - LastAxis; // TODO this might now be right

			// Debug
			if (CVarDisplayLaunchInputDelta->GetBool())
			{
				if (fabs(CurrentDelta) > 0.0f)
				{
					UE_LOG(LogTemp, Warning, TEXT("Axis: %f CurrentDelta: %f"), AxisValue, LastAxis);
				}
			}
			LastAxis = AxisValue;
			const bool IsFlick = fabs(CurrentDelta) > FlickThreshold;
			if (IsFlick)
			{
				TantrumnCharacter->RequestThrowObject();
			}
		}
		else
		{
			LastAxis = 0.0f;
		}
	}
}

void ATantrumnPlayerController::RequestPullObject()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->RequestPullObjectStart();
	}
}

void ATantrumnPlayerController::RequestStopPullObject()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->RequestPullObjectStop();
	}
}

void ATantrumnPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InputComponent)
	{
		// Tear down Enhanced Input subsystem for mappings
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