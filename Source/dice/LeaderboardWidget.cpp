  // Fill out your copyright notice in the Description page of Project Settings.


#include "LeaderboardWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Materials/MaterialInstanceDynamic.h"


void ULeaderboardWidget::SetupLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, int32 PlayerIdx)
{
    if (!WidgetTree) return;

    PlayerUIData.Empty();

    int Counter = 1;
    for (const TPair<int32, TArray<int32>>& Entry : Leaderboard)
    {
        int32 CurrentPlayer = Entry.Key;
        const TArray<int32>& Dice = Entry.Value;

        // TODO:
        bool bIsCurrentPlayer = CurrentPlayer == PlayerIdx;
        
        UBorder* PlayerBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        PlayerBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.25f));

        UVerticalBox* VerticalContainer = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
        UTextBlock* PlayerNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        
        PlayerNameText->SetText(FText::FromString(FString::Printf(TEXT("Player %d"), CurrentPlayer)));
        FSlateFontInfo FontInfo = PlayerNameText->GetFont();
        FontInfo.Size = 28; // bIsCurrentPlayer ? 32 : 28;
        PlayerNameText->SetFont(FontInfo);
        PlayerNameText->SetMargin(FMargin(10.0f, 5.0f, 0.0f, 5.0f));

        UHorizontalBox* PlayerIcons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

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
                //BorderSlot->SetSize(FVector2D(485.0f, 135.0f));
                BorderSlot->SetSize(FVector2D(370.0f, 120.0f));
                BorderSlot->SetPosition(LeaderboardStart);
            }
            else
            {
                // Border Size and Location
                BorderSlot->SetSize(FVector2D(370.0f, 120.0f));
                BorderSlot->SetPosition(FVector2D(LeaderboardStart.X, LeaderboardStart.Y - (BorderOffset * Counter)));
            }
        }

        PlayerUIData.Add(CurrentPlayer, FPlayerUIData(PlayerIcons, PlayerBorder));
        
        if (!bIsCurrentPlayer) Counter++;
    }

    UpdateLeaderboard(Leaderboard, PlayerIdx);
}

void ULeaderboardWidget::UpdateLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, int32 PlayerIdx)
{
    if (!WidgetTree || !DiceTexturesData) return;

    for (TPair<int32, TArray<int32>>& Entry : Leaderboard)
    {
        int32 CurrentPlayer = Entry.Key;
        TArray<int32>& PlayerDice = Entry.Value;

        // Get the corresponding player's Icon Box
        UHorizontalBox* CurrentPlayerIcons = PlayerUIData[CurrentPlayer].IconBox;
        
        // Clear previous icons
        CurrentPlayerIcons->ClearChildren();

        PlayerUIData[CurrentPlayer].Dice.Empty();

        /* Set the Current Player Die Images in the UI */
        int i = 0;
        for (const int32 Dice : PlayerDice)
        {
        UImage* ImageWidget = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
            
        UTexture2D* DiceTex = DiceTexturesData->DiceTextures[Dice];
        if (!IconMat)
        {
            ImageWidget->SetBrushFromTexture(DiceTex);
        }
        else
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(IconMat, this);
            ImageWidget->SetBrushFromMaterial(DynamicMaterial);
            DynamicMaterial->SetTextureParameterValue(FName("DiceTexture"), DiceTex);
        }

        if (UHorizontalBoxSlot* ImageSlot = CurrentPlayerIcons->AddChildToHorizontalBox(ImageWidget))
        {
            
            if (i == 4) // Last needs extra padding on the right (assumes 5 dice in total)
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
            
        PlayerUIData[CurrentPlayer].Dice.Add(Dice);
        }
    }
}

void ULeaderboardWidget::ClearAnimData()
{
    for (FFlyingAnimData& AnimData : ActiveAnimIcons)
    {
        AnimData.Glow->RemoveFromParent();
        AnimData.Glow->ConditionalBeginDestroy();

        AnimData.Glow = nullptr;
    }

    ActiveAnimIcons.Empty();
}

void ULeaderboardWidget::DestroyLoserUIDice(int32 LoserIdx, float DisVal)
{
    TArray<UWidget*> Images = PlayerUIData[LoserIdx].IconBox->GetAllChildren();

    // Last image that is not empty (i.e., -1)
    int32 LastIdx = -1;
    for (const uint32 DiceNum : PlayerUIData[LoserIdx].Dice)
    {
        if (DiceNum == -1) break;
        LastIdx++;
    }

    if (LastIdx == -1) return;

    if (UImage* Image = Cast<UImage>(Images[LastIdx]))
    {
        // Get the brush from the image
        FSlateBrush Brush = Image->GetBrush();
    
        // Get the material resource
        UObject* ResourceObject = Brush.GetResourceObject();

        // Cast to material instance dynamic
        if (UMaterialInstanceDynamic* DynamicMat = Cast<UMaterialInstanceDynamic>(ResourceObject))
        {
            DynamicMat->SetScalarParameterValue(FName("Dissolve"), DisVal);
        }
    }
}

void ULeaderboardWidget::StartCountingAnimation(TArray<int32>& InAcceptableBets, EAnimState& InCountingState, UBetWidget* InBetWidget, int32 InMinCount)
{
    if (!WidgetTree || !DiceTexturesData) return;

    // End location
    FGeometry Geometry = Root->GetCachedGeometry();
    FVector2D WidgetSize = Geometry.GetLocalSize();
    AnimEnd = FVector2D(WidgetSize.X * 0.5f, 32.0);

    MinCount = InMinCount;
    BetWidget = InBetWidget;
    AcceptableBets = InAcceptableBets;

    CountingState = &InCountingState;
    *CountingState = EAnimState::Running;

    // Animation Setup
    ActiveAnimIcons.Empty();
    for (const TPair<int32, FPlayerUIData>& Pair : PlayerUIData)
    {
        const int32& PlayerID = Pair.Key;
        const FPlayerUIData& Data = Pair.Value;
        const TArray<int32>& Die = Data.Dice;

        TArray<UWidget*> Images = Data.IconBox->GetAllChildren();

        for (int i = 0; i < Die.Num(); i++)
        {
            const int32 DiceNum = Die[i];
            // Acceptable for the current bet
            if (AcceptableBets.Contains(DiceNum))
            {

                // Get the position of the image on the leaderboard
                UImage* IconImage = Cast<UImage>(Images[i]);
               
                if (!IconImage) continue;

                // Force Layout to Ensure Geometry is Valid
                IconImage->ForceLayoutPrepass();

                FGeometry IconGeometry = IconImage->GetCachedGeometry();
                FGeometry CanvasGeometry = Root->GetCachedGeometry();

                // Get absolute position and convert to Canvas-local coordinates
                FVector2D AbsolutePosition = IconGeometry.GetAbsolutePositionAtCoordinates(FVector2D::ZeroVector);
                FVector2D IconPos = CanvasGeometry.AbsoluteToLocal(AbsolutePosition);
                FVector2D Size = IconGeometry.GetLocalSize();

                // Create the new image with the corresponding texture
                UImage* StarImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
                StarImage->SetBrushFromTexture(StarTexture);
                
                // Queue it for animation (now using Canvas-local position)
                FVector2D ControlPoint = ULeaderboardWidget::RandomBezierControlPoint(IconPos, AnimEnd);
                ActiveAnimIcons.Add(FFlyingAnimData(StarImage, IconPos, ControlPoint, DiceNum, IconImage));
            }
        }
    }

    // Start Icon Flying Animation
    GetWorld()->GetTimerManager().SetTimer(
        CountingTimerHandle,
        this,
        &ULeaderboardWidget::PlayCountingAnimation,
        TickDelta,
        true
    );
}

void ULeaderboardWidget::PulseImage()
{
    if (!CurrentAnimIcon || CurrentAnimIcon->Pulse == EAnimState::Finished)
    {
        GetWorld()->GetTimerManager().ClearTimer(PulseTimerHandle);
        return;
    }

    float DeltaTime = GetWorld()->GetDeltaSeconds();

    if (CurrentAnimIcon->bPulseScale)
    {
        float MaxScale = 1.5f;

        // Increase the scale towards MaxScale
        CurrentAnimIcon->PulseScale += DeltaTime * PulseSpeed;
        if (CurrentAnimIcon->PulseScale >= MaxScale)
        {
            CurrentAnimIcon->PulseScale = MaxScale;
            CurrentAnimIcon->bPulseScale = false;
        }
    }
    else
    {
        float MinScale = 1.0f;
        // Decrease the scale towards MinScale
        CurrentAnimIcon->PulseScale -= DeltaTime * PulseSpeed;
        if (CurrentAnimIcon->PulseScale <= MinScale)
        {
            CurrentAnimIcon->PulseScale = MinScale;
            CurrentAnimIcon->bPulseScale = true;
            CurrentAnimIcon->Pulse = EAnimState::Finished;
        }
    }

    // TODO: Change size of the image
    if (UImage* PulseImg = CurrentAnimIcon->PulseImage)
    {
        float Scale = CurrentAnimIcon->PulseScale;
        PulseImg->SetRenderScale(FVector2D(Scale, Scale));
    }
}

void ULeaderboardWidget::PlayCountingAnimation()
{
    // When we the animation stops, we stop the timer
    if (*CountingState != EAnimState::Running)
    {
        GetWorld()->GetTimerManager().ClearTimer(CountingTimerHandle);
        return;
    }

    bool bWait = false;
    bool bAllFinished = true;
    for (FFlyingAnimData& AnimData : ActiveAnimIcons)
    {
        if (!AnimData.FlyImage) continue;

        switch (AnimData.State)
        {
        case EAnimState::NotStarted:
        {
            bAllFinished = false;
            if (bWait) break;

            if (AnimData.Pulse == EAnimState::NotStarted)
            {
                AnimData.Pulse = EAnimState::Running;
                CurrentAnimIcon = &AnimData;

                GetWorld()->GetTimerManager().SetTimer(
                    PulseTimerHandle,
                    this,
                    &ULeaderboardWidget::PulseImage,
                    GetWorld()->GetDeltaSeconds(),
                    true
                );
            }

            if (AnimData.Pulse != EAnimState::Finished)
            {
                bWait = true;
                break;
            }
            
            /* Set image position and size on the screen */
            if (UCanvasPanelSlot* ImageSlot = Root->AddChildToCanvas(AnimData.FlyImage))
            {
                ImageSlot->SetPosition(AnimData.Start);
                ImageSlot->SetSize(FVector2D(30, 30));
            }

            /* Adds Glow with the Dice Texture */
            UMaterialInstanceDynamic* WhiteGlowMID = UMaterialInstanceDynamic::Create(WhiteGlowMat, this);
            WhiteGlowMID->SetTextureParameterValue(FName("DiceTexture"), DiceTexturesData->DiceTextures[AnimData.DiceNum]);

            AnimData.Glow = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
            AnimData.Glow->SetBrushFromMaterial(WhiteGlowMID);
            AnimData.Glow->SetRenderScale(FVector2D(1.75, 1.75));
            if (UCanvasPanelSlot* GlowSlot = Root->AddChildToCanvas(AnimData.Glow))
            {
                // Location
                GlowSlot->SetPosition(AnimData.Start);
                
                // Size
                auto Geometry = AnimData.PulseImage->GetCachedGeometry();
                GlowSlot->SetSize(Geometry.GetLocalSize());
            }

            AnimData.State = EAnimState::Running;
            break;
        }

        case EAnimState::Running: // TODO: Trajectory and Particles
        {
            bAllFinished = false;

            // Update elapsed time and compute alpha
            AnimData.Elapsed += TickDelta;
            float Alpha = FMath::Clamp(AnimData.Elapsed / Duration, 0.f, 1.f);

            // Update Position with Linear Interpolation (including Easing In and Out)
            float EaseAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.5f);

            // Find the next point
            FVector2D P0 = FMath::Lerp(AnimData.Start, AnimData.ControlPoint, EaseAlpha);
            FVector2D P1 = FMath::Lerp(AnimData.ControlPoint, AnimEnd, EaseAlpha);
            AnimData.Progress = FMath::Lerp(P0, P1, EaseAlpha);

            // Magical Effect with Wiggle
            AnimData.Progress.X += FMath::Sin(EaseAlpha * PI * 6) * WiggleAmount;

            // Rotate towards Motion
            FVector2D Direction = (AnimEnd - AnimData.Progress);
            float AngleRadians = FMath::Atan2(Direction.Y, Direction.X);
            float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

            AnimData.FlyImage->SetRenderTransformAngle(AngleDegrees);


            // Update the Icon Position
            if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(AnimData.FlyImage->Slot))
            {
                CanvasSlot->SetPosition(AnimData.Progress);
            }

            // Test if we are nearly at the Goal
            if (FMath::IsNearlyEqual(Alpha, 1.f, 0.05))
            {
                AnimData.State = EAnimState::Finished;

                AnimData.FlyImage->SetVisibility(ESlateVisibility::Hidden);
                BetWidget->IncCounter(MinCount);
            }

            break;
        }
        case EAnimState::Finished:
        {
            break;
        }
        }
    }

    if (bAllFinished) *CountingState = EAnimState::Finished;
}

FVector2D ULeaderboardWidget::RandomBezierControlPoint(const FVector2D& Start, const FVector2D& End)
{
    // Arc Motion with Bezier Curves
    float T = FMath::RandRange(0.25f, 0.75f);
    FVector2D BasePoint = FMath::Lerp(Start, End, T);

    float Offset = FMath::RandRange(-300.f, 300.f);
    FVector2D Dir = End - Start;
    FVector2D Perp = FVector2D(-Dir.Y, Dir.X).GetSafeNormal();

    return BasePoint + Perp * Offset;
}