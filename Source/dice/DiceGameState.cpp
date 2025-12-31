// Fill out your copyright notice in the Description page of Project Settings.


#include "DiceGameState.h"
#include "Net/UnrealNetwork.h"

ADiceGameState::ADiceGameState() 
	: CurrentPlayerIdx(-1)
	, ChallengerIdx(-1)
	, LoserIdx(-1)
{ }

void ADiceGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Synchrozined to users when changed by the server
	DOREPLIFETIME(ADiceGameState, CurrentPlayerIdx);
	DOREPLIFETIME(ADiceGameState, ChallengerIdx);
	DOREPLIFETIME(ADiceGameState, LoserIdx);
	DOREPLIFETIME(ADiceGameState, CurrentBet);
}