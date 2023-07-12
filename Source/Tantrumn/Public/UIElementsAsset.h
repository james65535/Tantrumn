// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TantrumnGameWidget.h"
#include "Engine/DataAsset.h"
#include "UIElementsAsset.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUMN_API UUIElementsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Tantrumn UI")
	FName Name;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Tantrumn UI")
	FText Description;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Tantrumn UI")
	FGameUI GameWidgetClasses;
	
};
