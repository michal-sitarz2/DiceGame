// Fill out your copyright notice in the Description page of Project Settings.


#include "BetWidget.h"

void UBetWidget::SetCurrentBetText(int32 NumFaces, int32 FaceNumber, int32 PlayerIdx)
{
    const FText& BetNum = FText::FromString(FString::Printf(TEXT("%d"), NumFaces));
    if (BetNumText) BetNumText->SetText(BetNum);

    const FText& PlayerTag = FText::FromString(FString::Printf(TEXT("Player %d"), PlayerIdx));
    if (PlayerText) PlayerText->SetText(PlayerTag);

    if (NoneText) NoneText->SetVisibility(ESlateVisibility::Hidden);

    if (DiceIcon && DiceTexturesData)
    {
        DiceIcon->SetBrushFromTexture(DiceTexturesData->DiceTextures[FaceNumber]);
        DiceIcon->SetVisibility(ESlateVisibility::Visible);
    }
}


void UBetWidget::ResetCurrentBetText()
{
    const FText& Empty = FText::FromString(FString::Printf(TEXT("")));
    if (BetNumText) BetNumText->SetText(Empty);
    if (PlayerText) PlayerText->SetText(Empty);

    if (NoneText) NoneText->SetVisibility(ESlateVisibility::Visible);

    if (DiceIcon)
    {
        DiceIcon->SetVisibility(ESlateVisibility::Hidden);
    }
}


void UBetWidget::ChallengeStart()
{
    ChallengePopUp->SetVisibility(ESlateVisibility::Visible);

    FTimerHandle TimerHandle_HideAlert;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_HideAlert,
        this,
        &UBetWidget::ChallengeStop,
        1.5f,
        false
    );

    // TODO: Counting animation
}

void UBetWidget::ChallengeStop()
{
    ChallengePopUp->SetVisibility(ESlateVisibility::Hidden);
}