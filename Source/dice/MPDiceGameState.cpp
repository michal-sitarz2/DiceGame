// Fill out your copyright notice in the Description page of Project Settings.


#include "MPDiceGameState.h"
#include "MPDicePlayerState.h"
#include "Net/UnrealNetwork.h"

AMPDiceGameState::AMPDiceGameState()
{
	ActivePlayer = nullptr;
	Phase = ETurnPhase::Waiting;
}

int32 AMPDiceGameState::GetTotalDice()
{
	int32 TotalDice = 0;
	for (APlayerState* Player : PlayerArray)
	{
		if (AMPDicePlayerState* MPPlayerState = Cast<AMPDicePlayerState>(Player))
		{
			TotalDice += MPPlayerState->DiceCount;
		}
	}
	return TotalDice;
}

void AMPDiceGameState::BroadcastBetChanged() 
{ 
	OnBetChanged.Broadcast(CurrentBet); 
}

void AMPDiceGameState::BroadcastGameStarted()
{
	OnGameStarted.Broadcast();
}

void AMPDiceGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPDiceGameState, Phase);
	DOREPLIFETIME(AMPDiceGameState, ActivePlayer);
	DOREPLIFETIME(AMPDiceGameState, CurrentBet);
	DOREPLIFETIME(AMPDiceGameState, bGameStarted);
}



