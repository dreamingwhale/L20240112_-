// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseItem.h"

// Sets default values
ABaseItem::ABaseItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	m_Scene = CreateDefaultSubobject<USceneComponent>("Scene");

	SetRootComponent(m_Scene);
	m_StaticMesh->SetupAttachment(RootComponent);
	m_StaticMesh->SetCollisionProfileName("OverlapAllDynamic");

	bReplicates = true;
	SetReplicateMovement(true);

	m_StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseItem::OnItemBeginOverlap);
}

// Called every frame
void ABaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	m_StaticMesh->AddRelativeRotation(FRotator(0.0f, 1.0f, 0.0f));
}

void ABaseItem::OnItemBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IItemInterface* InterfaceObj = Cast<IItemInterface>(OtherActor);
	if (nullptr == InterfaceObj)
		return;

	InterfaceObj->Execute_EventGetItem(OtherActor, m_eItemType);
}

// Called when the game starts or when spawned
void ABaseItem::BeginPlay()
{
	Super::BeginPlay();
	
}


