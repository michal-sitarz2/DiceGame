// Fill out your copyright notice in the Description page of Project Settings.


#include "MPDicePlayerState.h"
#include "MPDicePlayer.h"
#include "Net/UnrealNetwork.h"

AMPDicePlayerState::AMPDicePlayerState()
    : PlayerIdx(-1)
    , DiceCount(0) 
    , bRolled(false)
	, bDiceHidden(true) { }


AMPDicePlayer* AMPDicePlayerState::GetDicePawn() const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController not found"));
		return nullptr;
	}

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn not found"));
		return nullptr;
	}

	AMPDicePlayer* DicePawn = Cast<AMPDicePlayer>(Pawn);
	return DicePawn;
}

void AMPDicePlayerState::RevealDice()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("RevealDice called on client! Should only run on server."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Server] Dice Reveal"));
	bDiceHidden = false;
	RevealDiceValues = DiceValues;
	OnDiceRevealed.Broadcast(RevealDiceValues);
}

void AMPDicePlayerState::HideDice()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("HideDice called on client! Should only run on server."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Server] Dice Hide"));
	bDiceHidden = true;
	RevealDiceValues.Empty();
	OnDiceHidden.Broadcast(DiceCount);
}


void AMPDicePlayerState::OnRep_HideDice()
{
	if (bDiceHidden)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client] Dice Hide"));
		OnDiceHidden.Broadcast(DiceCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client] Dice Reveal"));
		OnDiceRevealed.Broadcast(RevealDiceValues);
	}
}


void AMPDicePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMPDicePlayerState, PlayerIdx);
    DOREPLIFETIME(AMPDicePlayerState, DiceCount);
    DOREPLIFETIME(AMPDicePlayerState, bRolled);
	DOREPLIFETIME(AMPDicePlayerState, bDiceHidden);
	DOREPLIFETIME(AMPDicePlayerState, RevealDiceValues);
}