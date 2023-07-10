// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnHUD.h"

#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TantrumnGameWidget.h"
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

void ATantrumnHUD::ToggleLevelMenuDisplay(const bool bIsDisplayed)
{
	if (!LevelMenuWidget)
	{
		LevelMenuWidget = AddSlotUI_Implementation(LevelMenuWidgetClass, MenuUINamedSlotName);
	}
	
	bIsDisplayed ?
		LevelMenuWidget->SetVisibility(ESlateVisibility::Visible) :
		LevelMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
}

UTantrumnGameWidget* ATantrumnHUD::AddSlotUI_Implementation(TSubclassOf<UTantrumnGameWidget> InWidgetClass, FName InSlotName)
{
	if (BaseUIWidget == nullptr)
	{
		/** Base UI is the parent for all Widgets */
		BaseUIWidget = AddWidget(BaseUIWidgetClass);
	}
	if(InWidgetClass != nullptr && BaseUIWidget != nullptr)
	{
		UTantrumnGameWidget* OutWidgetRef = CreateWidget<UTantrumnGameWidget>(GetOwningPlayerController(), InWidgetClass);
		if (OutWidgetRef != nullptr && BaseUIWidget != nullptr)
		{
			BaseUIWidget->SetContentForSlot(InSlotName, OutWidgetRef);
			if(BaseUIWidget->GetContentForSlot(InSlotName) != nullptr)
			{
				return OutWidgetRef;
			}
		}
	}
	return nullptr;
}

void ATantrumnHUD::SetLevelWidgetClass(TSubclassOf<UTantrumnGameWidget> InLevelWidgetClass)
{
	GameLevelWidget = AddSlotUI_Implementation(InLevelWidgetClass, LevelUINamedSlotName);
}

void ATantrumnHUD::ToggleStartMenu_Implementation(bool bShouldDisplay)
{
	if (!GameStartScreenWidget)
	{
		GameStartScreenWidget = AddSlotUI_Implementation(GameStartScreenWidgetClass, StartMenuUINamedSlotName);
	}

	bShouldDisplay ?
		GameStartScreenWidget->SetVisibility(ESlateVisibility::Visible) :
		GameStartScreenWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void ATantrumnHUD::DisplayResults_Implementation()
{
	LevelMenuWidget ?
		LevelMenuWidget->SetVisibility(ESlateVisibility::Visible) :
		LevelEndWidget = AddSlotUI_Implementation(LevelEndWidgetClass, LevelEndWidgetNamedSlotName);
	
	GameLevelWidget->DisplayResults();
}

void ATantrumnHUD::RemoveResults_Implementation()
{
	if (GameLevelWidget)
	{
		GameLevelWidget->RemoveResults();
	}

	if (LevelEndWidget)
	{
		LevelEndWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UTantrumnGameWidget* ATantrumnHUD::AddWidget(const TSubclassOf<UTantrumnGameWidget> InWidgetClass) const
{
	if(InWidgetClass != nullptr)
	{
		if(UTantrumnGameWidget* ReturnWidget = CreateWidget<UTantrumnGameWidget>(GetOwningPlayerController(), InWidgetClass))
		{
			ReturnWidget->AddToPlayerScreen();
			return ReturnWidget;
		}
	}
	return nullptr;
}

void ATantrumnHUD::DisplayGameTimer_Implementation(float GameTimeDuration)
{
	if (GameLevelWidget)
	{
		if (ATantrumnPlayerController* PlayerController =
			Cast<ATantrumnPlayerController>(GetOwningPlayerController()))
		{
			GameLevelWidget->InitiateGameTimer(GameTimeDuration, PlayerController);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not start countdown as gamelevelwidget is null"));
	}
}
