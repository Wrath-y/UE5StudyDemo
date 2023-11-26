// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyGameMode.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "CorpseParty/PlayerController/CorpsePartyPlayerController.h"

void ACorpsePartyGameMode::PlayerEliminated(ACorpsePartyCharacter* ElimmedCharacter, ACorpsePartyPlayerController* VictimController, ACorpsePartyPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
