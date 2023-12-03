// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyGameState.h"
#include "Net/UnrealNetwork.h"
#include "CorpseParty/PlayerState/CorpsePartyPlayerState.h"

void ACorpsePartyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACorpsePartyGameState, TopScoringPlayers);
}

void ACorpsePartyGameState::UpdateTopScore(class ACorpsePartyPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}