// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ObjectBase.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AObjectBase::AObjectBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AObjectBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AObjectBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AObjectBase, IValue);

	
}

// Called every frame
void AObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

