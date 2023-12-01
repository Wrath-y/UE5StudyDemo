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
	ACorpsePartyGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ACorpsePartyCharacter* ElimmedCharacter, class ACorpsePartyPlayerController* VictimController, ACorpsePartyPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	float LevelStartingTime = 0.f;
protected:
	virtual void BeginPlay() override;

private:
	float CountdownTime = 0.f;
};
