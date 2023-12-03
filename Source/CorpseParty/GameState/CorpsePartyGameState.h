// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CorpsePartyGameState.generated.h"

/**
 * 
 */
UCLASS()
class CORPSEPARTY_API ACorpsePartyGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class ACorpsePartyPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ACorpsePartyPlayerState*> TopScoringPlayers;
private:

	float TopScore = 0.f;
};
