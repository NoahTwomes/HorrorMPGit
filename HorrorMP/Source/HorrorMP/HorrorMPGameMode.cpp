// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorMPGameMode.h"
#include "HorrorMPCharacter.h"
#include "UObject/ConstructorHelpers.h"

AHorrorMPGameMode::AHorrorMPGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
