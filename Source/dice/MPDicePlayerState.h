// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MPDicePlayerState.generated.h"

UCLASS()
class DICE_API AMPDicePlayerState : public APlayerState
{
	GENERATED_BODY()

public: 
	AMPDicePlayerState();

	UPROPERTY(Replicated)
	int32 PlayerIdx;

	UPROPERTY(ReplicatedUsing = OnRep_DiceCount)
	int32 DiceCount;

	UPROPERTY(Replicated)
	bool bRolled;
	
	TArray<int32> DiceValues;

protected:
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UFUNCTION()
	void OnRep_DiceCount();
};
