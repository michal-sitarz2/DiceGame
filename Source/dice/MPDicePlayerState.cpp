// Fill out your copyright notice in the Description page of Project Settings.


#include "MPDicePlayerState.h"
#include "MPDicePlayer.h"
#include "Net/UnrealNetwork.h"

AMPDicePlayerState::AMPDicePlayerState()
    : PlayerIdx(-1)
    , DiceCount(0) 
    , bRolled(false) { }


void AMPDicePlayerState::OnRep_DiceCount()
{
    
}

void AMPDicePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMPDicePlayerState, PlayerIdx);
    DOREPLIFETIME(AMPDicePlayerState, DiceCount);
    DOREPLIFETIME(AMPDicePlayerState, bRolled);
}