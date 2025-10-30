// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiceTextureData.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "LeaderboardWidget.generated.h"


UCLASS()
class DICE_API ULeaderboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup and update of the leaderboard dynamically
	void SetupLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, int32 PlayerIdx);
	void UpdateLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, int32 PlayerIdx);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leaderboard")
	float BorderOffset = 140.f;

protected:
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* Root;

private:
	/* UI Components */
	UPROPERTY()
	TMap<int32, UHorizontalBox*> IconBoxes;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UDiceTextureData* DiceTexturesData;
};

