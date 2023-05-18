// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"

#include "TantrumnAIController.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnPlayerController.h"
#include "TantrumnPlayerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ATantrumnGameModeBase::ATantrumnGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATantrumnGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (ATantrumnGameStateBase* TantrumnGameState= GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->SetGameState(EGameState::Waiting);
	}
}

void ATantrumnGameModeBase::AttemptStartGame()
{
	if (ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->SetGameState(EGameState::Waiting);
	}
	
	if (GetNumPlayers() == NumExpectedPlayers)
	{
		// This needs to be replicated, call a function on game instance and replicate
		DisplayCountDown();
		if (GameCountDownDuration > SMALL_NUMBER)
		{
			GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle,
				this, &ATantrumnGameModeBase::StartGame,
				GameCountDownDuration,
				false);
		}
		else
		{
			StartGame();
		}
	}
}

void ATantrumnGameModeBase::DisplayCountDown()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (ATantrumnPlayerController* TantrumnPlayerController = Cast<ATantrumnPlayerController>(PlayerController))
			{
				TantrumnPlayerController->ClientDisplayCountDown(GameCountDownDuration);
			}
		}
	}
}

void ATantrumnGameModeBase::StartGame()
{
	if (ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->SetGameState(EGameState::Playing);
		TantrumnGameState->ClearResults();
	}
	
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);

			if (ATantrumnPlayerState* PlayerState = PlayerController->GetPlayerState<ATantrumnPlayerState>())
			{
				PlayerState->SetCurrentState(EPlayerGameState::Playing);
				PlayerState->SetIsWinner(false);
			}
		}
	}
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		if (ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(Iterator->Get()))
		{
			if (ATantrumnPlayerState* PlayerState = TantrumnAIController->GetPlayerState<ATantrumnPlayerState>())
			{
				PlayerState->SetCurrentState(EPlayerGameState::Playing);
				PlayerState->SetIsWinner(false);
			}
		}
	}
}

void ATantrumnGameModeBase::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (APlayerController* PlayerController = Cast<APlayerController>(NewPlayer))
	{
		if (PlayerController->GetCharacter() && PlayerController->GetCharacter()->GetCharacterMovement())
		{
			PlayerController->GetCharacter()->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			ATantrumnPlayerState* PlayerState = PlayerController->GetPlayerState<ATantrumnPlayerState>();
			if (PlayerState)
			{
				PlayerState->SetCurrentState(EPlayerGameState::Waiting);
			}
		}
	}

	AttemptStartGame();
}

void ATantrumnGameModeBase::RestartGame()
{

	// Destroy existing AI and let level reset create a fresh one on game restart
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(Iterator->Get());
		if (TantrumnAIController && TantrumnAIController->GetPawn())
		{
			TantrumnAIController->Destroy(true);
		}
	}
	
	ResetLevel();

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (ATantrumnPlayerController* TantrumnPlayerController = Cast<ATantrumnPlayerController>(PlayerController))
			{
				TantrumnPlayerController->ClientRestartGame();
			}
			RestartPlayer(PlayerController);
		}
	}
}