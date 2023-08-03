// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnHUD.h"

#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TantrumnGameWidget.h"
#include "UIElementsAsset.h"
#include "Tantrumn/TantrumnPlayerController.h"

void ATantrumnHUD::BeginPlay()
{
	Super::BeginPlay();
	
	/** Get the GameUserSettings data container with which our work will depend upon */
	UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
	/** Deserialise settings from Disk - Ex: DefaultGameUserSettings.ini */
	GameUserSettings->LoadSettings();
	/** Overwrite any garbage with default settings if need be */
	GameUserSettings->ValidateSettings();

	TantrumnPlayerController = Cast<ATantrumnPlayerController>(GetOwningPlayerController());
	check(TantrumnPlayerController);

	/** Reduce displayed timer to max 1 digit fractional, ex from 2.54 to 2.5 */
	FloatDisplayFormat.SetMaximumFractionalDigits(1);
}

UTantrumnGameWidget* ATantrumnHUD::AddSlotUI_Implementation(TSubclassOf<UTantrumnGameWidget> InWidgetClass, FName InSlotName)
{
	check(InWidgetClass);
	if (BaseUIWidget == nullptr)
	{
		/** Base UI is the parent for all Widgets */
		BaseUIWidget = AddWidget(BaseUIWidgetClass);
		check(BaseUIWidget);
	}

	if (UTantrumnGameWidget* OutWidgetRef = CreateWidget<UTantrumnGameWidget>(GetOwningPlayerController(), InWidgetClass))
	{
		BaseUIWidget->SetContentForSlot(InSlotName, OutWidgetRef);
		if(BaseUIWidget->GetContentForSlot(InSlotName) != nullptr)
		{
			return OutWidgetRef;
		}
	}
	return nullptr;
}

UTantrumnGameWidget* ATantrumnHUD::AddWidget(const TSubclassOf<UTantrumnGameWidget> InWidgetClass) const
{
	check(InWidgetClass);
	if(UTantrumnGameWidget* ReturnWidget = CreateWidget<UTantrumnGameWidget>(GetOwningPlayerController(), InWidgetClass))
	{
		ReturnWidget->AddToPlayerScreen();
		return ReturnWidget;
	}
	return nullptr;
}

void ATantrumnHUD::CreateScreenResOpts(UPARAM(ref) TMap<FString, FIntPoint>& ScreenResOpts)
{
	/** Use Kismet Library to retrieve a list of Screen Resolutions */
	TArray<FIntPoint> SupportedScreenResolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedScreenResolutions);

	/** Clear out data container since we are unsure of what is in there */
	ScreenResOpts.Empty();

	/** Iterate over possible Screen Resolutions and populate the data container */
	for (FIntPoint SupportedScreenResolution : SupportedScreenResolutions)
	{
		/** Derive a KeyName which is human friendly, ex: '1024 x 768' */
		FString KeyName = FString::Printf(TEXT("%i x %i"), SupportedScreenResolution.X, SupportedScreenResolution.Y);
		/** Use Emplace instead of Add to overwrite duplicate keys just in case they occur */
		ScreenResOpts.Emplace(KeyName, SupportedScreenResolution);
	}
}

void ATantrumnHUD::SetScreenRes(FIntPoint InScreenRes,  bool bOverrideCommandLine)
{
	if(UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
	{
		GameUserSettings->SetScreenResolution(InScreenRes);
		/** We need to apply the settings before they take effect  */
		GameUserSettings->ApplyResolutionSettings(bOverrideCommandLine);
	}
}

void ATantrumnHUD::ConfirmGameUserSettings(bool bOverrideCommandLine)
{
	if(UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
	{
		GameUserSettings->ApplySettings(bOverrideCommandLine);
	}
}

void ATantrumnHUD::SetGameUIAssets(const TSoftObjectPtr<UUIElementsAsset> InGameUIElementsAssets)
{
	checkfSlow(InGameUIElementsAssets, "PlayerHUD: Received Null UI Element Assets Soft Ptr");
	const UUIElementsAsset* UIElementAssets = InGameUIElementsAssets.LoadSynchronous();
	checkfSlow(UIElementAssets, "PlayerHUD: Could not load UI Element Assets");

	// TODO Develop ENUM Iterator
	/** Map Asset Settings to UI Element Types and Their Corresponding Slots */
	const FName MenuSlotName = UIElementAssets->GameWidgetClasses.GameMenuWidget.WidgetSlot;
	LevelMenuWidget = AddSlotUI_Implementation(UIElementAssets->GameWidgetClasses.GameMenuWidget.WidgetClass, MenuSlotName);
	
	const FName PlaySlotName = UIElementAssets->GameWidgetClasses.GamePlayWidget.WidgetSlot;
	GameLevelWidget = AddSlotUI_Implementation(UIElementAssets->GameWidgetClasses.GamePlayWidget.WidgetClass, PlaySlotName);
	
	const FName EndSlotName = UIElementAssets->GameWidgetClasses.GameEndScreenWidget.WidgetSlot;
	LevelEndWidget = AddSlotUI_Implementation(UIElementAssets->GameWidgetClasses.GameEndScreenWidget.WidgetClass, EndSlotName);

	DisplayUI();
}

void ATantrumnHUD::ToggleLevelMenuDisplay(const bool bIsDisplayed)
{
	checkfSlow(BaseUIWidget, "PlayerHUD attempted to toggle Level Menu but BaseUIWidget was null");
	BaseUIWidget->ToggleGameMenu(bIsDisplayed);
}

void ATantrumnHUD::DisplayResults(const TArray<FGameResult>& InResults) const
{
	checkfSlow(LevelMenuWidget, "Player HUD - Level Menu Widget was not set prior to call for display")
	LevelEndWidget->DisplayResults(InResults);
}

void ATantrumnHUD::RemoveResults()
{
	if (LevelEndWidget){ LevelEndWidget->RemoveResults(); }
	check(BaseUIWidget)
	BaseUIWidget->ToggleGameMenu(true);
}

void ATantrumnHUD::ToggleDisplayGameTime(const bool bIsDisplayed) const 
{
	check(GameLevelWidget)
	bIsDisplayed ? 
		GameLevelWidget->DisplayGameTimer() :
		GameLevelWidget->HideGameTimer();
}

void ATantrumnHUD::SetMatchTimerSeconds(const float InMatchTimerSeconds) const
{
	check(GameLevelWidget)
	GameLevelWidget->DisplayedMatchTime = FText::AsNumber(InMatchTimerSeconds, &FloatDisplayFormat);
}

void ATantrumnHUD::DisplayMatchStartCountDownTime(const float InMatchStartCountDownTime) const
{
	check(GameLevelWidget)
	GameLevelWidget->InitiateMatchStartTimer(InMatchStartCountDownTime);
	check(BaseUIWidget)
	BaseUIWidget->ToggleGameMenu(false);
}

void ATantrumnHUD::UpdateUIOnFinish() const
{
	if (GameLevelWidget){ GameLevelWidget->UpdateOnFinish(); }
}
