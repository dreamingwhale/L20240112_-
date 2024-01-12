// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprints/ActorSpawner.h"

// Sets default values
AActorSpawner::AActorSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AActorSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (false == HasAuthority())
		return;

	if (false == IsValid(SpawnClass))
		return;

	GetWorld()->SpawnActor<AActor>(SpawnClass, GetActorTransform());
}

// Called every frame
void AActorSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

