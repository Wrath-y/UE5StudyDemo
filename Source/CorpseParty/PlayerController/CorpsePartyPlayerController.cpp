// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyPlayerController.h"
#include "CorpseParty/HUD/CorpsePartyHUD.h"
#include "CorpseParty/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "CorpseParty/GameMode/CorpsePartyGameMode.h"
#include "CorpseParty/PlayerState/CorpsePartyPlayerState.h"
#include "CorpseParty/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"

void ACorpsePartyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CorpsePartyHUD = Cast<ACorpsePartyHUD>(GetHUD());
	ServerCheckMatchState();
}

void ACorpsePartyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACorpsePartyPlayerController, MatchState);
}


void ACorpsePartyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
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


void ACorpsePartyPlayerController::ServerCheckMatchState_Implementation()
{
	ACorpsePartyGameMode* GameMode = Cast<ACorpsePartyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime);
	}
}

void ACorpsePartyPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (CorpsePartyHUD && MatchState == MatchState::WaitingToStart)
	{
		CorpsePartyHUD->AddAnnouncement();
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


void ACorpsePartyPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->Announcement &&
		CorpsePartyHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		CorpsePartyHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ACorpsePartyPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void ACorpsePartyPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (CorpsePartyHUD && CorpsePartyHUD->CharacterOverlay)
		{
			CharacterOverlay = CorpsePartyHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
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

void ACorpsePartyPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ACorpsePartyPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ACorpsePartyPlayerController::HandleMatchHasStarted()
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	if (CorpsePartyHUD)
	{
		CorpsePartyHUD->AddCharacterOverlay();
		if (CorpsePartyHUD->Announcement)
		{
			CorpsePartyHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
