// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MPDicePlayerState.generated.h"

class AMPDicePlayer;

UCLASS()
class DICE_API AMPDicePlayerState : public APlayerState
{
	GENERATED_BODY()

public: 
	AMPDicePlayerState();

	UPROPERTY(Replicated)
	int32 PlayerIdx;

	UPROPERTY(Replicated)
	int32 DiceCount;

	UPROPERTY(Replicated)
	bool bRolled;

	UPROPERTY(ReplicatedUsing = OnRep_HideDice)
	bool bDiceHidden;

	UPROPERTY(Replicated) // Using = OnRep_RevealedDice
	TArray<int32> RevealDiceValues;

	
	TArray<int32> DiceValues;

	AMPDicePlayer* GetDicePawn() const;
	void RevealDice();
	void HideDice();

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDiceRevealed, const TArray<int32>&);
	FOnDiceRevealed OnDiceRevealed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDiceHidden, int32);
	FOnDiceHidden OnDiceHidden;
	
protected:
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UFUNCTION()
	void OnRep_HideDice();
};
