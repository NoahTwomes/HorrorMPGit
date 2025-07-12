// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGameMode.h"
#include "HorrorMP/HorrorMpCharacter.h"
#include "PlayerSpawnPoint.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


AHorrorGameMode::AHorrorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter_C"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	bHasLoadedSpawnPoints = false;
}

void AHorrorGameMode::SetSpawnPoints()
{
	TArray<AActor*> TempActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerSpawnPoint::StaticClass(), TempActors);

	for (AActor* Actor : TempActors)
	{
		if (APlayerSpawnPoint* SpawnPoint = Cast<APlayerSpawnPoint>(Actor))
		{
			PlayerSpawnPoints.Add(SpawnPoint);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Spawn Point Count: %d"), PlayerSpawnPoints.Num());
	bHasLoadedSpawnPoints = true;
}

void AHorrorGameMode::InitGameState()
{
	Super::InitGameState();
}

void AHorrorGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (bHasLoadedSpawnPoints == false)
		SetSpawnPoints();

	for (APlayerSpawnPoint* SpawnPoint : PlayerSpawnPoints)
	{
		if (!SpawnPoint->IsUsed())
		{
			FVector SpawnLocation = SpawnPoint->GetActorLocation();
			if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(PlayerClass, SpawnLocation, FRotator::ZeroRotator))
			{
				UE_LOG(LogTemp, Warning, TEXT("Spawned Pawn to possess"));
				NewPlayer->Possess(Pawn);
				SpawnPoint->SetUsed(true);
				AHorrorMPCharacter* Character = Cast<AHorrorMPCharacter>(PlayerClass);
				Players.Add(Character);
				
				return;
			}
		}
	}
}

