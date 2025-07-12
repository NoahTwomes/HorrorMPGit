// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Engine/DataTable.h"
#include "HorrorMPCharacter.generated.h"



class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

USTRUCT(BlueprintType)
struct FItemsStruct
{
	GENERATED_BODY()
public:
		
		
	FName RowName = "";
	int RowAmount = 0;
};


UCLASS(config=Game)
class AHorrorMPCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* GrabAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* CupboardAction;

	
public:
	AHorrorMPCharacter();

protected:
	virtual void BeginPlay();

public:

	UFUNCTION(BlueprintImplementableEvent)
	void Evidence();

	UFUNCTION(BlueprintImplementableEvent)
	void AddValue(float Amount);
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
	float WalkSpeed;
	
	UPROPERTY(ReplicatedUsing = OnRep_WalkingSpeed)
	float WalkingSpeed;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
	float RunSpeed;

	UPROPERTY(ReplicatedUsing = OnRep_SprintSpeed)
	float SprintSpeed;

	UFUNCTION()
	void OnRep_WalkingSpeed();

	UFUNCTION()
	void OnRep_SprintSpeed();

	UPROPERTY(BlueprintReadWrite, EditAnywhere,  Replicated)
	float MaxStamina;

	UPROPERTY(BlueprintReadWrite, EditAnywhere,  Replicated)
	float CurrentStamina;
	
	UPROPERTY()
	bool StaminaGain;

	UPROPERTY()
	bool StaminaReduce;

	UPROPERTY(Replicated)
	bool bIsSprinting;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsSeen;

	UPROPERTY(BlueprintReadWrite)
	bool bIsOverlapping;

	UPROPERTY(BlueprintReadWrite)
	bool bHasKey;

	UPROPERTY(BlueprintReadWrite)
	bool IsAlive;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bInRange;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bDoorInRange;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bExitInRange;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bValInRange;
	

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Replicated)
	TArray<FName> TItems;

	UPROPERTY(EditAnywhere)
	USoundBase* FloorSound;

	UPROPERTY(BlueprintReadWrite, Replicated)
	float ViewValue;

	UPROPERTY(EditAnywhere)
	UDataTable* ItemDataTable;


	

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void Sprint();

	UFUNCTION(Server,Reliable)
	void Server_Sprint();
	bool Server_Sprint_Validate();
	void Server_Sprint_Implementation();

	UFUNCTION(BlueprintCallable)
	void Walk();


	UFUNCTION(Server,Reliable)
	void Server_Walk();
	bool Server_Walk_Validate();
	void Server_Walk_Implementation();

	UFUNCTION()
	void Sight();
	
	UFUNCTION(Server, Reliable)
	void Server_sight();
	bool Server_sight_Validate();
	void Server_sight_Implementation();

	void InteractionTrace();
	UFUNCTION(Server, Reliable)
	void Server_InteractionTrace();
	bool Server_InteractionTrace_Validate();
	void Server_InteractionTrace_Implementation();

	void Cupboard();
	UFUNCTION(Server, Reliable)
	void Server_Cupboard();
	bool Server_Cupboard_Validate();
	void Server_Cupboard_Implementation();

	void Equip();
	//UFUNCTION(Server,Reliable)
	//void Server_Equip();
	//void Server_Equip_Validate();
	//void Server_Equip_Implementation();

	FTimerHandle StaminaGainTimerHandle;
	FTimerHandle StaminaReduceTimerHandle;

	FTimerHandle InteractionTimerHandle;

	FTimerHandle SightTimerHandle;


	//void InteractionTrace();
	
	void StaminaGainFunction();
	void StaminaReduceFunction();
	void TimerHandles();


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	


};

