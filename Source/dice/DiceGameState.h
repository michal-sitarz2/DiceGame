// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DiceGameState.generated.h"

USTRUCT(BlueprintType)
struct FBetData
{
	GENERATED_BODY()

	// PlayerID of the player who made the bet
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerIdx = -1;

	// The number of dice faces bet
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumFaces = 0;

	// The face value bet (1-6)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Face = 0;

	// Default Constructor (required for USTRUCTs)
	FBetData() = default;

	// Custom Constructor
	FBetData(int32 InNumFaces, int32 InFace, int32 InPlayerIdx)
		: PlayerIdx(InPlayerIdx), NumFaces(InNumFaces), Face(InFace) {
	}
};

UCLASS()
class DICE_API ADiceGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ADiceGameState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/* Index of the current player taking turn */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DiceGame")
	int32 CurrentPlayerIdx;

	/* Index of the player who challenged the bet */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DiceGame")
	int32 ChallengerIdx;

	/* Index of the player who lost the round */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DiceGame")
	int32 LoserIdx;

	/* Current active bet */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DiceGame")
	FBetData CurrentBet;
};
