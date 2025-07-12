// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorGameMode.generated.h"

/**
 * 
 */
UCLASS()
class HORRORMP_API AHorrorGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AHorrorGameMode();

protected:
	bool bHasLoadedSpawnPoints;
	TArray<class APlayerSpawnPoint*> PlayerSpawnPoints;
	//TArray<class ItemSpawner*> ItemSpawners; will come back and create item spawner c++ class
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "PlayerSettings")
	TArray<class AHorrorMPCharacter*> Players;

	UPROPERTY(EditAnywhere, Category = "PlayerSettings")
	TSubclassOf<class AHorrorMPCharacter> PlayerClass;

protected:
	void SetSpawnPoints();
	void InitGameState();

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

};
