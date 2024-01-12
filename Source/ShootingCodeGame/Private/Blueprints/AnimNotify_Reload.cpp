// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprints/AnimNotify_Reload.h"
#include "GameMode/ShootingCodeGameCharacter.h"
#include "Blueprints/WeaponInterface.h"

void UAnimNotify_Reload::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AShootingCodeGameCharacter* pChar = Cast<AShootingCodeGameCharacter>(MeshComp->GetOwner());
	if (false == IsValid(pChar))
		return;

	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(pChar->m_EquipWeapon);
	if (nullptr == InterfaceObj)
		return;

	InterfaceObj->Execute_EventResetAmmo(pChar->m_EquipWeapon);
}
