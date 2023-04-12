// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"
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

	// Start game play with a timer before player can move
	CurrentGameState = ETantrumnGameState::Waiting;
}

void ATantrumnGameModeBase::ReceivePlayer(APlayerController* PlayerController)
{
	AttemptStartGame();
}

void ATantrumnGameModeBase::AttemptStartGame()
{
	if (GetNumPlayers() == NumExpectedPlayers)
	{
		DisplayCountdown();
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

void ATantrumnGameModeBase::StartGame()
{
	CurrentGameState = ETantrumnGameState::Playing;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

// Countdown timer for game start
void ATantrumnGameModeBase::DisplayCountdown()
{
	// Multiplayer consideration for adding widgets to player screens
	if (!GameWidgetClass)
	{
		return;
	}
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (UTantrumnGameWidget* GameWidget = CreateWidget<UTantrumnGameWidget>(PlayerController, GameWidgetClass))
			{
				GameWidget->AddToPlayerScreen();
				GameWidget->StartCountDown(GameCountDownDuration, this);
				GameWidgets.Add(PlayerController, GameWidget);
			}
		}
	}
}

ETantrumnGameState ATantrumnGameModeBase::GetCurrentGameState() const
{
	return CurrentGameState;
}

void ATantrumnGameModeBase::PLayerReachedEnd(APlayerController* PlayerController)
{
	//one gamemode base is shared between players in local mp
	CurrentGameState = ETantrumnGameState::Gameover;
	UTantrumnGameWidget** GameWidget = GameWidgets.Find(PlayerController);
	if (GameWidget)
	{
		(*GameWidget)->LevelComplete();
		FInputModeUIOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
		if (PlayerController->GetCharacter() && PlayerController->GetCharacter()->GetCharacterMovement())
		{
			PlayerController->GetCharacter()->GetCharacterMovement()->DisableMovement();
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
		}
	}
}







