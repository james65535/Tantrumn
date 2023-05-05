// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnLevelEndTrigger.h"

#include "TantrumnGameStateBase.h"
#include "Tantrumn/TantrumnCharacterBase.h"
#include "Tantrumn/TantrumnGameModeBase.h"

void ATantrumnLevelEndTrigger::BeginPlay()
{
	Super::BeginPlay();
}

ATantrumnLevelEndTrigger::ATantrumnLevelEndTrigger()
{
	OnActorBeginOverlap.AddDynamic(this, &ATantrumnLevelEndTrigger::OnOverlapBegin);
}

void ATantrumnLevelEndTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (HasAuthority())
	{
		if (ATantrumnGameStateBase* TantrumnGameState = GetWorld()->GetGameState<ATantrumnGameStateBase>())
		{
			if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(OtherActor))
			{
				TantrumnGameState->OnPlayerReachedEnd(TantrumnCharacterBase);
			}
		}
	}
}




