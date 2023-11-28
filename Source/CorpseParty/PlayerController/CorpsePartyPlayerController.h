// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CorpsePartyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CORPSEPARTY_API ACorpsePartyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	virtual void OnPossess(APawn* InPawn) override;
protected:
	virtual void BeginPlay() override;

private:
	class ACorpsePartyHUD* CorpsePartyHUD;
};
