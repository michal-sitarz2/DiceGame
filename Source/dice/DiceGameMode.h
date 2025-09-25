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
	/** Player class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	TSubclassOf<ADicePlayer> PlayerClass;
};
