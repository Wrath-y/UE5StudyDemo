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
#include "CorpseParty/CorpsePartyComponents/CombatComponent.h"
#include "CorpseParty/Weapon/Weapon.h"
#include "CorpseParty/GameState/CorpsePartyGameState.h"
#include "Components/Image.h"

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
	CheckPing(DeltaTime);
}

void ACorpsePartyPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying =
		CorpsePartyHUD && CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->HighPingAnimation &&
		CorpsePartyHUD->CharacterOverlay->IsAnimationPlaying(CorpsePartyHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
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
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ACorpsePartyPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->ShieldBar &&
		CorpsePartyHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		CorpsePartyHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		CorpsePartyHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
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
		bInitializeScore = true;
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
		bInitializeDefeats = true;
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
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
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
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}


void ACorpsePartyPlayerController::HighPingWarning()
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->HighPingImage &&
		CorpsePartyHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		CorpsePartyHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		CorpsePartyHUD->CharacterOverlay->PlayAnimation(
			CorpsePartyHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

void ACorpsePartyPlayerController::StopHighPingWarning()
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->HighPingImage &&
		CorpsePartyHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		CorpsePartyHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (CorpsePartyHUD->CharacterOverlay->IsAnimationPlaying(CorpsePartyHUD->CharacterOverlay->HighPingAnimation))
		{
			CorpsePartyHUD->CharacterOverlay->StopAnimation(CorpsePartyHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ACorpsePartyPlayerController::ServerCheckMatchState_Implementation()
{
	ACorpsePartyGameMode* GameMode = Cast<ACorpsePartyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void ACorpsePartyPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
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
		SetHUDShield(CorpsePartyCharacter->GetShield(), CorpsePartyCharacter->GetMaxShield());
		CorpsePartyCharacter->UpdateHUDAmmo();
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
		if (CountdownTime < 0.f)
		{
			CorpsePartyHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		
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
		if (CountdownTime < 0.f)
		{
			CorpsePartyHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		CorpsePartyHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

	void ACorpsePartyPlayerController::SetHUDGrenades(int32 Grenades)
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	bool bHUDValid = CorpsePartyHUD &&
		CorpsePartyHUD->CharacterOverlay &&
		CorpsePartyHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		CorpsePartyHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void ACorpsePartyPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
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
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				ACorpsePartyCharacter* CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(GetPawn());
				if (CorpsePartyCharacter && CorpsePartyCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(CorpsePartyCharacter->GetCombat()->GetGrenades());
				}
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
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
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
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ACorpsePartyPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ACorpsePartyPlayerController::HandleMatchHasStarted()
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	if (CorpsePartyHUD)
	{
		if (CorpsePartyHUD->CharacterOverlay == nullptr) CorpsePartyHUD->AddCharacterOverlay();
		if (CorpsePartyHUD->Announcement)
		{
			CorpsePartyHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ACorpsePartyPlayerController::HandleCooldown()
{
	CorpsePartyHUD = CorpsePartyHUD == nullptr ? Cast<ACorpsePartyHUD>(GetHUD()) : CorpsePartyHUD;
	if (CorpsePartyHUD)
	{
		CorpsePartyHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = CorpsePartyHUD->Announcement && 
			CorpsePartyHUD->Announcement->AnnouncementText && 
			CorpsePartyHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			CorpsePartyHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			CorpsePartyHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			
			ACorpsePartyGameState* CorpsePartyGameState = Cast<ACorpsePartyGameState>(UGameplayStatics::GetGameState(this));
			ACorpsePartyPlayerState* CorpsePartyPlayerState = GetPlayerState<ACorpsePartyPlayerState>();
			if (CorpsePartyGameState && CorpsePartyPlayerState)
			{
				TArray<ACorpsePartyPlayerState*> TopPlayers = CorpsePartyGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == CorpsePartyPlayerState)
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}

				CorpsePartyHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	ACorpsePartyCharacter* CorpsePartyCharacter = Cast<ACorpsePartyCharacter>(GetPawn());
	if (CorpsePartyCharacter && CorpsePartyCharacter->GetCombat())
	{
		CorpsePartyCharacter->bDisableGameplay = true;
		CorpsePartyCharacter->GetCombat()->FireButtonPressed(false);
	}
}
