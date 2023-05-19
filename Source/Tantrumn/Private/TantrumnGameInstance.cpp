// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnGameInstance.h"
#include "TantrumnGameWidget.h"
#include "TantrumnGameStateBase.h"
#include "Tantrumn/TantrumnPlayerController.h"


// void UTantrumnGameInstance::DisplayCountDown(float GameCountDownDuration, ATantrumnPlayerController* TantrumnPlayerController)
// {
// 	if (GetWorld())
// 	{
// 		UTantrumnGameWidget* PlayerGameWidget = nullptr;
// 		UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
// 		if(!GameWidget)
// 		{
// 			PlayerGameWidget = CreateWidget<UTantrumnGameWidget>(TantrumnPlayerController, GameWidgetClass);
// 			if (PlayerGameWidget)
// 			{
// 				// Only do this once
// 				// We never remove this as it has the race time
// 				UE_LOG(LogTemp, Warning, TEXT("Attempting to create a new game widget and add to playerscreen"));
// 				PlayerGameWidget->AddToPlayerScreen();
// 				GameWidgets.Add(TantrumnPlayerController, PlayerGameWidget);
// 			}
// 		}
// 		else
// 		{
// 			PlayerGameWidget = *GameWidget;
// 		}
//
// 		if (PlayerGameWidget)
// 		{
// 			PlayerGameWidget->StartCountDown(GameCountDownDuration, TantrumnPlayerController);
// 			TantrumnGameStateBase = GetWorld()->GetGameState<ATantrumnGameStateBase>();
// 			if (!TantrumnGameStateBase)
// 			{
// 				GetWorld()->GameStateSetEvent.AddUObject(this, &UTantrumnGameInstance::OnGameStateSet);
// 			}
// 		}
// 	}
// }
//
// void UTantrumnGameInstance::OnGameStateSet(AGameStateBase* GameStateBase)
// {
// 	TantrumnGameStateBase = Cast<ATantrumnGameStateBase>(GameStateBase);
// }
//
// void UTantrumnGameInstance::DisplayLevelComplete(ATantrumnPlayerController* TantrumnPlayerController)
// {
// 	UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
// 	if (GameWidget)
// 	{
// 		(*GameWidget)->DisplayResults();
// 	}
// }
//
// void UTantrumnGameInstance::RestartGame(ATantrumnPlayerController* TantrumnPlayerController)
// {
// 	UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
// 	if (GameWidget)
// 	{
// 		(*GameWidget)->RemoveResults();
// 		// Restore Game Input
// 		FInputModeGameOnly InputMode;
// 		TantrumnPlayerController->SetInputMode(InputMode);
// 		TantrumnPlayerController->SetShowMouseCursor(false);
// 	}
// }
//
// void UTantrumnGameInstance::OnRetrySelected(ATantrumnPlayerController* TantrumnPlayerController)
// {
// 	UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
// 	if (GameWidget)
// 	{
// 		RestartGame(TantrumnPlayerController);
// 		TantrumnPlayerController->ServerRestartLevel();
// 	}
// }





