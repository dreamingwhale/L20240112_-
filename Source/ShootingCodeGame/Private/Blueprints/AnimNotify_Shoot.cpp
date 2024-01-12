// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprints/AnimNotify_Shoot.h"
#include "GameMode/ShootingCodeGameCharacter.h"
#include "Blueprints/WeaponInterface.h"

void UAnimNotify_Shoot::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AShootingCodeGameCharacter* pChar = Cast<AShootingCodeGameCharacter>(MeshComp->GetOwner());
	if (false == IsValid(pChar))
		return;

	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(pChar->m_EquipWeapon);
	if (nullptr == InterfaceObj)
		return;

	InterfaceObj->Execute_EventShoot(pChar->m_EquipWeapon);
}
