// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyPlayerController.h"
#include "GameMode/ShootingGameInstance.h"
#include "GameMode/LobbyPlayerState.h"

void ALobbyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("(Server)OnPossess"));

	ResClientPossess();
}

void ALobbyPlayerController::ResClientPossess_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("(Client)ResClientPossess"));

	UShootingGameInstance* pGameInst = Cast<UShootingGameInstance>(GetGameInstance());
	if (false == IsValid(pGameInst))
		return;

	FString strName = pGameInst->GetUserName();
	ReqChangeUserName(strName);
}

void ALobbyPlayerController::ReqChangeUserName_Implementation(const FString& NewName)
{
	ALobbyPlayerState* pPS = Cast<ALobbyPlayerState>(PlayerState);
	if (false == IsValid(pPS))
		return;

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("(Server)ReqChangeUserName"));

	pPS->SetUserName(NewName);
}
