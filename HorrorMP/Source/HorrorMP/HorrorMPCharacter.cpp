// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorMPCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "K2Node_GetDataTableRow.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Objects/ObjectBase.h"


//////////////////////////////////////////////////////////////////////////
// AHorrorMPCharacter

AHorrorMPCharacter::AHorrorMPCharacter()
{
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), "head");
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
	WalkSpeed = 400.0f;
	RunSpeed = 800.0f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	MaxStamina = 800.0f;
	CurrentStamina = MaxStamina;
	StaminaGain = false;
	StaminaReduce = false;
	bIsSeen = false;
	bIsOverlapping = false;
	bHasKey = false;
	bInRange = false;
	IsAlive = true;

	bReplicates = true;
	SetReplicateMovement(true);
	GetCharacterMovement()->SetIsReplicated(true);


}

void AHorrorMPCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(InteractionTimerHandle, this, &AHorrorMPCharacter::InteractionTrace, 0.01f, IsAlive);
	GetWorld()->GetTimerManager().SetTimer(SightTimerHandle, this, &AHorrorMPCharacter::Sight, 0.01f, true);
	

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

void AHorrorMPCharacter::OnRep_WalkingSpeed()//Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
{
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;//Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
}

void AHorrorMPCharacter::OnRep_SprintSpeed()//Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;//Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
}


void AHorrorMPCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHorrorMPCharacter, bIsSeen);
	DOREPLIFETIME(AHorrorMPCharacter, bInRange);
	DOREPLIFETIME(AHorrorMPCharacter, TItems);
	DOREPLIFETIME(AHorrorMPCharacter, bValInRange);
	DOREPLIFETIME(AHorrorMPCharacter, ViewValue);
	DOREPLIFETIME(AHorrorMPCharacter, bDoorInRange);
	DOREPLIFETIME(AHorrorMPCharacter, WalkSpeed);
	DOREPLIFETIME(AHorrorMPCharacter, RunSpeed);
	DOREPLIFETIME(AHorrorMPCharacter, MaxStamina);
	DOREPLIFETIME(AHorrorMPCharacter, CurrentStamina);
	DOREPLIFETIME(AHorrorMPCharacter, SprintSpeed);
	DOREPLIFETIME(AHorrorMPCharacter, WalkingSpeed);
	DOREPLIFETIME(AHorrorMPCharacter, bIsSprinting);
	DOREPLIFETIME(AHorrorMPCharacter, bExitInRange);
	
	
}


//////////////////////////////////////////////////////////////////////////// Input

void AHorrorMPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHorrorMPCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHorrorMPCharacter::Look);

		
		EnhancedInputComponent->BindAction(CupboardAction, ETriggerEvent::Triggered, this, &AHorrorMPCharacter::Cupboard);
		EnhancedInputComponent->BindAction(CupboardAction, ETriggerEvent::Triggered, this, &AHorrorMPCharacter::Equip);
		
		PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AHorrorMPCharacter::Sprint);
		PlayerInputComponent->BindAction("Run", IE_Released, this, &AHorrorMPCharacter::Walk);
		
	}
}



void AHorrorMPCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
		int min = 0;
		int max = 500;

		int Chance = UKismetMathLibrary::RandomIntegerInRange(min, max);
		if(!bIsCrouched)
		{
			switch (Chance)
            		{
            		case 0:
            			//UGameplayStatics::PlaySoundAtLocation(this, FloorSound, GetActorLocation(),2.0f, 2.0f, 0.0f);
            		//	MakeNoise(1.0f, nullptr, FVector::ZeroVector, 2000.0f);
            			UE_LOG(LogTemp, Warning, TEXT("CREAK"));
            			break;
            			
            		case 4:
            		//	UGameplayStatics::PlaySoundAtLocation(this, FloorSound, GetActorLocation(),2.0f, 2.0f, 0.0f);
            		//	MakeNoise(1.0f, nullptr, FVector::ZeroVector, 2000.0f);
            			UE_LOG(LogTemp, Warning, TEXT("CREAK"));
            			break;
            			
            		}
            		if(UKismetMathLibrary::RandomIntegerInRange(min, max))
            		{
            			
            		}
		}
		
	}
}

void AHorrorMPCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AHorrorMPCharacter::Sprint()
{
	if(HasAuthority())
	{
		if (CurrentStamina > 0)
        	{
        		SprintSpeed = RunSpeed; //Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
        		StaminaGain = false;
        		StaminaReduce = true;
        		TimerHandles();
			   ForceNetUpdate(); 
        	}
        	else
        	{
        		SprintSpeed = WalkSpeed;
        		bIsSprinting = false;
        		ForceNetUpdate(); 
        	}

		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
        }
	else
	{
		Server_Sprint();
	}
}



bool AHorrorMPCharacter::Server_Sprint_Validate()
{
	return true;
}

void AHorrorMPCharacter::Server_Sprint_Implementation()
{
	
	
	if (CurrentStamina > 0)
	{
		SprintSpeed = RunSpeed; //Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
		StaminaGain = false;
		StaminaReduce = true;
		TimerHandles();
		ForceNetUpdate(); //Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
	}
	else
	{
		SprintSpeed = WalkSpeed;
		bIsSprinting = false;
		ForceNetUpdate(); //Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
	}

	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AHorrorMPCharacter::Walk()
{

	if(HasAuthority())
	{
			SprintSpeed = WalkSpeed; //Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
        	StaminaGain = true;
        	StaminaReduce = false;
        	TimerHandles();
		ForceNetUpdate(); //Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
	}
	else
	{
		Server_Walk();
	}
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

}

bool AHorrorMPCharacter::Server_Walk_Validate()
{
	return true;
}

void AHorrorMPCharacter::Server_Walk_Implementation()
{

	
	
		   SprintSpeed = WalkSpeed; //Ai was only used for replicating sprinting and walking as there were some issues with the method I was orignally using.
        	StaminaGain = true;
        	StaminaReduce = false;
        	TimerHandles();
        	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
        	ForceNetUpdate(); 
	
}

void AHorrorMPCharacter::Sight()
{
	Server_sight();
}


bool AHorrorMPCharacter::Server_sight_Validate()
{
	return true;

}

void AHorrorMPCharacter::Server_sight_Implementation()
{

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(this, 2);
	FHitResult HitObjects;
	

	FVector Start = FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetComponentRotation().Vector() * 725.0f;  //{600.0, 0.0, 0.0};
	FVector End = FirstPersonCameraComponent->GetComponentRotation().Vector() * 3500.0f + Start;

	
	
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 530.0f, TraceObjects, false, IgnoreActors, EDrawDebugTrace::None, HitObjects, true, FLinearColor::Red);

	if(HitObjects.bBlockingHit == true)
	{
		bIsSeen = true;
	}
	else
	{
		bIsSeen = false;
	}
	
}


void AHorrorMPCharacter::InteractionTrace()
{
	if(HasAuthority() && IsAlive)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		FVector STart =	PlayerController->PlayerCameraManager->GetCameraLocation();
		FVector ENd = PlayerController->PlayerCameraManager->GetCameraRotation().Vector() * 400.0f + STart;
	
		FHitResult HitResult;

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
		TraceParams.bTraceComplex = true;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.bIgnoreTouches = true;
	
		//DrawDebugLine(GetWorld(), STart, ENd, FColor::Red, false, 2.0f, 0, 3.0f);
		if(GetWorld()->LineTraceSingleByObjectType(HitResult, STart, ENd, ECC_WorldDynamic, TraceParams) == true)
		{
			AActor* HitActor = Cast<AActor>(HitResult.GetActor());
			AObjectBase* ItemRef = Cast<AObjectBase>(Cast<AActor>(HitResult.GetActor()));
			UStaticMeshComponent* HitMesh = Cast<UStaticMeshComponent>(HitResult.Component);
			if(HitMesh && !HitMesh->ComponentTags.IsEmpty())
			{
				bInRange = true;
			}
		
			if(HitActor && !HitActor->Tags.IsEmpty() && !HitActor->ActorHasTag("Door") && !HitActor->ActorHasTag("Exit"))
			{
				bInRange = true;
			}
			 if(HitActor && HitActor->ActorHasTag("Door") && HitResult.Distance <= 200)
			{
				bDoorInRange = true;
			}
			if(HitActor && HitActor->ActorHasTag("Exit") && HitResult.Distance <= 300)
			{
				bExitInRange = true;
			}
			 if(HitActor && HitActor->Tags.Num() > 1)
			{
				bValInRange = true;
				ViewValue = ItemRef->IValue;
			}
			else if(HitActor && HitActor->Tags.Num() <= 1)
			{
				bValInRange = false;
				ViewValue = 0.0f;
			}
		
		}
		else
		{
			bInRange = false;
			bValInRange = false;
			bDoorInRange = false;
			bExitInRange = false;
		}
		
	}
	else
	{
		Server_InteractionTrace();
	}
}

bool AHorrorMPCharacter::Server_InteractionTrace_Validate()
{
	return true;
}

void AHorrorMPCharacter::Server_Cupboard_Implementation()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	FVector Start =	PlayerController->PlayerCameraManager->GetCameraLocation();
	FVector End = PlayerController->PlayerCameraManager->GetCameraRotation().Vector() * 400.0f + Start;
	
	
	FHitResult HitResult;

	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bIgnoreTouches = true;
	
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 3.0f);
	if(GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End, ECC_WorldDynamic, TraceParams) == true)
	{
		AActor* HitActor = Cast<AActor>(HitResult.GetActor());
		AObjectBase* ItemRef = Cast<AObjectBase>(Cast<AActor>(HitResult.GetActor()));
		UStaticMeshComponent* HitMesh = Cast<UStaticMeshComponent>(HitResult.Component);
        		if(HitMesh && HitMesh->ComponentHasTag("L"))
        		{
        			UE_LOG(LogTemp, Warning, TEXT("HIT MESH DOOR"));
        			if(HitMesh->GetComponentRotation().Yaw <= 90)
        			{
        				HitMesh->AddWorldRotation({0.0, 90.0, 0.0});
        				UE_LOG(LogTemp, Warning, TEXT("HIT MESH"));
        			}
        			else
        			{
        				HitMesh->AddWorldRotation({0.0, -90.0, 0.0});
        			}
        		}
        		else if(HitMesh && HitMesh->ComponentHasTag("R"))
        		{
        			if(HitMesh->GetComponentRotation().Yaw <= 0)
        			{
        				HitMesh->AddWorldRotation({0.0, 90.0, 0.0});
        			}
        			else
        			{
        				HitMesh->AddWorldRotation({0.0, -90.0, 0.0});
        			}
        		}
		if(HitActor && HitActor->Tags.Num() == 1)
		{
			
			FName RowName = HitActor->Tags[0];
			TItems.Add(RowName);
			HitActor->Destroy();
			
		
		}
		if(HitActor && HitActor->Tags.Num() > 1)
		{
			float CurValue;
			HitActor->Destroy();
			AddValue(ItemRef->IValue);
			CurValue = ItemRef->IValue;
			
			
		}
		
	}
}

void AHorrorMPCharacter::Equip()
{
	const FName RowName = TItems[0];
	const FItemsStruct* Item = ItemDataTable->FindRow<FItemsStruct>(RowName, "");
	UKismetSystemLibrary::PrintString(GetWorld(), Item->RowName.ToString());
	
	
}

void AHorrorMPCharacter::Server_InteractionTrace_Implementation()
{
	if (IsAlive)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		FVector STart =	PlayerController->PlayerCameraManager->GetCameraLocation();
		FVector ENd = PlayerController->PlayerCameraManager->GetCameraRotation().Vector() * 400.0f + STart;


	
		FHitResult HitResult;

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
		TraceParams.bTraceComplex = true;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.bIgnoreTouches = true;
	
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 3.0f);
		if(GetWorld()->LineTraceSingleByObjectType(HitResult, STart, ENd, ECC_WorldDynamic, TraceParams) == true)
		{
			AActor* HitActor = Cast<AActor>(HitResult.GetActor());
			AObjectBase* ItemRef = Cast<AObjectBase>(Cast<AActor>(HitResult.GetActor()));
			UStaticMeshComponent* HitMesh = Cast<UStaticMeshComponent>(HitResult.Component);
			if(HitMesh && !HitMesh->ComponentTags.IsEmpty())
			{
				bInRange = true;
			}
		
			if(HitActor && !HitActor->Tags.IsEmpty() && !HitActor->ActorHasTag("Door") && !HitActor->ActorHasTag("Exit"))
			{
				bInRange = true;
			}
			if(HitActor && HitActor->ActorHasTag("Door") && HitResult.Distance <= 200)
			{
				bDoorInRange = true;
			}
			if(HitActor && HitActor->ActorHasTag("Exit") && HitResult.Distance <= 300)
			{
				bExitInRange = true;
			}
			if(HitActor && HitActor->Tags.Num() > 1)
			{
				bValInRange = true;
				ViewValue = ItemRef->IValue;
			}
			else if(HitActor && HitActor->Tags.Num() <= 1)
			{
				bValInRange = false;
				ViewValue = 0.0f;
			}
		
		}
		else
		{
			bInRange = false;
			bValInRange = false;
			bDoorInRange = false;
			bExitInRange = false;
		}
	}
}


void AHorrorMPCharacter::Cupboard()
{
	if(HasAuthority())
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		FVector Start =	PlayerController->PlayerCameraManager->GetCameraLocation();
		FVector End = PlayerController->PlayerCameraManager->GetCameraRotation().Vector() * 400.0f + Start;
	
		FHitResult HitResult;

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
		TraceParams.bTraceComplex = true;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.bIgnoreTouches = true;
	
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 3.0f);
		if(GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End, ECC_WorldDynamic, TraceParams) == true)
		{
			
			AActor* HitActor = Cast<AActor>(HitResult.GetActor());
			AObjectBase* ItemRef = Cast<AObjectBase>(Cast<AActor>(HitResult.GetActor()));
			UStaticMeshComponent* HitMesh = Cast<UStaticMeshComponent>(HitResult.Component);
			if(HitMesh && HitMesh->ComponentHasTag("L"))
			{
				UE_LOG(LogTemp, Warning, TEXT("HIT MESH DOOR"));
				if(HitMesh->GetComponentRotation().Yaw <= 90)
				{
					HitMesh->AddWorldRotation({0.0, 90.0, 0.0});
					UE_LOG(LogTemp, Warning, TEXT("HIT MESH"));
				}
				else
				{
					HitMesh->AddWorldRotation({0.0, -90.0, 0.0});
				}
			}
			else if(HitMesh && HitMesh->ComponentHasTag("R"))
			{
				if(HitMesh->GetComponentRotation().Yaw <= 0)
				{
					HitMesh->AddWorldRotation({0.0, 90.0, 0.0});
				}
				else
				{
					HitMesh->AddWorldRotation({0.0, -90.0, 0.0});
				}
			}
			if(HitActor && HitActor->Tags.Num() == 1)
			{
			
				FName RowName = HitActor->Tags[0];
				TItems.Add(RowName);
				HitActor->Destroy();
				
			}
			if(HitActor && HitActor->Tags.Contains("Evidence"))
			{
				float CurValue;
				UE_LOG(LogTemp, Warning, TEXT("WORKED"))
				HitActor->Destroy();
				AddValue(ItemRef->IValue);
				CurValue = ItemRef->IValue;

				
			}
		
		}
		
	}
	else
	{
		Server_Cupboard();
	}
}

bool AHorrorMPCharacter::Server_Cupboard_Validate()
{
	return true;
}

void AHorrorMPCharacter::StaminaGainFunction()
{
	if (CurrentStamina < MaxStamina)
	{
		CurrentStamina += 1;


	}
	else
	{
		StaminaGain = false;
	}
}

void AHorrorMPCharacter::StaminaReduceFunction()
{
	if (CurrentStamina > 0)
	{
		CurrentStamina -= 1;
	}
	else
	{
		StaminaReduce = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	}
}

void AHorrorMPCharacter::TimerHandles()
{
	GetWorld()->GetTimerManager().SetTimer(StaminaGainTimerHandle, this, &AHorrorMPCharacter::StaminaGainFunction, 0.01f, StaminaGain);
	GetWorld()->GetTimerManager().SetTimer(StaminaReduceTimerHandle, this, &AHorrorMPCharacter::StaminaReduceFunction, 0.01f, StaminaReduce);
	
}



