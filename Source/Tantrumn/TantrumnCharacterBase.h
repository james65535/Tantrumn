// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TantrumnCharacterBase.generated.h"

UCLASS()
class TANTRUMN_API ATantrumnCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATantrumnCharacterBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when Landed
	virtual void Landed(const FHitResult& Hit) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Fall Impact, Stun")
	bool bIsStunned = false;

	//UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Fall Impact, Stun")
	FRichCurve StunRelease;

	UPROPERTY()
	float StunTime = 0.0f;

	UPROPERTY()
	float StunBeginTimeStamp = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float StunDuration = 1.0f;

	FTimerHandle StunTimer;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	float MinImpactSpeed = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	float MaxImpactSpeed = 1600.0f;

	void OnStunBegin(float StunLength);
	void OnStunEnd();
};
