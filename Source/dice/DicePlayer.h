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
	int32 DiceCount = 5;

	/** List of die faces **/
	TArray<int32> DiceRolls;

	/* Player ID */
	UPROPERTY(VisibleAnywhere, Category = "Player Info")
	int32 PlayerID;

	/** Player class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Dice")
	TSubclassOf<AActor> DiceClass;

	/** Widget for Face Selection **/
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFaceSelectionWidget> FaceSelectionWidgetClass;

	/** Camera **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* PlayerCamera;

	/** Light **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpotLightComponent* PlayerLight;

	/* Sets the player */
	void PlayerSetup(int32 NewPlayerID);

	/** Sets the player as the current player **/
	void StartPlayerTurn();

	/* Spawns the dice with random faces */
	void RollDice();

private:
	/** List of die actors **/
	TArray<AActor*> DiceActors;

	/* Currently playing */
	bool bIsPlaying;

	/* Selected Face Number */
	int32 Face;

	/* Generate a random face side */
	FRotator GenerateDiceRot(int32 FaceVal);

	/* Active Face UI widget */
	UFaceSelectionWidget* ActiveFaceWidget = nullptr;

	/* Deals with Event Dispatcher from Face Selection UI*/
	UFUNCTION()
	void HandleFaceChosen(int32 FaceValue);

};
