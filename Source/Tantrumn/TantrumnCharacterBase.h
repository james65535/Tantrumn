// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TantrumnCharMovementComponent.h"
#include "GameFramework/Character.h"
#include "TantrumnCharacterBase.generated.h"

class AThrowableActor;
class USoundCue;

UENUM(BlueprintType)
enum class ECharacterThrowState : uint8
{
	None			UMETA(DisplayName = "None"),
	RequestingPull  UMETA(DisplayName = "RequestingPull"),
	Pulling			UMETA(DisplayName = "Pulling"),
	Attached		UMETA(DisplayName = "Attached"),
	Throwing		UMETA(DisplayName = "Throwing"),
};

UCLASS()
class TANTRUMN_API ATantrumnCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATantrumnCharacterBase(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// General Movement
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RequestSprintStart();
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RequestSprintEnd();

	// Blueprint accessible throw and pull functions
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void RequestThrowObject();
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void RequestPullObjectStart();
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void RequestPullObjectStop();
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void ResetThrowableObject();
	UFUNCTION(BlueprintPure)
	bool IsPullingObject() const { return CharacterThrowState ==
		ECharacterThrowState::Pulling ||
			CharacterThrowState == ECharacterThrowState::RequestingPull ||
				CharacterThrowState == ECharacterThrowState::Attached; }
	UFUNCTION(BlueprintPure)
	ECharacterThrowState GetCharacterThrowState() const { return CharacterThrowState; }

	// Internal throw functions
	void OnThrowableAttached(AThrowableActor* InThrowableActor);
	bool CanThrowObject() const { return CharacterThrowState == ECharacterThrowState::Attached;}

	// Called when player needs to be stunned from fall landing or otherwise
	UFUNCTION(BlueprintCallable, Category = "Stun")
	void RequestStunStart(const float DurationMultiplier);
	UFUNCTION(BlueprintPure, Category = "Stun")
	bool IsStunned() const { return TantrumnCharMoveComp->IsStunned();}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Custom character movement Component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	UTantrumnCharMovementComponent* TantrumnCharMoveComp;

	// Hit checks for throwable object targets when performing pull action
	void SphereCastPlayerView();
	void SphereCastActorTransform();
	void LineCastActorTransform();
	void ProcessTraceResult(const FHitResult& HitResult);

	// Used for throw animation
	bool PlayThrowMontage();
	void UnbindMontage();
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);
	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ThrowMontage = nullptr;
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;

	// Throwing defaults
	UPROPERTY(VisibleAnywhere, Category = "Throw")
	ECharacterThrowState CharacterThrowState = ECharacterThrowState::None;
	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0", Unit = "ms"))
	float ThrowSpeed = 5000.0f;

	// Called when Landed from jump or fall
	virtual void Landed(const FHitResult& Hit) override;

	// Stun delegate multicast listener
	UFUNCTION()
	void OnStunEnd() const;
	FDelegateHandle OnStunEndHandle;
	
	// Impact settings for when landing after fall or jump
	UPROPERTY(EditDefaultsOnly, Category = "Fall")
	float MinImpactSpeed = 500.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Fall")
	float MaxImpactSpeed = 1000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Fall")
	float MinImpactStunMultiplier = 0.5;
	UPROPERTY(EditDefaultsOnly, Category = "Fall")
	float MaxImpactStunMultiplier = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	USoundCue* HeavyLandSound = nullptr;

private:

	// Reference to object which can be thrown
	UPROPERTY()
	AThrowableActor* ThrowableActor;
};
