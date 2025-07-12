// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "PlayerSpawnPoint.generated.h"

/**
 * 
 */
UCLASS()
class HORRORMP_API APlayerSpawnPoint : public ATargetPoint
{
	GENERATED_BODY()
public:
	APlayerSpawnPoint();

protected:
	bool bIsUsed;

public:
	bool IsUsed();
	void SetUsed(bool Used);

};
