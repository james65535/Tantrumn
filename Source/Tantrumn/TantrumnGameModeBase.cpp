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
		
		if(bToggleInitialMainMenu)
		{ TantrumnGameState->SetGameState(ETantrumnGameState::None); }
		else
		{ TantrumnGameState->SetGameState(ETantrumnGameState::Waiting); }
	}
}

void ATantrumnGameModeBase::PlayerNotifyIsReady(ATantrumnPlayerState* InPlayerState)
{
	if(CheckAllPlayersStatus(EPlayerGameState::Waiting))
	{ AttemptStartGame(); }
}

void ATantrumnGameModeBase::AttemptStartGame()
{
	/** Run Countdown timer to start of match */
	if (GameCountDownDuration > SMALL_NUMBER)
	{
		GetWorld()->GetTimerManager().SetTimer(DelayStartTimerHandle,
			this, &ThisClass::DisplayCountDown,
			DelayStartDuration,
			false);
	}
	else
	{ StartGame(); }
}

void ATantrumnGameModeBase::DisplayCountDown()
{
	/**
	 * Perform Spectator Check prior to calling Game Countdown to minimise calls between
	 * when Timer is started on server and when it is started on client
	 */
	TArray<ATantrumnPlayerController*> NonSpectatingPlayers;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if(ATantrumnPlayerController* PlayerController = Cast<ATantrumnPlayerController>(Iterator->Get()))
		{
			if(!MustSpectate(PlayerController))
			{ NonSpectatingPlayers.Emplace(PlayerController); }
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		CountdownTimerHandle,
		this, &ThisClass::StartGame,
		GameCountDownDuration,
		false);

	for (ATantrumnPlayerController* NonSpectatingPlayer : NonSpectatingPlayers)
	{ NonSpectatingPlayer->C_StartGameCountDown(GameCountDownDuration); }
}



void ATantrumnGameModeBase::StartGame()
{
	if (!HasAuthority())
	{ return; }
	
	/** Restore control to Players and reset Player State for Players and AI */
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		if(ATantrumnPlayerController* PlayerController = Cast<ATantrumnPlayerController>(Iterator->Get()))
		{
			if (!MustSpectate(PlayerController))
			{
				ATantrumnPlayerState* PlayerState = PlayerController->GetPlayerState<ATantrumnPlayerState>();
				check(PlayerState);
				PlayerState->SetCurrentState(EPlayerGameState::Playing);
				PlayerState->SetIsWinner(false);
			}
		}
		
		if(const ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(Iterator->Get()))
		{
			ATantrumnPlayerState* AIPlayerState = TantrumnAIController->GetPlayerState<ATantrumnPlayerState>();
			check(AIPlayerState)
			AIPlayerState->SetCurrentState(EPlayerGameState::Playing);
			AIPlayerState->SetIsWinner(false);
		}
	}

	/** Inform Gamestate that match has started */
	ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>();
	check(TantrumnGameState);
	TantrumnGameState->NM_MatchStart();
}

bool ATantrumnGameModeBase::CheckAllPlayersStatus(const EPlayerGameState StateToCheck) const
{
	uint32 Count = 0;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if(const ATantrumnPlayerState* PlayerState =
			Cast<APlayerController>(Iterator->Get())->GetPlayerState<ATantrumnPlayerState>())
		{
			if (PlayerState->GetCurrentState() != StateToCheck)
			{ return false; }
			Count++;
		}
	}
	if (Count <= 0) { return false; }
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
		{ PlayerState->SetCurrentState(EPlayerGameState::Unready); }
	}
}

void ATantrumnGameModeBase::RestartGame()
{
	/** Destroy existing AI and let level reset create a fresh one on game restart */
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		if (ATantrumnAIController* TantrumnAIController = Cast<ATantrumnAIController>(Iterator->Get()))
		{
			if (TantrumnAIController->GetPawn())
			{ TantrumnAIController->Destroy(true); }
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
			{ TantrumnPlayerController->C_ResetPlayer(); }

			RestartPlayer(PlayerController);
		}
	}
}
