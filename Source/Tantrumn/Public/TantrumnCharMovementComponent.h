// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TantrumnCharMovementComponent.generated.h"

/**
 * CMC works with three different sets of moves: Input State, Intermediate State, and Output State
 * For a detailed description: https://docs.google.com/document/d/1UO6Ww6Lfpti3YElVdo9uioTUtQJQ9CoSLvd9kF8hvJo/edit
 *
 * High Level Flow:
 * Client: On tick - Receive input state, run logic to derive intermediates state which produces output state on end.
 * Client: Send SavedMoves to Server containing three moves: OldMove(the oldest,unacknowledged important move), PendingMove, and NewMove.
 * Server: Receive SavedState, process movement, reply back to client with an ack or movement correction.
 * Client: If receive ack, then move on, if correction then process correction next tick.
 * Note: CMC is not replicated, CMC ultimately uses an RPC call through Character to send data from client to server
 *
 * More Detailed Flow:
 * 1. Client: OnTick - ReplicateMoveToServer: Creates SavedMoves using SetMoveFor to setup data for a predictive correction,
 *		CanCombineWith to dedupe moves, perform Movement with PerformMovement(),
 * 2. Client: Within ReplicateMoveToServer on every Other Frame - run CallServerMove (uses GetCompressedFlags) or CallServerMovedPacked(if this is enabled) .
 * 3. Client: CallServerMove and CallServerMovePacked will create an archive container of the three moves (ClientNewMove, PendingMove, OldMove).
 * 4. Client: If using CallServerMovePacked serialise the archive via bitstream serialisation.
 * 5. Client: Within Character class, ServerMovePacked_ClientSend runs an RPC call to server via Character. Note: Character class ServerMove is deprecated.
 * 5. Server: In Character class on RPC run ServerMovePacked_ServerReceive to deserialize archive and process via ServerMove_HandleMoveData.
 * 6. Server: Call ServerMove_PerformMovement on the three Movements: Old, Pending, new using CharacterController class.
 * 7. Server: Using ServerMoveHandleClientError, review error message from client if it generated one when performing move locally on client.
 * 8. Server: If performed movements don't match up to client movements then queue PendingAdjustment with FClientAdjustment. Adjustments are added to ServerData object.
 * 9. Server: On UNetDriver::TickFlush, send serialized ServerData to client using SendClientAdjustment(contains ack or Adjustments).
 * 10. Server: ServerSendMoveResponse fills up a response container archive and serialize.
 * 11. Server: Call ClientMoveResponsePacked which is an unreliable RPC to send response back to client.
 * 12. Client: MoveResponsePacked_ClientReceive is called in Character class amd deserialise data from bits to variables.
 * 13. Client: If client received an ack from server then ack the ack with ClientAckGoodMove RPC
 * 14. Client: If adjustment is needed then different adjustment calls can be made, most likely ClientAdjustPosition.
 * 15. Client: Unpack Server state and ack that via AckMove and copy server data to local movement data and set bUpDatePosition = true
 * 16. Client: On tick - If bUpDatePosition then replay movements via ClientUpdatePositionAfterServerUpdate then toggle bUpDatePosition false
 */

// Stun delegates
// TODO refactor to non-dynamic
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStunStartDelegate);

UENUM(BlueprintType)
enum ECustomMovementMode
{
	TANTRUMNMOVE_None			UMETA(Hidden),
	TANTRUMNMOVE_Stun			UMETA(DisplayName = "Stunned"),
};

class ATantrumnCharacterBase;

UCLASS()
class TANTRUMN_API UTantrumnCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Tantrumn : public FSavedMove_Character
	{
		typedef FSavedMove_Tantrumn Super;
		
	public:

		/* The following can be used for better network optimisation of vars which need to replice on tick */
		// enum CompressedFlags
		// {
		// 	FLAG_Stun			= 0x10,
		// };
		
		// Flags
		uint8 Saved_bWantsToStun:1;
		
		FSavedMove_Tantrumn();

		// Checks two moves, current move and new move, if the moves equal each other then return true
		// So need to override this to check stun and return false if not the same
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		// Resets a SavedMove object
		virtual void Clear() override;

		// This is what replicates movement data, 8 bit integer, 4 bits are available to use
		// this is a translation layer, use RPC for large data like vectors. Flags are sent every frame
		virtual uint8 GetCompressedFlags() const override;

		// Capture state data of character movement component, set the respective saved variables for the current snapshot
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		// take data in the SavedMove and apply it to the current state of the character movement component, 
		// kind of the opposite to Setmove.  Remember there will be an array of SavedMoves - Old, Pending, New
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	// We want to use our saved move instead of default one. Keep in mind these classes do not inherit from uobject,
	// so may need standard c++ code like 'new' and won't be able to up or downcast
	class FNetworkPredictionData_Client_Tantrumn : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Tantrumn(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	// Stun properties
	bool Safe_bWantsToStun; // This must be the same on client owner and server
	UPROPERTY(EditDefaultsOnly)
	float StunDuration = 1.0f;
	float StunStartTime;
	FTimerHandle TimerHandle_EnterStun;

	UPROPERTY(Transient)
	ATantrumnCharacterBase* TantrumnCharacter;
	
	void OnStunCoolDownFinished();
	
public:
	// Set default values
	UTantrumnCharMovementComponent();

	// This checks if client prediction data is null and if it is then make it or return, no need to recreate it if it already exists
	// Operates on heap
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
	// General Movement
	UFUNCTION(Category = "Movement")
	void DoSprintStart();
	UFUNCTION(Category = "Movement")
	void DoSprintEnd();
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Movement")
	float SprintModifier = 2.0f;
	UPROPERTY(BlueprintReadWrite,  EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 600.0f;
	
	// Stun Character Methods
	UFUNCTION(BlueprintCallable, Category = "Stun")
	bool IsStunned() const;
	UFUNCTION(BlueprintCallable, meta=(ClampMin="0.1", UIMin="0.1"), Category = "Stun")
	void RequestStun(bool bClientSimulation = false, float DurationMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, meta=(ClampMin="0.1", UIMin="0.1"), Category = "Stun")
	void RequestStunEnd(bool bClientSimulation = false);

protected:
	virtual void InitializeComponent() override;

	// Stun Properties
	// If true, try to stun on next update. If false, try to stop stun on next update. TODO fixup
	UPROPERTY(Category="Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 bWantsToStun:1;
	UPROPERTY(BlueprintAssignable)
	FStunStartDelegate  StunStartDelegate;

	// This is what runs the commands like ApplyStun().  Automatically called at the end of every perform move.
	// Allows to write some movement logic that runs regardless of what movement mode you're in
	// This function uses a safe variable to update a non safe variable
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Final step: pass in flags, then set state of movement component based on these flags
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

private:
	UFUNCTION()
	void OnRep_Stun();
};
