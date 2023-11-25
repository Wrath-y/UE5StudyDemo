// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyPlayerController.h"
#include "CorpseParty/HUD/CorpsePartyHUD.h"
#include "CorpseParty/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ACorpsePartyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CorpsePartyHUD = Cast<ACorpsePartyHUD>(GetHUD());
}

void ACorpsePartyPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;

	bool bHUDValid = CorpsePartyHUD && 
		CorpsePartyHUD->CharacterOverlay && 
		CorpsePartyHUD->CharacterOverlay->HealthBar && 
		CorpsePartyHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		CorpsePartyHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		CorpsePartyHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}