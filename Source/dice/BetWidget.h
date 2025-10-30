// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiceTextureData.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "BetWidget.generated.h"

UCLASS()
class DICE_API UBetWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetCurrentBetText(int32 NumFaces, int32 FaceNumber, int32 PlayerIdx);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void ResetCurrentBetText();

	void ChallengeStart();

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* BetNumText;

	UPROPERTY(meta = (BindWidget))
	class UImage* DiceIcon;

	UPROPERTY(meta = (BindWidget))
	class UImage* NoneText;

	UPROPERTY(meta = (BindWidget))
	class UImage* ChallengePopUp;

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UDiceTextureData* DiceTexturesData;

	void ChallengeStop();
};

