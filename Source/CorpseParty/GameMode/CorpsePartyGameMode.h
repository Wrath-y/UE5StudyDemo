// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CorpsePartyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CORPSEPARTY_API ACorpsePartyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class ACorpsePartyCharacter* ElimmedCharacter, class ACorpsePartyPlayerController* VictimController, ACorpsePartyPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
};
