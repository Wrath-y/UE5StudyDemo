// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "CorpseParty/CorpsePartyComponents/CombatComponent.h"
#include "CorpseParty/Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"

ACorpsePartyCharacter::ACorpsePartyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void ACorpsePartyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	UE_LOG(LogTemp, Warning, TEXT("GetLifetimeReplicatedProps"))
	DOREPLIFETIME_CONDITION(ACorpsePartyCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ACorpsePartyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACorpsePartyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACorpsePartyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACorpsePartyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACorpsePartyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ACorpsePartyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ACorpsePartyCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ACorpsePartyCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACorpsePartyCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ACorpsePartyCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ACorpsePartyCharacter::AimButtonReleased);
}

void ACorpsePartyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}


void ACorpsePartyCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ACorpsePartyCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ACorpsePartyCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ACorpsePartyCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ACorpsePartyCharacter::EquipButtonPressed()
{
	// Server 才会调用 EquipWeapon
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ACorpsePartyCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ACorpsePartyCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ACorpsePartyCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ACorpsePartyCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}


void ACorpsePartyCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("SetOverlappingWeapon"))
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ACorpsePartyCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_OverlappingWeapon"))
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	// 如果结束重叠，则PickupWidget将被设为false
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ACorpsePartyCharacter::IsWeaponEquipped()
{
	return Combat && Combat->EquippedWeapon;
}

bool ACorpsePartyCharacter::IsAiming()
{
	return Combat && Combat->bAiming;
}
