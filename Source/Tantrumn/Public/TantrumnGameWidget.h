// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnGameWidget.generated.h"

/**
 * 
 */

enum class ETantrumnGameType : uint8;

USTRUCT(BlueprintType, Category = "Tantrumn UI")
struct FGameUIClassInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Tantrumn UI")
	TSubclassOf<UTantrumnGameWidget> WidgetClass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Tantrumn UI")
	FName WidgetSlot;
};


USTRUCT(BlueprintType, Category = "Tantrumn UI")
struct FGameUI
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Tantrumn UI")
	FGameUIClassInfo GamePlayWidget;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Tantrumn UI")
	FGameUIClassInfo GameMenuWidget;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Tantrumn UI")
	FGameUIClassInfo GameEndScreenWidget;
};

UCLASS()
class TANTRUMN_API UTantrumnGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void InitiateMatchStartTimer(float CountDownTime);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void DisplayGameTimer(float MatchStartTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void LevelComplete();

	/**
	 * Display Results of Finished Game
	 * @param InResults A Collection of listings and if the corresponding listing is a winner
	 */ 
	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void DisplayResults(const TArray<FGameResult>& InResults);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void RemoveResults();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void DisplayStartMenu();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tantrumn UI")
	void DisplayGameModeUI();
};
