// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FaceSelectionWidget.h"
#include "LeaderboardWidget.h"
#include "DiceGameMode.h"
#include "UIEnums.h"
#include "DicePlayer.generated.h"

UCLASS()
class DICE_API ADicePlayer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADicePlayer();

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Callbacks for Player Actions **/
	void SubmitBet();
	void ChallengeBet();

public:	
	/** Current number of die that the player has in play **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dice")
	int32 InitDiceCount = 5; 

	/** List of die faces **/
	TArray<int32> DiceRolls; // TODO: Should use a dictionary that matches the AActor to its value, rather than two arrays
	
	/* Checks whether the dice are settled */
	bool bHasSettled;

	/* Player ID */
	UPROPERTY(VisibleAnywhere, Category = "Player Info")
	int32 PlayerID;

	/* Dissolve step speed */
	UPROPERTY(EditDefaultsOnly, Category = "Destruction")
	float Step = 0.05f;

	/** Player class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Dice")
	TSubclassOf<AActor> DiceClass;

	/** Widget classes **/
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFaceSelectionWidget> FaceSelectionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULeaderboardWidget> LeaderboardWidgetClass;

	/* Sets the player */
	void PlayerSetup(int32 NewPlayerID);

	/** Sets the player as the current player **/
	void StartPlayerTurn();

	/* Spawns the dice with random faces */
	void RollDice();

	/* Destroys a Dice */
	void RemoveDice(EAnimState& InDestructionState);
	void RemoveDiceUI(EAnimState& InDestructionUIState);

	/* Reveals the Leaderboard */
	void RevealLeaderboard(bool bLost);

	/* Triggers the bet counting animation */
	void StartCountingAnim(TArray<int32>& AcceptableBets, EAnimState& InCountingState, UBetWidget* InBetWidget, int32 NumOfFaces);

	// TODO: Multiplayer -> Protected
	/** Callbacks to open or close UI for Bidding **/
	void OnOpenBetUI();
	void OnCloseBetUI();
	void SetupUI();

private:
	TMap<FVector, int32> DiceDirectionsMap;

	/** List of die actors **/
	TArray<AActor*> DiceActors;

	/* Leaderboard */
	TMap<int32, TArray<int32>> Leaderboard;

	/* Game Mode for the game */
	ADiceGameMode* GameMode = nullptr;

	/* Currently playing */
	bool bIsPlaying;

	/* Waiting for the dice to settle? */
	bool bIsWaiting;

	/* Keeps track how long the dice have been stationary for */
	float TimeStationary;

	/* Selected Face Number */
	int32 Face;

	/* Generate a random face side */
	FRotator GenerateDiceRot(int32 FaceVal);

	/* Active Face UI widget */
	UFaceSelectionWidget* ActiveFaceWidget = nullptr;

	/* Active Leaderboard widget */
	ULeaderboardWidget* ActiveLeaderWidget = nullptr;

	/* Deals with Event Dispatcher from Face Selection UI*/
	UFUNCTION()
	void HandleFaceChosen(int32 FaceValue);

	/** Camera **/
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* PlayerCamera;

	/** Light **/
	UPROPERTY(VisibleAnywhere)
	class USpotLightComponent* PlayerLight;

	/* Check if the dice is within bounds */
	bool IsWithinBounds(AActor* Dice) const;

	/* Check the top face */
	void SaveFaces();

	/* Spawn the dice actor */
	void SpawnDice();

	/* Physically Roll a dice actor */
	void RollOneDice(AActor* Dice);

	/* End of the turn for the player */
	void OnTurnEnd();

	////////////////////////
	/** Dice Destruction **/
	////////////////////////

	/* Animates the dice destruction */
	void AnimateDestruction();
	void AnimateUIDestruction();

	/* List of material instances for the die about to be destroyed */
	TArray<UMaterialInstanceDynamic*> Materials;
	
	/* Flag from GameMode indicating if the dice was destroyed */
	EAnimState* DestructionState = nullptr;
	EAnimState* DestructionUIState = nullptr;

	/* Reference to the dice being destroyed */
	AActor* DiceDes = nullptr;
	
	/* Timer Handle for Destruction animation */
	FTimerHandle AnimTimerHandle;
	FTimerHandle AnimUITimerHandle;

	/* Starting Dissolve values for dice destruction */
	float DissolveVal = -1.f;
	float DissolveUIVal = -1.f;
};
