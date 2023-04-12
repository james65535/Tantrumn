// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnLevelEndTrigger.h"
#include "Tantrumn/TantrumnCharacterBase.h"
#include "Tantrumn/TantrumnGameModeBase.h"

void ATantrumnLevelEndTrigger::BeginPlay()
{
	Super::BeginPlay();
	GameModeRef = GetWorld()->GetAuthGameMode<ATantrumnGameModeBase>();
}

ATantrumnLevelEndTrigger::ATantrumnLevelEndTrigger()
{
	OnActorBeginOverlap.AddDynamic(this, &ATantrumnLevelEndTrigger::OnOverlapBegin);
}

void ATantrumnLevelEndTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	
	if (ATantrumnCharacterBase* PlayerCharacter = Cast<ATantrumnCharacterBase>(OtherActor))
	{
		if (PlayerCharacter->IsPlayerControlled())
		{
			GameModeRef->PLayerReachedEnd(PlayerCharacter->GetNetOwningPlayer()->GetPlayerController(GetWorld()));
		}
	}
}




