// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerController.h"

#include "TantrumnCharacterBase.h"
#include "TantrumnGameElementsRegistry.h"
#include "TantrumnGameModeBase.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnHUD.h"
#include "TantrumnPlayerState.h"
#include "TantrumnInputConfigRegistry.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput//Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/InputActionValue.h"

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
	check(TantrumnGameState);

	/** Player HUD related Tasks */
	if (!IsRunningDedicatedServer())
	{
		/** Specify HUD Representation at start of play */
		PlayerHUD = Cast<ATantrumnHUD>(GetHUD());
		/** If Local game as listener or single player then Grab Gametype, otherwise use delegate to update on replication */
		UpdateHUDWithGameUIElements(TantrumnGameState->GetGameType());
		TantrumnGameState->OnGameTypeUpdateDelegate.AddUObject(this, &ATantrumnPlayerController::UpdateHUDWithGameUIElements);
	}

	/* Set Enhanced Input Mapping Context to Game Context */
	SetInputContext(GameInputMapping);
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
			&ATantrumnPlayerController::RequestHoldObject);

		EIPlayerComponent->BindAction(InputActions->InputPullObject,
			ETriggerEvent::Completed,
			this,
			&ATantrumnPlayerController::RequestStopHoldObject);

		// Character Menu Display
		EIPlayerComponent->BindAction(InputActions->InputOpenMenu,
			ETriggerEvent::Completed,
			this,
			&ATantrumnPlayerController::RequestDisplayLevelMenu);

		// Character Menu Display Remove
		EIPlayerComponent->BindAction(InputActions->InputCloseMenu,
			ETriggerEvent::Completed,
			this,
			&ATantrumnPlayerController::RequestHideLevelMenu);
	}

	/** Game Starts with a UI */
	SetControllerGameInputMode(ETantrumnInputMode::UIOnly);
	
	/** Listen for match start announcements */
	TantrumnGameState->OnStartMatchDelegate.AddUObject(this, &ATantrumnPlayerController::MatchPlayStart);
}

void ATantrumnPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!IsRunningDedicatedServer() && TantrumnPlayerState)
	{
		if(TantrumnPlayerState->GetCurrentState() == EPlayerGameState::Playing)
		{
			PlayerHUD->DisplayGameTime(TantrumnGameState->GetMatchDeltaTime());
		} 
	}
}

bool ATantrumnPlayerController::CanProcessRequest() const
{
	if (TantrumnGameState && TantrumnGameState->IsGameInPlay())
	{
		return (TantrumnPlayerState->GetCurrentState() == EPlayerGameState::Playing);
	}
	return false;
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

void ATantrumnPlayerController::OnRetrySelected()
{
	S_RestartLevel();
}

void ATantrumnPlayerController::OnReadySelected()
{
	if (!TantrumnPlayerState)
	{
		TantrumnPlayerState = CastChecked<ATantrumnPlayerState>(PlayerState);
	}
	S_OnReadySelected();
}

void ATantrumnPlayerController::S_OnReadySelected_Implementation()
{
	if (GetWorld()->GetAuthGameMode<ATantrumnGameModeBase>())
	{
		if (!TantrumnPlayerState)
		{
			TantrumnPlayerState = Cast<ATantrumnPlayerState>(PlayerState);
		}
		TantrumnPlayerState->SetCurrentState(EPlayerGameState::Ready);
		UE_LOG(LogTemp, Warning, TEXT("Player set self to ready. State: %hhd"), TantrumnPlayerState->GetCurrentState())
	}
}

void ATantrumnPlayerController::C_StartGameCountDown_Implementation(const float InCountDownDuration)
{
	PlayerHUD->DisplayMatchStartCountDownTime(InCountDownDuration);
}

void ATantrumnPlayerController::UpdateHUDWithGameUIElements(ETantrumnGameType InGameType)
{
	checkfSlow(GameElementsRegistry, "PlayerController: Verify Controller Blueprint has a UI Elements registry set");
	if(GameElementsRegistry->GameTypeUIMapping.Contains(InGameType))
	{
		PlayerHUD->SetGameUIAssets(*GameElementsRegistry->GameTypeUIMapping.Find(InGameType));
	}
}

void ATantrumnPlayerController::RequestDisplayLevelMenu()
{
	if (CanProcessRequest())
	{
		SetInputContext(MenuInputMapping);
		PlayerHUD->ToggleLevelMenuDisplay(true);
		SetControllerGameInputMode(ETantrumnInputMode::GameAndUI);
	}
}

void ATantrumnPlayerController::RequestHideLevelMenu()
{
	if (CanProcessRequest())
	{
		SetInputContext(GameInputMapping);
		PlayerHUD->ToggleLevelMenuDisplay(false);
		SetControllerGameInputMode(ETantrumnInputMode::GameOnly);
	}
}

void ATantrumnPlayerController::MatchPlayStart(const float InMatchStartTime)
{
	check(TantrumnPlayerState)
	TantrumnPlayerState->SetCurrentState(EPlayerGameState::Playing);
	TantrumnPlayerState->SetIsWinner(false);
	SetControllerGameInputMode(ETantrumnInputMode::GameOnly);
	if (!IsRunningDedicatedServer())
	{
		PlayerHUD->DisplayGameTime(InMatchStartTime);
	}
}

void ATantrumnPlayerController::FinishedMatch()
{
	if (!IsRunningDedicatedServer())
	{
		RequestHideLevelMenu();
		SetInputContext(MenuInputMapping);
		SetControllerGameInputMode(ETantrumnInputMode::UIOnly);
	}
}

void ATantrumnPlayerController::RequestDisplayFinalResults() const
{
	if (!IsRunningDedicatedServer())
	{
		PlayerHUD->DisplayResults(TantrumnGameState->GetResults());
	}
}

void ATantrumnPlayerController::C_ResetPlayer_Implementation()
{
	check(PlayerHUD);
	PlayerHUD->RemoveResults();
	UpdateHUDWithGameUIElements(TantrumnGameState->GetGameType());
}

void ATantrumnPlayerController::S_RestartLevel_Implementation()
{
	ATantrumnGameModeBase* TantrumnGameMode = GetWorld()->GetAuthGameMode<ATantrumnGameModeBase>();
	if (ensureMsgf(TantrumnGameMode, TEXT("ATantrumnPlayerController::ServerRestartLevel_Implementation Invalid Game Mode")))
	{
		TantrumnGameMode->RestartGame();
	}
}

void ATantrumnPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Tear down Enhanced Input subsystem for mappings
	if (const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (!GameInputMapping.IsNull())
			{
				InputSystem->ClearAllMappings();
			}
		}
	}
}

void ATantrumnPlayerController::SetInputContext(TSoftObjectPtr<UInputMappingContext> InMappingContext)
{
	if(const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if(const UInputMappingContext* InMappingContextLoaded = InMappingContext.LoadSynchronous())
		{
			if (!InputSystem->HasMappingContext(InMappingContextLoaded))
			{
				InputSystem->ClearAllMappings();
				InputSystem->AddMappingContext(InMappingContextLoaded, 0, FModifyContextOptions());
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

void ATantrumnPlayerController::RequestThrowObject(const FInputActionValue& ActionValue)
{
	// Before we throw we need to check if character is allowed to throw
	const float AxisValue = ActionValue.Get<float>();  // TODO check this works as intended
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		if (TantrumnCharacter->CanThrowObject())
		{
			// the delta and axis values influence throw behaviour
			const float CurrentDelta = AxisValue - LastAxis;

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
			//const bool IsFlick = fabs(CurrentDelta) > FlickThreshold;
			if (fabs(CurrentDelta) > FlickThreshold)
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

void ATantrumnPlayerController::RequestHoldObject()
{
	if (!CanProcessRequest())
	{
		return;
	}
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		if (TantrumnCharacter->CanAim())
		{
			TantrumnCharacter->RequestAim();
		}
		else
		{
			TantrumnCharacter->RequestPullObjectStart();	
		}
	}
}

void ATantrumnPlayerController::RequestStopHoldObject()
{
	if (ATantrumnCharacterBase* TantrumnCharacter = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacter->RequestPullObjectStop();
	}
}

void ATantrumnPlayerController::SetControllerGameInputMode_Implementation(const ETantrumnInputMode InRequestedInputMode)
{
	switch (InRequestedInputMode)
	{
	case (ETantrumnInputMode::GameOnly):
		{
			const FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
			SetShowMouseCursor(false);
			break;
		}
	case (ETantrumnInputMode::GameAndUI):
		{
			const FInputModeGameAndUI InputMode;
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
			break;
		}
	case (ETantrumnInputMode::UIOnly):
		{
			const FInputModeUIOnly InputMode;
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
			break;
		}
	}
}
