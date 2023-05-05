// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerController.h"
#include "TantrumnCharacterBase.h"
#include "TantrumnGameModeBase.h"
#include "TantrumnGameInstance.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnPlayerState.h"
#include "TantrumnInputConfigRegistry.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput//Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"

// Console variable to toggle character throwing velocity
static TAutoConsoleVariable<bool> CVarDisplayLaunchInputDelta(
	TEXT("Tantrumn.Character.Debug.DisplayLaunchInputdelta"),
	false,
	TEXT("Display Launch Input Delta"),
	ECVF_Default);

void ATantrumnPlayerController::BeginPlay()
{
	Super::BeginPlay();
	TantrumnGameState = GetWorld()->GetGameState<ATantrumnGameStateBase>();

	// Setup Enhanced Input
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
				&ATantrumnPlayerController::RequestPullObject);

			EIPlayerComponent->BindAction(InputActions->InputPullObject,
				ETriggerEvent::Completed,
				this,
				&ATantrumnPlayerController::RequestStopPullObject);
		}
	}
}

void ATantrumnPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("OnPossess: %s"), *GetName());
}

void ATantrumnPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	UE_LOG(LogTemp, Warning, TEXT("OnUnPossess: %s"), *GetName());
}

void ATantrumnPlayerController::ClientDisplayCountDown_Implementation(float GameCountDownDuration)
{
	if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
	{
		TantrumnGameInstance->DisplayCountDown(GameCountDownDuration, this);
	}
}

void ATantrumnPlayerController::ClientRestartGame_Implementation()
{
	if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
	{
		TantrumnGameInstance->RestartGame(this);
	}
}

void ATantrumnPlayerController::ClientReachedEnd_Implementation()
{
	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->ServerPlayCelebrateMontage();
		TantrumnCharacterBase->GetCharacterMovement()->DisableMovement();
	}

	if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
	{
		// TODO Call the level complete event for the widget...
	}

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ATantrumnPlayerController::ServerRestartLevel_Implementation()
{
	ATantrumnGameModeBase* TantrumnGameMode = GetWorld()->GetAuthGameMode<ATantrumnGameModeBase>();
	if (ensureMsgf(TantrumnGameMode, TEXT("ATantrumnPlayerController::ServerRestartLevel_Implementation Invalid Game Mode")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Restart Game Level Requested."))
		TantrumnGameMode->RestartGame();
	}
}

void ATantrumnPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		if (HUDClass)
		{
			HUDWidget = CreateWidget(this, HUDClass);
			if (HUDWidget)
			{
				HUDWidget->AddToPlayerScreen();
			}
		}
	}
}

void ATantrumnPlayerController::RequestMove(const FInputActionValue& ActionValue)
{
	if (!CanProcessRequest())
	{
		return;
	}
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
	if (!CanProcessRequest())
	{
		return;
	}
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
	if (!CanProcessRequest())
	{
		return;
	}
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
	if (!CanProcessRequest())
	{
		return;
	}
	
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

bool ATantrumnPlayerController::CanProcessRequest() const
{
	if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
	{
		// if (TantrumnGameInstance->IsPlayableGame == false)
		// {
		// 	// Not playing a game, used for testing and debug gameplay
		// 	return true;
		// }
		if (TantrumnGameState && TantrumnGameState->IsPlaying())
		{
			if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
			{
				return (TantrumnPlayerState->GetCurrentState() == EPlayerGameState::Playing);
			}
		}
	}

	return false;
}

void ATantrumnPlayerController::RequestThrowObject(const FInputActionValue& ActionValue)
{
	// Before we throw we need to check if character is allowed to throw
	const float AxisValue = ActionValue.Get<float>();  // TODO check this works as intended
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		if (TantrumnCharacter->CanThrowObject())
		{
			// the delta and axis values influence throw behaviour
			float CurrentDelta = AxisValue - LastAxis;

			// Debug toggle to character display throwing velocity
			if (CVarDisplayLaunchInputDelta->GetBool())
			{
				if (fabs(CurrentDelta) > 0.0f)
				{
					UE_LOG(LogTemp, Warning, TEXT("Axis: %f CurrentDelta: %f"), AxisValue, LastAxis);
				}
			}
			LastAxis = AxisValue;

			// Flick action triggers the player to throw
			const bool IsFlick = fabs(CurrentDelta) > FlickThreshold;
			if (IsFlick)
			{
				// Depending on mousewheel forward or back; throw or apply effect on ourselves
				if (AxisValue > 0)
				{
					TantrumnCharacter->RequestThrowObject();
				}
				else
				{
					TantrumnCharacter->RequestUseObject();
				}
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
	if (!CanProcessRequest())
	{
		return;
	}
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