// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/EntryHUD.h"
#include "Blueprint/UserWidget.h"

void AEntryHUD::BeginPlay()
{
	Super::BeginPlay();

	check(HudWidgetClass);

	HudWidget = CreateWidget<UUserWidget>(GetWorld(), HudWidgetClass);
	HudWidget->AddToViewport();

	APlayerController* pPlayer0 = GetWorld()->GetFirstPlayerController();
	if (IsValid(pPlayer0) == false)
		return;

	pPlayer0->SetInputMode(FInputModeUIOnly());
	pPlayer0->bShowMouseCursor = true;
}
