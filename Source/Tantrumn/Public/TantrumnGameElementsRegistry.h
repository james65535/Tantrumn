// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TantrumnGameStateBase.h"
#include "UIElementsAsset.h"
#include "Engine/DataAsset.h"
#include "TantrumnGameElementsRegistry.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUMN_API UTantrumnGameElementsRegistry : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tantrumn UI")
	TMap<ETantrumnGameType, TSoftObjectPtr<UUIElementsAsset>> GameTypeUIMapping;
	
};
