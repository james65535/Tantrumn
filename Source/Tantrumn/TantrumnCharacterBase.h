// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractInterface.h"
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
	Aiming			UMETA(DisplayName = "Aiming")
};

UCLASS()
class TANTRUMN_API ATantrumnCharacterBase : public ACharacter, public IInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATantrumnCharacterBase(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void FellOutOfWorld(const UDamageType& dmgType) override;
	
	/*** General Movement ***/

	// Sprint modifiers
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RequestSprintStart();
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RequestSprintEnd();
	
	// Called when player needs to be stunned from fall landing or otherwise
	UFUNCTION(BlueprintCallable, Category = "Stun")
	void RequestStunStart(const float DurationMultiplier);
	
	/*
	 * Note:
	 * Changing a replicated variable's value on the client is not recommended. The value will continue to differ from
	 * the server's value until the next time the server detects a change and sends an update.
	 * If the server's copy of the property does not change very often, it could be a long time before the client receives a correction.
	 * 
	 */
	// Stun
	UPROPERTY(BlueprintReadOnly, replicatedUsing=OnRep_IsStunned)
	bool bIsStunned;
	UFUNCTION()
	virtual void OnRep_IsStunned();

	// Rescue State Accessor
	bool IsBeingRescued() const { return bIsBeingRescued; }

	// Hover State Accessor
	UFUNCTION(BlueprintPure)
	bool IsHovering() const;

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
	bool IsPullingObject() const { return CharacterThrowState == ECharacterThrowState::Pulling ||
		CharacterThrowState == ECharacterThrowState::RequestingPull; }
	UFUNCTION(BlueprintPure)
	ECharacterThrowState GetCharacterThrowState() const { return CharacterThrowState; }

	// For AI as it is not able to look at objects.  Bypasses tracing done on tick()
	UFUNCTION(BlueprintCallable)
	bool AttemptPullObjectAtLocation(const FVector& InLocation);

	// Reaction to being hit by a throwable object
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void NotifyHitByThrowable(AThrowableActor* InThrowable);
	
	// Apply an effect from a held throwable object
	void RequestUseObject();

	// Aiming Mechanics
	UFUNCTION(BlueprintCallable, Category = "Throw")
	bool CanAim() const { return CharacterThrowState == ECharacterThrowState::Attached;}
	UFUNCTION(BlueprintCallable, Category = "Throw")
	bool IsAiming() const { return CharacterThrowState == ECharacterThrowState::Aiming;}
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void RequestAim();
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void RequestStopAim();

	// Internal throw functions
	UFUNCTION(BlueprintPure, Category = "Throw")
	bool IsThrowing() { return CharacterThrowState == ECharacterThrowState::Throwing; }
	void OnThrowableAttached(AThrowableActor* InThrowableActor);
	bool CanThrowObject() const { return CharacterThrowState == ECharacterThrowState::Aiming;}

	// Animation Montage for times of celebration such as winning race
	UFUNCTION(Server, Reliable)
	void ServerPlayCelebrateMontage();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Custom character movement Component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	class UTantrumnCharMovementComponent* TantrumnCharMoveComp;

	// Server RPCs for character sprint
	UFUNCTION(Server, Reliable)
	void ServerSprintStart();
	UFUNCTION(Server, Reliable)
	void ServerSprintEnd();

	// Hit checks for throwable object targets when performing pull action
	void SphereCastPlayerView();
	void SphereCastActorTransform();
	void LineCastActorTransform();
	void ProcessTraceResult(const FHitResult& HitResult,  bool bHighlight = true);

	// Network capable RPCs
	UFUNCTION(Server, Reliable)
	void ServerPullObject(AThrowableActor* InThrowableActor);
	UFUNCTION(Server, Reliable)
	void ServerRequestPullObject(bool bIsPulling);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestThrowObject();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRequestThrowObject();
	UFUNCTION(Client, Reliable)
	void ClientThrowableAttached(AThrowableActor* InThrowableActor);
	UFUNCTION(Server, Reliable)
	void ServerBeginThrow();
	UFUNCTION(Server, Reliable)
	void ServerFinishThrow();
	UFUNCTION() // Note: this function definition varies from class video
	void OnRep_CharacterThrowState(const ECharacterThrowState& OldCharacterThrowState);
	// This was not in the class download file
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(Server, Reliable)
	void ServerRequestToggleAim(bool IsAiming);

	// Used for throw animation
	bool PlayThrowMontage();
	bool PlayCelebrateMontage();
	void UpdateThrowMontagePlayRate();
	void UnbindMontage();
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayCelebrateMontage();
	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);
	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ThrowMontage = nullptr;
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* CelebrateMontage = nullptr;

	// Throwing defaults
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterThrowState, Category = "Throw")
	ECharacterThrowState CharacterThrowState = ECharacterThrowState::None;
	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0", Unit = "ms"))
	float ThrowSpeed = 5000.0f;

	// Called when Landed from jump or fall
	virtual void Landed(const FHitResult& Hit) override;

	// Impact settings for when landing after fall or jump
	UPROPERTY(EditDefaultsOnly, Category = "Fall")
	float MinImpactSpeed = 500.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Fall")
	float MaxImpactSpeed = 1000.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Fall")
	float MinImpactStunMultiplier = 1.0;
	UPROPERTY(EditDefaultsOnly, Category = "Fall")
	float MaxImpactStunMultiplier = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	USoundCue* HeavyLandSound = nullptr;

	// Handle falling out of the world and rescue mechanics
	UPROPERTY(Replicated)
	FVector LastGroundPosition = FVector::ZeroVector;
	UPROPERTY(ReplicatedUsing = OnRep_IsBeingRescued)
	bool bIsBeingRescued = false;
	UFUNCTION()
	void OnRep_IsBeingRescued();
	UPROPERTY(EditAnywhere, Category = "KillZ")
	float TimeToRescuePlayer = 1.f;
	FVector FallOutOfWorldPosition = FVector::ZeroVector;
	float CurrentRescueTime = 0.0f;
	// These only happen on the server.  The variable bIsBeingRescued is replicated
	void StartRescue();
	void UpdateRescue(float DeltaTime);
	void EndRescue();

private:

	// Reference to object which can be thrown
	UPROPERTY()
	AThrowableActor* ThrowableActor;

	/**
	 * Checks for valid location in increasing circumference of sphere traces on X/Y plane
	 * @param StartLoc The centrepoint of the circumference from which you want to check
	 * @param ValidLoc If function returns true then ValidPos is updated with result otherwise unmodified
	 * @param SearchRadius The radius utilised for checks
	 * @param NumRetries The number of attempts to perform before abandoning query
	 * @return TRUE if any hit is found
	 */
	bool IsLandingValid(FVector StartLoc, FVector& ValidLoc, float SearchRadius , uint8 NumRetries);
	
	// *** Interface Work *** //
	virtual void ApplyEffect_Implementation(EEffectType EffectType, bool bIsBuff) override;
	void EndEffect();
	bool bIsUnderEffect = false;
	bool bIsEffectBuff = false;
	float DefaultEffectCooldown = 5.0f;
	float EffectCooldown = 0.0f;
	float SprintSpeed = 2.0f;
	EEffectType CurrentEffect = EEffectType::NONE;
};
