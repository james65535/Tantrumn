// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnLevelEndTrigger.h"

#include "TantrumnGameStateBase.h"
#include "Tantrumn/TantrumnCharacterBase.h"

ATantrumnLevelEndTrigger::ATantrumnLevelEndTrigger()
{
	//OnActorBeginOverlap.AddDynamic(this, &ATantrumnLevelEndTrigger::OnOverlapBegin);
}

void ATantrumnLevelEndTrigger::BeginPlay()
{
	Super::BeginPlay();
	TantrumnGameState = GetWorld()->GetGameState<ATantrumnGameStateBase>();
	if(TantrumnGameState)
	{
		OnActorBeginOverlap.AddDynamic(this, &ATantrumnLevelEndTrigger::OnOverlapBegin);	
	}
}

void ATantrumnLevelEndTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (HasAuthority())
	{
		if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(OtherActor))
		{
			TantrumnGameState->OnPlayerReachedEnd(TantrumnCharacterBase);
		}
	}
}
