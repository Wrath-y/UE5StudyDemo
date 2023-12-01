// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyPlayerController.h"
#include "CorpseParty/HUD/CorpsePartyHUD.h"
#include "CorpseParty/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"

void ACorpsePartyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CorpsePartyHUD = Cast<ACorpsePartyHUD>(GetHUD());
}

void ACorpsePartyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
}

void ACorpsePartyPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
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

void ACorpsePartyPlayerController::SetHUDScore(float Score)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		CorpsePartyHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ACorpsePartyPlayerController::SetHUDDefeats(int32 Defeats)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		CorpsePartyHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void ACorpsePartyPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		CorpsePartyHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ACorpsePartyPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		CorpsePartyHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ACorpsePartyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ACorpsePartyCharacter* CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(InPawn);
	if (CorpsePartyCharacter)
	{
		SetHUDHealth(CorpsePartyCharacter->GetHealth(), CorpsePartyCharacter->GetMaxHealth());
	}
}

void ACorpsePartyPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		CorpsePartyHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ACorpsePartyPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}

	CountdownInt = SecondsLeft;
}

void ACorpsePartyPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ACorpsePartyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ACorpsePartyPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ACorpsePartyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
