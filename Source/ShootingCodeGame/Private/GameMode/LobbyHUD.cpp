// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyHUD.h"
#include "Blueprint/UserWidget.h"
#include "GameMode/LobbyPlayerState.h"

void ALobbyHUD::BeginPlay()
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

void ALobbyHUD::BindPlayerState(ALobbyPlayerState* PlayerState)
{
	if (IsValid(PlayerState))
	{
		PlayerState->m_Dele_UpdateUserName.AddDynamic(this, &ALobbyHUD::OnUpdateUserName);
		OnUpdateUserName(PlayerState->m_UserName);
	}
}

void ALobbyHUD::OnUpdateUserName_Implementation(const FString& UserName)
{
}
