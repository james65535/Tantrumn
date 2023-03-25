// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnCharMovementComponent.h"


UTantrumnCharMovementComponent::UTantrumnCharMovementComponent()
{
	
}

void UTantrumnCharMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

}

bool UTantrumnCharMovementComponent::IsStunned() const
{
	return true;
}


