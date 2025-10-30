  // Fill out your copyright notice in the Description page of Project Settings.


#include "LeaderboardWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"


void ULeaderboardWidget::SetupLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, int32 PlayerIdx)
{
    if (!WidgetTree) return;

    IconBoxes.Empty();

    int Counter = 1;
    for (const TPair<int32, TArray<int32>>& Entry : Leaderboard)
    {
        int32 CurrentPlayer = Entry.Key;
        const TArray<int32>& Dice = Entry.Value;

        bool bIsCurrentPlayer = CurrentPlayer == PlayerIdx;

        UBorder* PlayerBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        PlayerBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.25f));

        UVerticalBox* VerticalContainer = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
        UTextBlock* PlayerNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        
        PlayerNameText->SetText(FText::FromString(FString::Printf(TEXT("Player %d"), CurrentPlayer)));
        FSlateFontInfo FontInfo = PlayerNameText->GetFont();
        FontInfo.Size = bIsCurrentPlayer ? 32 : 28;
        PlayerNameText->SetFont(FontInfo);
        PlayerNameText->SetMargin(FMargin(10.0f, 5.0f, 0.0f, 5.0f));

        UHorizontalBox* PlayerIcons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
        IconBoxes.Add(CurrentPlayer, PlayerIcons);

        PlayerBorder->SetContent(VerticalContainer);
        VerticalContainer->AddChildToVerticalBox(PlayerNameText);

        if (UVerticalBoxSlot* BoxSlot = VerticalContainer->AddChildToVerticalBox(PlayerIcons))
        {
            FSlateChildSize Size(ESlateSizeRule::Fill);
            BoxSlot->SetSize(Size);
        }

        if (UCanvasPanelSlot* BorderSlot = Root->AddChildToCanvas(PlayerBorder))
        {
            if (bIsCurrentPlayer)
            {
                // Border Size and Location
                BorderSlot->SetSize(FVector2D(480.0f, 135.0f));
                BorderSlot->SetPosition(FVector2D(76.f, 900.f));
            }
            else
            {
                // Border Size and Location
                BorderSlot->SetSize(FVector2D(365.0f, 110.0f));
                BorderSlot->SetPosition(FVector2D(76.f, 900.f - (BorderOffset * Counter)));
            }
        }

        if (!bIsCurrentPlayer) Counter++;

    }

    UpdateLeaderboard(Leaderboard, PlayerIdx);
}

void ULeaderboardWidget::UpdateLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, int32 PlayerIdx)
{
    if (!WidgetTree || !DiceTexturesData) return;

    for (const TPair<int32, TArray<int32>>& Entry : Leaderboard)
    {
        int32 CurrentPlayer = Entry.Key;
        const TArray<int32>& PlayerDice = Entry.Value;

        // Get the corresponding player's Icon Box
        UHorizontalBox* CurrentPlayerIcons = IconBoxes[CurrentPlayer];
        
        // Clear previous icons
        CurrentPlayerIcons->ClearChildren();

        /* Set the Current Player Die Images in the UI */
        int i = 0;
        for (const int32 Dice : PlayerDice)
        {
            UImage* ImageWidget = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
            ImageWidget->SetBrushFromTexture(DiceTexturesData->DiceTextures[Dice]);
            if (UHorizontalBoxSlot* ImageSlot = CurrentPlayerIcons->AddChildToHorizontalBox(ImageWidget))
            {
                // TODO: Assumes 5 dice
                if (i == 4) // Last needs extra padding on the right
                {
                    ImageSlot->SetPadding(FMargin(10.f, 3.f));
                }
                else // Padding for all the other components
                {
                    ImageSlot->SetPadding(FMargin(10.f, 3.f, 0.f, 3.f));
                }
                ImageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            }   
            i++;
        }
    }
}