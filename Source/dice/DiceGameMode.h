// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DiceGameMode.generated.h"

class ADicePlayer;

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

private:	
	/** Current Player **/
	int32 CurrentPlayer;
	
	/** List of Players **/
	TArray<ADicePlayer*> Players;

	/** Toggles the next player in the line **/
	UFUNCTION(BlueprintCallable)
	void ToggleNextPlayer(int32 PlayerIdx = -1, bool Overwrite = false);
};
