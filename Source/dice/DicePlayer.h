// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

public:	
	/** Current number of die that the player has in play **/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dice")
	int32 DiceCount;

	/** List of die **/
	TArray<int32> DiceRolls;

	/** Player class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Dice")
	TSubclassOf<AActor> DiceClass;

	/** Camera **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* PlayerCamera;

	/** Light **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpotLightComponent* PlayerLight;

	/* Sets the player */
	void PlayerSetup(int32 NewPlayerID);

private:
	/* Player ID */
	UPROPERTY(VisibleAnywhere, Category = "Player Info")
	int32 PlayerID;

	/* Spawns the dice with random faces */
	void RollDice();

	/* Generate a random face side*/
	FRotator GenerateDiceRot(int32 FaceVal);
};
