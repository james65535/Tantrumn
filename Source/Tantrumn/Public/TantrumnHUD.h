// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TantrumnHUD.generated.h"

class UTantrumnGameWidget;
class UUserWidget;

/**
 * 
 */
UCLASS()
class TANTRUMN_API ATantrumnHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:

	/** Graphics Menu */
	/**
	 * Checks for available Screen Resolutions
	 * @param ScreenResOpts A reference to the data container with which to store the Screen Resolutions
	 */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn UI")
	void CreateScreenResOpts(UPARAM(ref) TMap<FString, FIntPoint>& ScreenResOpts);
	/**
	 * Sets the given Screen Resolution
	 * @param InScreenRes The IntPoint with which to set the Screen Resolution
	 * @param bOverrideCommandLine Should the Game User Settings override conflicting command line settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn UI")
	void SetScreenRes(FIntPoint InScreenRes, bool bOverrideCommandLine);
	/**
	 * Stores the Game User Settings to Disk
	 * @param bOverrideCommandLine Should the Game User Settings override conflicting command line settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn UI")
	void ConfirmGameUserSettings(bool bOverrideCommandLine);
	
	

	
	// TODO Determine if reliable can be removed
	UFUNCTION(Client, Reliable, Category = "Tantrumn")
	void DisplayGameTimer(float GameTimeDownDuration);

	UFUNCTION(Client, Reliable, Category = "Tantrumn")
	void DisplayResults();

	UFUNCTION(Client, Reliable, Category = "Tantrumn")
	void RemoveResults();

	UFUNCTION(BlueprintCallable, Category = "Tantrumn UI")
	void SetLevelWidgetClass(TSubclassOf<UTantrumnGameWidget> InLevelWidgetClass);
	
	UFUNCTION(Client, Reliable, Category = "Tantrumn")
	void ToggleStartMenu(bool bShouldDisplay);
	/** Display the Game Menu HUD within a Level */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn UI")
	void ToggleLevelMenuDisplay(const bool bIsDisplayed);

private:
	
	/** Level Specific UI */
	UPROPERTY(VisibleInstanceOnly, Category = "Tantrumn UI")
	UTantrumnGameWidget* GameLevelWidget;
	/** Class - Level Specific UI Class */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	TSubclassOf<UTantrumnGameWidget> GameLevelWidgetClass;
	/** Named Slot Widgets to Add Specific UI Content */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	FName LevelUINamedSlotName = "NS_LevelUI";

	/** Level Specific UI */
	UPROPERTY(VisibleInstanceOnly, Category = "Tantrumn UI")
	UTantrumnGameWidget* LevelEndWidget;
	/** Class - Level Specific UI Class */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	TSubclassOf<UTantrumnGameWidget> LevelEndWidgetClass;
	/** Named Slot Widgets to Add Specific UI Content */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	FName LevelEndWidgetNamedSlotName = "NS_LevelEndUI";
	
	/** Game Menu to be Displayed in Level */
	UPROPERTY(VisibleInstanceOnly, Category = "Tantrumn UI")
	UTantrumnGameWidget* LevelMenuWidget;
	/** Class - Game Menu to be Displayed in Level */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	TSubclassOf<UTantrumnGameWidget> LevelMenuWidgetClass;
	/** Named Slot Widget to Add Menu UI Content */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	FName MenuUINamedSlotName = "NS_MenuUI";
	
	/** Game Start Main Menu */
	UPROPERTY(VisibleInstanceOnly, Category = "Tantrumn UI")
	UTantrumnGameWidget* GameStartScreenWidget;
	/** Class- General Game Widget */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	TSubclassOf<UTantrumnGameWidget> GameStartScreenWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	FName StartMenuUINamedSlotName = "NS_StartMenuUI";

	/** Game Base UI Widget - Parent for all other UIs */
	UPROPERTY(VisibleInstanceOnly, Category = "Tantrumn UI")
	UTantrumnGameWidget* BaseUIWidget;
	/** Class - Game Base UI Widget - Parent for all other UIs */
	UPROPERTY(EditDefaultsOnly, Category = "Tantrumn UI")
	TSubclassOf<UTantrumnGameWidget> BaseUIWidgetClass;

	/** Add Game Mode UI to Game Base UI */
	UFUNCTION(BlueprintCallable, Category = "Tantrumn UI")
	UTantrumnGameWidget* AddSlotUI_Implementation(TSubclassOf<UTantrumnGameWidget> InWidgetClass, FName InSlotName);
	/** Add Widget to Player Viewport Wrapper */
	UTantrumnGameWidget* AddWidget(const TSubclassOf<UTantrumnGameWidget> InWidgetClass) const;
};
