// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MPDiceGameMode.generated.h"

UENUM(BlueprintType)
enum class ETurnModeSelect : uint8
{
	Next,           // Go to next player in order
	Random,         // Pick a random player
	Specific        // Choose a specific player by index
};

class AMPDicePlayerState;

UCLASS()
class DICE_API AMPDiceGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AMPDiceGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	void StartGame();
	void StartTurn();
	void NextTurn(ETurnModeSelect SelectionMode = ETurnModeSelect::Next, int32 SpecificPlayerIdx = -1);

	/* Randomly generate new die faces */
	void RollDice(AMPDicePlayerState* PlayerState);

	void PlayerRollComplete(APlayerController* PlayerController);

	void OnPlayerBet(APlayerController* Bettor, int32 Quantity, int32 FaceVal);
	void OnPlayerChallenge(APlayerController* InChallenger);

	void OnAnimChallengeComplete(APlayerController* InPlayerController);
	void OnAnimCountingComplete(APlayerController* InPlayerController);
	void OnAnimDestroyComplete(APlayerController* InPlayerController);
	
protected:
	int32 TurnIdx;
	AMPDicePlayerState* NextPlayer = nullptr;
	APlayerController* Challenger = nullptr;
	AMPDicePlayerState* Loser = nullptr;

private:
	void DiceToOwner(AMPDicePlayerState* PlayerState) const;
	void EndOfRound();
	void CheckStartTurn();

	void RevealDice();
	void HideDice();
	void DestroyDice();

	int CountCurrentBet() const;
	void NotifyPlayerTurn(int32 PlayerIdx) const;
	void NotifyRoundRestart() const;
	bool IsValidBet(int32 Quantity, int32 Face);
	bool CheckAnimComplete(APlayerController* InPlayerController);

	TArray<APlayerController*> CompletedAnimationPlayers;
};
