// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FaceSelectionWidget.h"
#include "DiceGameMode.h"
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

	/** Callbacks to open or close UI for Bidding **/
	void OnOpenBetUI();
	void OnCloseBetUI();

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

	/** Player class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Dice")
	TSubclassOf<AActor> DiceClass;

	/** Widget for Face Selection **/
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFaceSelectionWidget> FaceSelectionWidgetClass;

	/* Sets the player */
	void PlayerSetup(int32 NewPlayerID);

	/** Sets the player as the current player **/
	void StartPlayerTurn();

	/* Spawns the dice with random faces */
	void RollDice();

	/* Destroys a Dice */
	void RemoveDice();

private:
	TMap<FVector, int32> DiceDirectionsMap;

	/** List of die actors **/
	TArray<AActor*> DiceActors;

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

};
