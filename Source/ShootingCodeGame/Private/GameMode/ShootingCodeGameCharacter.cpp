// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/ShootingCodeGameCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"// DOREPLIFETIME 사용을 위해 추가
#include "GameMode/ShootingPlayerState.h"
#include "Blueprints/Weapon.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AShootingCodeGameCharacter

AShootingCodeGameCharacter::AShootingCodeGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AShootingCodeGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);            

	DOREPLIFETIME(AShootingCodeGameCharacter, ControlRot);
}

void AShootingCodeGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	check(NameTagClass);

	NameTagWidget = CreateWidget<UUserWidget>(GetWorld(), NameTagClass);
	NameTagWidget->AddToViewport();

	FTimerManager& timerManager = GetWorld()->GetTimerManager();
	timerManager.SetTimer(th_Nametag, this, 
		&AShootingCodeGameCharacter::EventUpdateNametag, 0.01f, true);

	BindPlayerState();
}

void AShootingCodeGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() == true)
	{
		ControlRot = GetControlRotation();
	}
}

float AShootingCodeGameCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, 
		FString::Printf(TEXT("TakeDamage DamageAmount=%f EventInstigator=%s DamageCauser=%s"), 
			DamageAmount,
			*EventInstigator->GetName(),
			*DamageCauser->GetName()));

	AShootingPlayerState* ps = Cast<AShootingPlayerState>(GetPlayerState());
	if (false == IsValid(ps))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("PS is not valid"));
		return 0.0f;
	}

	ps->AddDamage(DamageAmount);

	return DamageAmount;
}

//////////////////////////////////////////////////////////////////////////
// Network

void AShootingCodeGameCharacter::ReqPressF_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("ReqPressF"));
	AActor* pNearestActor = FindNearestWeapon();

	if (false == IsValid(pNearestActor))
		return;

	if (nullptr != m_EquipWeapon)
	{
		m_EquipWeapon->SetOwner(nullptr);
	}

	pNearestActor->SetOwner(GetController());

	ResPressF(pNearestActor);
}

void AShootingCodeGameCharacter::ResPressF_Implementation(AActor* PickUpActor)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("ResPressF"));

	if (nullptr != m_EquipWeapon)
	{
		IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(m_EquipWeapon);
		if (nullptr == InterfaceObj)
			return;

		InterfaceObj->Execute_EventDrop(m_EquipWeapon, this);
		m_EquipWeapon = nullptr;
	}

	m_EquipWeapon = PickUpActor;

	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(m_EquipWeapon);
	if (nullptr == InterfaceObj)
		return;

	InterfaceObj->Execute_EventPickUp(m_EquipWeapon, this);
}

void AShootingCodeGameCharacter::ResPressFClient_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("ResPressFClient"));
}

void AShootingCodeGameCharacter::ReqTrigger_Implementation()
{
	ResTrigger();
}

void AShootingCodeGameCharacter::ResTrigger_Implementation()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(m_EquipWeapon);
	if (nullptr == InterfaceObj)
		return;

	InterfaceObj->Execute_EventTrigger(m_EquipWeapon);
}


void AShootingCodeGameCharacter::ReqReload_Implementation()
{
	ResReload();
}


void AShootingCodeGameCharacter::ResReload_Implementation()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(m_EquipWeapon);
	if (nullptr == InterfaceObj)
		return;

	InterfaceObj->Execute_EventReload(m_EquipWeapon);
}


void AShootingCodeGameCharacter::ReqDrop_Implementation()
{
	if (false == IsValid(m_EquipWeapon))
		return;

	m_EquipWeapon->SetOwner(nullptr);
	ResDrop();
}

void AShootingCodeGameCharacter::ResDrop_Implementation()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(m_EquipWeapon);
	if (nullptr == InterfaceObj)
		return;

	InterfaceObj->Execute_EventDrop(m_EquipWeapon, this);
	m_EquipWeapon = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AShootingCodeGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShootingCodeGameCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShootingCodeGameCharacter::Look);

		// Trigger
		EnhancedInputComponent->BindAction(TriggerAction, ETriggerEvent::Started, this, &AShootingCodeGameCharacter::Trigger);

		// PressF
		EnhancedInputComponent->BindAction(PressFAction, ETriggerEvent::Started, this, &AShootingCodeGameCharacter::PressF);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AShootingCodeGameCharacter::Reload);

		// Drop
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &AShootingCodeGameCharacter::Drop);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AShootingCodeGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AShootingCodeGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AShootingCodeGameCharacter::Trigger(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Trigger"));
	ReqTrigger();
}

void AShootingCodeGameCharacter::PressF(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("PressF"));
	ReqPressF();
}

void AShootingCodeGameCharacter::Reload(const FInputActionValue& Value)
{
	ReqReload();
}

void AShootingCodeGameCharacter::Drop(const FInputActionValue& Value)
{
	ReqDrop();
}

void AShootingCodeGameCharacter::EquipTestWeapon(TSubclassOf<class AWeapon> WeaponClass)
{
	if (false == HasAuthority())
		return;

	m_EquipWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, FVector(0, 0, 0), FRotator(0, 0, 0));

	AWeapon* pWeapon = Cast<AWeapon>(m_EquipWeapon);
	if (false == IsValid(pWeapon))
		return;

	pWeapon->m_pOwnChar = this;

	TestWeaponSetOwner();

	m_EquipWeapon->AttachToComponent(GetMesh()
		, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("weapon"));
}

void AShootingCodeGameCharacter::TestWeaponSetOwner()
{
	if (IsValid(GetController()))
	{
		m_EquipWeapon->SetOwner(GetController());
		return;
	}

	FTimerManager& tm = GetWorld()->GetTimerManager();
	tm.SetTimer(th_BindSetOwner, this, &AShootingCodeGameCharacter::TestWeaponSetOwner, 0.1f, false);
}

AActor* AShootingCodeGameCharacter::FindNearestWeapon()
{
	TArray<AActor*> actors;
	GetCapsuleComponent()->GetOverlappingActors(actors, AWeapon::StaticClass());

	double nearestDist = 9999999.0f;
	AActor* pNearestActor = nullptr;
	for (AActor* pTarget : actors)
	{
		if (m_EquipWeapon == pTarget)
			continue;

		double dist = FVector::Distance(GetActorLocation(), pTarget->GetActorLocation());
		if (dist >= nearestDist)
			continue;

		nearestDist = dist;
		pNearestActor = pTarget;
	}

	return pNearestActor;
}

void AShootingCodeGameCharacter::EventUpdateNametag_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("EventUpdateNametag C++"));
}

void AShootingCodeGameCharacter::EventUpdateNametagHp_Implementation(float CurHp, float MaxHp)
{
}

void AShootingCodeGameCharacter::BindPlayerState()
{
	AShootingPlayerState* pPS = Cast<AShootingPlayerState>(GetPlayerState());
	if (IsValid(pPS))
	{
		pPS->m_Dele_UpdateHp.AddDynamic(this, &AShootingCodeGameCharacter::EventUpdateNametagHp);
		EventUpdateNametagHp(pPS->m_CurHp, 100);
		return;
	}

	FTimerManager& timerManger = GetWorld()->GetTimerManager();
	timerManger.SetTimer(th_BindPlayerState, this, 
		&AShootingCodeGameCharacter::BindPlayerState, 0.01f, false);
}

void AShootingCodeGameCharacter::EventGetItem_Implementation(EItemType itemType)
{
	switch (itemType)
	{
	case EItemType::IT_MAG:
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("EventGetItem Mag"));

		AShootingPlayerState* pPS = Cast<AShootingPlayerState>(GetPlayerState());
		if (IsValid(pPS))
		{
			pPS->AddMag();
		}
	}
	break;

	case EItemType::IT_HEAL:
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("EventGetItem Heal"));
	}
	break;

	default:
		break;
	}

}
