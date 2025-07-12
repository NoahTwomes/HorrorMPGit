// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSpawnPoint.h"


APlayerSpawnPoint::APlayerSpawnPoint()
{
	bIsUsed = false;
}

bool APlayerSpawnPoint::IsUsed()
{
	return bIsUsed;
}

void APlayerSpawnPoint::SetUsed(bool Used)
{
	bIsUsed = Used;
}
