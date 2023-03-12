// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerController.h"
#include "GameFramework/Character.h"

void ATantrumnPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (InputComponent)
	{
		InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::RequestJump);
		InputComponent->BindAxis("MoveX", this, &ATantrumnPlayerController::RequestMoveX);
		InputComponent->BindAxis("MoveY", this, &ATantrumnPlayerController::RequestMoveY);
		InputComponent->BindAxis("LookPitch", this, &ATantrumnPlayerController::ATantrumnPlayerController::RequestLookPitch);
		InputComponent->BindAxis("LookYaw", this, &ATantrumnPlayerController::ATantrumnPlayerController::RequestLookYaw);
	}
}

void ATantrumnPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InputComponent)
	{
		InputComponent->ClearActionBindings();
	}
}

void ATantrumnPlayerController::RequestJump()
{
	if (GetCharacter())
	{
		GetCharacter()->Jump();
	}
}

void ATantrumnPlayerController::RequestMoveX(float AxisValue)
{
	if  (AxisValue != 0.f)
	{
		if (GetCharacter())
		{
			GetCharacter()->AddMovementInput(GetCharacter()->GetActorForwardVector(),AxisValue);
		}
	}
}

void ATantrumnPlayerController::RequestMoveY(float AxisValue)
{
	if  (AxisValue != 0.f)
	{
		if (GetCharacter() && AxisValue != 0.f)
		{
			GetCharacter()->AddMovementInput(GetCharacter()->GetActorRightVector(),AxisValue);
		}
	}
}

void ATantrumnPlayerController::RequestLookPitch(float AxisValue)
{
	if  (AxisValue != 0.f)
	{
		AddPitchInput(AxisValue * BaseLookPitchRate * GetWorld()->DeltaTimeSeconds);
	}
}

void ATantrumnPlayerController::RequestLookYaw(float AxisValue)
{
	if  (AxisValue != 0.f)
	{
		AddYawInput(AxisValue * BaseLookYawRate * GetWorld()->DeltaTimeSeconds);
	}
}
