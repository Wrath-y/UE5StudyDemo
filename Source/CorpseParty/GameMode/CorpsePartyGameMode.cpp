// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyGameMode.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "CorpseParty/PlayerController/CorpsePartyPlayerController.h"
#include "CorpseParty/PlayerState/CorpsePartyPlayerState.h"
#include "CorpseParty/GameState/CorpsePartyGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ACorpsePartyGameMode::ACorpsePartyGameMode()
{
	bDelayedStart = true;
}

void ACorpsePartyGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ACorpsePartyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ACorpsePartyGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACorpsePartyPlayerController* CorpsePartyPlayer = Cast<ACorpsePartyPlayerController>(*It);
		if (CorpsePartyPlayer)
		{
			CorpsePartyPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ACorpsePartyGameMode::PlayerEliminated(ACorpsePartyCharacter* ElimmedCharacter, ACorpsePartyPlayerController* VictimController, ACorpsePartyPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ACorpsePartyPlayerState* AttackerPlayerState = AttackerController ? Cast<ACorpsePartyPlayerState>(AttackerController->PlayerState) : nullptr;
	ACorpsePartyPlayerState* VictimPlayerState = VictimController ? Cast<ACorpsePartyPlayerState>(VictimController->PlayerState) : nullptr;

	ACorpsePartyGameState* CorpsePartyGameState = GetGameState<ACorpsePartyGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && CorpsePartyGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		CorpsePartyGameState->UpdateTopScore(AttackerPlayerState);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
}

void ACorpsePartyGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ACorpsePartyGameMode::PlayerLeftGame(ACorpsePartyPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ACorpsePartyGameState* CorpsePartyGameState = GetGameState<ACorpsePartyGameState>();
	if (CorpsePartyGameState && CorpsePartyGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		CorpsePartyGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ACorpsePartyCharacter* CharacterLeaving = Cast<ACorpsePartyCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}