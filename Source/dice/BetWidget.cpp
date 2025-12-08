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
    if (BetChallengeText)
    {
        BetChallengeText->SetText(Empty);
        FLinearColor ShadowColor = BetChallengeText->GetShadowColorAndOpacity();
        ShadowColor.A = 0.f;
        BetChallengeText->SetShadowColorAndOpacity(ShadowColor);
    }

    if (NoneText) NoneText->SetVisibility(ESlateVisibility::Visible);

    if (DiceIcon)
    {
        DiceIcon->SetVisibility(ESlateVisibility::Hidden);
    }

    auto* EmptyTEX = DiceTexturesData->DiceTextures[-1];
    RedRays->SetBrushFromTexture(EmptyTEX);
    WhiteRays->SetBrushFromTexture(EmptyTEX);
    RadialPulse->SetBrushFromTexture(EmptyTEX);
}


void UBetWidget::ChallengeStart(EAnimState& InChallengeState)
{
    ChallengeState = &InChallengeState;
    *ChallengeState = EAnimState::Running;

    ChallengePopUp->SetVisibility(ESlateVisibility::Visible);

    BetCounter = 0;

    const FText& TotalNum = FText::FromString(FString::Printf(TEXT("0"))); // TEXT("%d"), TotalDiceCounter));
    if (BetChallengeText) BetChallengeText->SetText(TotalNum);

    FTimerHandle TimerHandle_HideAlert;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_HideAlert,
        this,
        &UBetWidget::ChallengeStop,
        3.f,
        false
    );
}

void UBetWidget::ChallengeStop()
{
    ChallengePopUp->SetVisibility(ESlateVisibility::Hidden);
    if (ChallengeState) *ChallengeState = EAnimState::Finished;
}

void UBetWidget::IncCounter(int32 Desired)
{
    BetCounter++;

    const FText& TotalNum = FText::FromString(FString::Printf(TEXT("%d"), BetCounter));
    if (BetChallengeText) BetChallengeText->SetText(TotalNum);

    UWorld* World = GetWorld();
    if (Desired == BetCounter) 
    {
        if (PulseSpecialAnim) PlayAnimation(PulseSpecialAnim, 0.f, 1);
        if (SpecialSound) UGameplayStatics::PlaySound2D(World, SpecialSound, 1.0f, 1.0f, 0.0f);

        TriggerStreakBurstFX();

        UMaterialInstanceDynamic* RedRaysMID = UMaterialInstanceDynamic::Create(RedRaysMaterial, this);
        RedRays->SetBrushFromMaterial(RedRaysMID);

        UMaterialInstanceDynamic* WhiteRaysMID = UMaterialInstanceDynamic::Create(WhiteRaysMaterial, this);
        WhiteRays->SetBrushFromMaterial(WhiteRaysMID);

        UMaterialInstanceDynamic* PulseMID = UMaterialInstanceDynamic::Create(PulseMaterial, this);
        RadialPulse->SetBrushFromMaterial(PulseMID);
    }
    else
    {
        if (CountingSound) UGameplayStatics::PlaySound2D(World, CountingSound, 1.0f, 1.0f, 0.15f);
    }

    // Pulse the dice icon
    if (DiceIconPulse) PlayAnimation(DiceIconPulse, 0.f, 1);
}