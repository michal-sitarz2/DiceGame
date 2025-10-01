// Fill out your copyright notice in the Description page of Project Settings.


#include "BetWidget.h"

void UBetWidget::SetCurrentBetText(int32 NumFaces, int32 FaceNumber, int32 PlayerIdx)
{
    const FText& Text = FText::FromString(FString::Printf(TEXT("Current Bet: %d of die %d [Player %d]"), NumFaces, FaceNumber, PlayerIdx));
    if (BetText) BetText->SetText(Text);
}

void UBetWidget::ResetCurrentBetText()
{
    const FText& Text = FText::FromString(FString::Printf(TEXT("Current Bet: None")));
    if (BetText) BetText->SetText(Text);
}