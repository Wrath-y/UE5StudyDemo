// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyGameMode.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "CorpseParty/PlayerController/CorpsePartyPlayerController.h"
#include "CorpseParty/PlayerState/CorpsePartyPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

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

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddToDefeats"))
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
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
