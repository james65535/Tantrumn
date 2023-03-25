// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Sound/SoundCue.h"
#include "TantrumnCharacterBase.generated.h"

class AThrowableActor;

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

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when Landed
	virtual void Landed(const FHitResult& Hit) override;

	void RequestThrowObject();
	void RequestPullObjectStart();
	void RequestPullObjectStop();
	void ResetThrowableObject();

	void OnThrowableAttached(AThrowableActor* InThrowableActor);

	bool CanThrowObject() const { return CharacterThrowState == ECharacterThrowState::Attached;}
	
	UFUNCTION(BlueprintPure)
	bool IsPullingObject() const { return CharacterThrowState == ECharacterThrowState::Pulling || CharacterThrowState == ECharacterThrowState::RequestingPull || CharacterThrowState == ECharacterThrowState::Attached; }

	UFUNCTION(BlueprintPure)
	ECharacterThrowState GetCharacterThrowState() const { return CharacterThrowState; }

	UFUNCTION(BlueprintPure)
	bool IsStunned() const { return bIsStunned; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SphereCastPlayerView();
	void SphereCastActorTransform();
	void LineCastActorTransform();
	void ProcessTraceResult(const FHitResult& HitResult);

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

	void OnStunBegin(float StunLength);
	void OnStunEnd();
	void UpdateStun();

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	float MinImpactSpeed = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	float MaxImpactSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	USoundCue* HeavyLandSound = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Throw")
	ECharacterThrowState CharacterThrowState = ECharacterThrowState::None;

	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0", Unit = "ms"))
	float ThrowSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ThrowMontage = nullptr;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;

private:

	UPROPERTY()
	AThrowableActor* ThrowableActor;
};
