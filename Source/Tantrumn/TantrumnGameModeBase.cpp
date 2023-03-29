// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"

#include "LocalizationDescriptor.h"
#include "TantrumnGameWidget.h"
#include "TantrumnPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATantrumnGameModeBase::ATantrumnGameModeBase()
{
	
}

void ATantrumnGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Start game play with a timer before player can move
	CurrentGameState = ETantrumnGameState::WAITING;
	DisplayCountdown();
	GetWorld()->GetTimerManager().SetTimer(
		CountdownTimerHandle,
		this,
		&ATantrumnGameModeBase::StartGame,
		GameCountDownDuration,
		false);
}

void ATantrumnGameModeBase::StartGame()
{
	CurrentGameState = ETantrumnGameState::PLAYING;
	// Input may be disabled if replaying the game
	FInputModeGameOnly const InputMode;
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(false);
}

// Countdown timer for game start
void ATantrumnGameModeBase::DisplayCountdown()
{
	if (!GameWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Gamemode did not have a valid widget class for UI"));
		return;
	}

	// Add game timer UI to player
	PC = Cast<ATantrumnPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if(PC)
	{
		GameWidget = CreateWidget<UTantrumnGameWidget>(PC, GameWidgetClass);
		GameWidget->AddToViewport();
		GameWidget->StartCountDown(GameCountDownDuration, this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Gamemode could not cast a player controller to add game UI"));
	}
}

ETantrumnGameState ATantrumnGameModeBase::GetCurrentGameState() const
{
	return CurrentGameState;
}

void ATantrumnGameModeBase::PLayerReachedEnd()
{
	CurrentGameState = ETantrumnGameState::GAMEOVER;
	GameWidget->LevelComplete();
	FInputModeUIOnly const InputMode;
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
}







