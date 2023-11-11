// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyAnimInstance.h"
#include "CorpsePartyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCorpsePartyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(TryGetPawnOwner());
}

void UCorpsePartyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (CorpsePartyCharacter == nullptr)
	{
		CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(TryGetPawnOwner());
	}
	if (CorpsePartyCharacter == nullptr) return;

	FVector Velocity = CorpsePartyCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = CorpsePartyCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = CorpsePartyCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
