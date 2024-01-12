// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"// DOREPLIFETIME 사용을 위해 추가
#include "GameMode/LobbyHUD.h"

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, m_UserName);
}


void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	UpdateBind();
}

void ALobbyPlayerState::UpdateBind()
{
	APlayerController* pPlayer0 = GetWorld()->GetFirstPlayerController();
	if (pPlayer0)
	{
		ALobbyHUD* pHud = Cast<ALobbyHUD>(pPlayer0->GetHUD());
		if (pHud)
		{
			pHud->BindPlayerState(this);
			return;
		}
	}

	FTimerManager& TimeMgr = GetWorldTimerManager();
	TimeMgr.SetTimer(th_UpdateBind, this, &ALobbyPlayerState::UpdateBind, 0.01f, false);
}

void ALobbyPlayerState::SetUserName(const FString& UserName)
{
	m_UserName = UserName;
	OnRep_UserName();
}

void ALobbyPlayerState::OnRep_UserName()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, 
		FString::Printf(TEXT("(Client)OnRep_UserName=%s"), *m_UserName));

	if (m_Dele_UpdateUserName.IsBound())
		m_Dele_UpdateUserName.Broadcast(m_UserName);
}

