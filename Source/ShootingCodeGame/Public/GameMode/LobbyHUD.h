// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyHUD.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTINGCODEGAME_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	// To add mapping context
	virtual void BeginPlay();

public:
	void BindPlayerState(class ALobbyPlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnUpdateUserName(const FString& UserName);

	void OnUpdateUserName_Implementation(const FString& UserName);

public:
	// TSubclassOf : 지정한 클래스와 클래스를 상속한 모든 클래스를 지정 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> HudWidgetClass;

	UPROPERTY(BlueprintReadWrite)
	UUserWidget* HudWidget;
};
