// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiceTextureData.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PlayerRowWidget.generated.h"

class AMPDicePlayerState;

UCLASS()
class DICE_API UPlayerRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // Retrieve all dice icon widgets (for animation)
    TArray<UImage*> GetDiceIconWidgets();

    // Get the current dice values
    const TArray<int32>& GetDisplayedDiceValues() const { return DisplayedDice; }

    void SetPlayerName(const FString PlayerName);
    void UpdateDiceIcons(const TArray<int32>& DiceValues);
    void SetupHandlers(AMPDicePlayerState* PS);

    DECLARE_MULTICAST_DELEGATE_OneParam(FOnRowDiceRevealed, UPlayerRowWidget*);
    FOnRowDiceRevealed OnRowDiceRevealed;

    UHorizontalBox* GetDiceIconBox() const { return DiceIconBox; }

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* DiceIconBox;

    UPROPERTY(meta = (BindWidget))
    UBorder* PlayerBorder;

    UPROPERTY(EditDefaultsOnly, Category = "Data")
    UDiceTextureData* DiceTexturesData;

private:
    TArray<int32> DisplayedDice;
    
    void HandleDiceReveal(const TArray<int32>& Dice);
    void HandleDiceHiding(int32 Num);
};
