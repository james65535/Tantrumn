// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"

#include "TantrumnAIController.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnPlayerController.h"
#include "TantrumnPlayerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ATantrumnGameModeBase::ATantrumnGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATantrumnGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	/**
	 * If Game is in Start Menu then Player's HUD will take care of the rest
	 * Otherwise we let the HUD of each player controller know the class of Widget to Display
	 * for this Game Mode
	 */
	if (ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->SetGameType(DesiredGameType);

		// TODO replace with desiredgametype
		if(bToggleInitialMainMenu)
		{
			TantrumnGameState->SetGameState(ETantrumnGameState::None);
		}
		else
		{
			TantrumnGameState->SetGameState(ETantrumnGameState::Waiting);
		}
	}
}

void ATantrumnGameModeBase::AttemptStartGame()
{
	if (ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->SetGameType(DesiredGameType);
		TantrumnGameState->SetGameState(ETantrumnGameState::Waiting);
	}

	if (CheckAllPlayersStatus(EPlayerGameState::Ready))
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode: %s - All Players Ready"), *GetName());
		// This needs to be replicated, call a function on game instance and replicate
		if (GameCountDownDuration > SMALL_NUMBER)
		{
			GetWorld()->GetTimerManager().SetTimer(DelayStartTimerHandle,
				this, &ATantrumnGameModeBase::DisplayCountDown,
				DelayStartDuration,
				false);
		}
		else
		{
			StartGame();
		}
	} else
	{
		// TODO Get rid of this timer and just do a check for final ready
		UE_LOG(LogTemp, Warning, TEXT("GameMode: %s - Not all players ready"), *GetName());
		GetWorld()->GetTimerManager().SetTimer(
			MatchTryStartTimerHandle,
			this, &ATantrumnGameModeBase::AttemptStartGame,
			MatchTryStartWaitDuration,
			false);
	}
}

void ATantrumnGameModeBase::DisplayCountDown()
{
	/**
	 * Perform Spectator Check prior to calling Game Countdown to minimise calls between
	 * When Timer is started on server and when it is started on client
	 */
	TArray<ATantrumnPlayerController*> NonSpectatingPlayers;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if(ATantrumnPlayerController* PlayerController = Cast<ATantrumnPlayerController>(Iterator->Get()))
		{
			if(!MustSpectate(PlayerController))
			{
				NonSpectatingPlayers.Emplace(PlayerController);
			}
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		CountdownTimerHandle,
		this, &ATantrumnGameModeBase::StartGame,
		GameCountDownDuration,
		false);

	for (ATantrumnPlayerController* NonSpectatingPlayer : NonSpectatingPlayers)
	{
		NonSpectatingPlayer->C_StartGameCountDown(GameCountDownDuration);
	}
}

void ATantrumnGameModeBase::StartGame()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("This System does not have authority to start game"));
		return;
	}
	ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>();
	check(TantrumnGameState);
	UE_LOG(LogTemp, Warning, TEXT("Gamemode entering playing state"));
	TantrumnGameState->NM_MatchStart();
	
	/** Restore control to Players and reset Player State for Players and AI */
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ATantrumnPlayerController* PlayerController = Cast<ATantrumnPlayerController>(Iterator->Get());
		check(PlayerController);
		if (!MustSpectate(PlayerController))
		{
			ATantrumnPlayerState* PlayerState = PlayerController->GetPlayerState<ATantrumnPlayerState>();
			check(PlayerState);
			PlayerState->SetCurrentState(EPlayerGameState::Playing);
			PlayerState->SetIsWinner(false);
		}
	}
	
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		if(const ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(Iterator->Get()))
		{
			ATantrumnPlayerState* AIPlayerState = TantrumnAIController->GetPlayerState<ATantrumnPlayerState>();
			check(AIPlayerState)
			AIPlayerState->SetCurrentState(EPlayerGameState::Playing);
			AIPlayerState->SetIsWinner(false);
		}
	}
}

bool ATantrumnGameModeBase::CheckAllPlayersStatus(const EPlayerGameState StateToCheck) const
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if(const ATantrumnPlayerState* PlayerState =
			Cast<APlayerController>(Iterator->Get())->GetPlayerState<ATantrumnPlayerState>())
		{
			if (PlayerState->GetCurrentState() != StateToCheck)
			{
				UE_LOG(LogTemp, Warning, TEXT("PlayerID: %i did not match requested state"), PlayerState->GetPlayerId())
				return false;
			}
		}
	}
	return true;
}

void ATantrumnGameModeBase::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (const APlayerController* PlayerController = Cast<APlayerController>(NewPlayer))
	{
		check(PlayerController->GetCharacter() && PlayerController->GetCharacter()->GetCharacterMovement());
		PlayerController->GetCharacter()->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		if(ATantrumnPlayerState* PlayerState = PlayerController->GetPlayerState<ATantrumnPlayerState>())
		{
			PlayerState->SetCurrentState(EPlayerGameState::Waiting);
		}
	}

	AttemptStartGame();
}

void ATantrumnGameModeBase::RestartGame()
{
	// Destroy existing AI and let level reset create a fresh one on game restart
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		if (ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(Iterator->Get()))
		{
			if (TantrumnAIController->GetPawn())
			{
				TantrumnAIController->Destroy(true);
			}
		}
	}
	
	ResetLevel();
	// TODO should we check if all players ready here?
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (ATantrumnPlayerController* TantrumnPlayerController = Cast<ATantrumnPlayerController>(PlayerController))
			{
				TantrumnPlayerController->C_ResetPlayer();
			}

			RestartPlayer(PlayerController);
		}
	}
}