// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BetWidget.h"
#include "DiceGameMode.generated.h"

class ADicePlayer;

USTRUCT(BlueprintType)
struct FBet
{
	GENERATED_BODY()

public:
	// Number of faces on the die
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bet")
	int32 NumFaces;

	// The face number chosen for the bet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bet")
	int32 Face;

	// The player that submitted the bet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bet")
	int32 PlayerIdx;

	FBet()
		: NumFaces(0)
		, Face(0)
		, PlayerIdx(-1)
	{}

	FBet(int32 NumFaces, int32 Face, int32 PlayerIdx)
		: NumFaces(NumFaces)
		, Face(Face)
		, PlayerIdx(PlayerIdx)
	{}
};


UCLASS()
class DICE_API ADiceGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ADiceGameMode();

protected:
	virtual void BeginPlay() override;

public:
	/** Number of Players **/
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	int32 InitPlayerNum = 0;

	/** Player class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	TSubclassOf<ADicePlayer> PlayerClass;

	/** Widget for Face Selection **/
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UBetWidget> BetWidgetClass;

	/* Submit the bet from the player */
	void SubmitBet(FBet& CurrentBet);

	/* Verify if the bet is correct */
	bool VerifyBet(FBet& NewBet);

	/* Submit a challenge from a player */
	void SubmitChallenge(int32 PlayerIdx);

private:	
	/** Current Player **/
	int32 CurrentPlayer;

	/** Current Bet **/
	FBet* CurrentBet = nullptr;
	
	/** List of Players **/
	TArray<ADicePlayer*> Players;

	/* Active Face UI widget */
	UBetWidget* BetWidget = nullptr;

	/** Toggles the next player in the line **/
	UFUNCTION(BlueprintCallable)
	void ToggleNextPlayer(int32 PlayerIdx = -1, bool Overwrite = false);
};
