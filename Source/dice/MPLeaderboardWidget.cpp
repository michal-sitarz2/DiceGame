// Fill out your copyright notice in the Description page of Project Settings.


#include "MPLeaderboardWidget.h"
#include "MPDiceGameState.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "MPDicePlayerState.h"


void UMPLeaderboardWidget::NativeDestruct()
{
    Super::NativeDestruct();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(CountTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(PulseTimerHandle);
    }

    CleanupAnimation();
}

void UMPLeaderboardWidget::SetupPlayers()
{
	if (!LeaderboardList) return;

	UE_LOG(LogTemp, Warning, TEXT("Setting up Leaderboard Players"));

	PlayerRowWidgets.Empty();
	LeaderboardList->ClearChildren();

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	AMPDiceGameState* GS = PC->GetWorld()->GetGameState<AMPDiceGameState>();
	if (!GS) return;

	for (APlayerState* PlayerState : GS->PlayerArray)
	{
		if (!PlayerState) continue;

		AMPDicePlayerState* MPPlayerState = Cast<AMPDicePlayerState>(PlayerState);
		if (!MPPlayerState) continue;

		UPlayerRowWidget* Row = CreateWidget<UPlayerRowWidget>(this, PlayerRowWidgetClass);
		if (!Row) continue;

		Row->SetPlayerName(MPPlayerState->GetPlayerName());

		TArray<int32> Zeros;
		Zeros.Init(0, GS->InitDiceNum);
		Row->UpdateDiceIcons(Zeros);

		if (UVerticalBoxSlot* RowSlot = LeaderboardList->AddChildToVerticalBox(Row))
		{
			RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 7.5f));
		}

		PlayerRowWidgets.Add(MPPlayerState->PlayerIdx, Row);

		Row->SetupHandlers(MPPlayerState);

        Row->OnRowDiceRevealed.AddUObject(this, &UMPLeaderboardWidget::RowRevealReady);
	}
}

void UMPLeaderboardWidget::RowRevealReady(UPlayerRowWidget* Row)
{
    RevealedRowSet.Add(Row);

    if (RevealedRowSet.Num() == (PlayerRowWidgets.Num() - 1))
    {
        RevealedRowSet.Empty();
        
        FTimerHandle LayoutTimer;
        GetWorld()->GetTimerManager().SetTimer(
            LayoutTimer,
            [this]() { StartCountingAnimation(); },
            0.1f, // 100ms delay (let the layout settle)
            false
        );
    }
}

void UMPLeaderboardWidget::UpdatePlayerDice(int32 PlayerIdx, const TArray<int32>& DiceVals)
{
	UPlayerRowWidget** RowPtr = PlayerRowWidgets.Find(PlayerIdx);

	if (!RowPtr || !*RowPtr) return;

	UPlayerRowWidget* Row = *RowPtr;
	Row->UpdateDiceIcons(DiceVals);
}

void UMPLeaderboardWidget::PrepCountingAnimation(const TArray<int32>& InAcceptableBets, UBetWidget* InBetWidget, int32 InMinCount)
{
    UE_LOG(LogTemp, Warning, TEXT("Prepare Counting Animation"));

    MinCount = InMinCount;
    BetWidget = InBetWidget;
    AcceptableBets = InAcceptableBets;
}

void UMPLeaderboardWidget::StartDestructionAnimation(int32 InDestroyPlayerIdx)
{
    DestroyPlayerIdx = InDestroyPlayerIdx;

    if (UPlayerRowWidget** RowPtr = PlayerRowWidgets.Find(InDestroyPlayerIdx))
    {
        UPlayerRowWidget* Row = *RowPtr;
        TArray<UWidget*> Images = Row->GetDiceIconBox()->GetAllChildren();
        DestroyImg = Cast<UImage>(Images.Last());
        if (!DestroyImg)
        {
            UE_LOG(LogTemp, Error, TEXT("[UI] Destroy image cast failed"));
            OnDestructionAnimComplete.Broadcast();
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[UI] Starting Destruction Animation"));

    // Start animation timer
    GetWorld()->GetTimerManager().SetTimer(
        DestroyTimerHandle,
        this,
        &UMPLeaderboardWidget::PlayDestructionAnimation,
        0.016f,
        true
    );
}

void UMPLeaderboardWidget::CleanupAnimation()
{
    auto SafeRemove = [](UWidget* Widget)
    {
        if (IsValid(Widget) && Widget->GetParent())
        {
            Widget->RemoveFromParent();
        }

        Widget = nullptr;
    };

    for (FCountingAnimData& AnimData : ActiveAnimIcons)
    {
        SafeRemove(AnimData.FlyImage);
        SafeRemove(AnimData.Glow);
        SafeRemove(AnimData.PulseImage);
    }

    ActiveAnimIcons.Empty();
    CurrentAnimIconIdx = INDEX_NONE;
}

void UMPLeaderboardWidget::PlayDestructionAnimation()
{
    if (!DestroyImg)
    {
        UE_LOG(LogTemp, Error, TEXT("[UI] Destroy Image not found"));
        GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
        OnDestructionAnimComplete.Broadcast();
        return;
    }
    DissolveVal += DissolveStep;

    if (FMath::IsNearlyEqual(DissolveVal, 1.0f, 0.01f))
    {    
        if (UPlayerRowWidget** RowPtr = PlayerRowWidgets.Find(DestroyPlayerIdx))
        {
            UPlayerRowWidget* Row = *RowPtr;
            TArray<int32> CurrentDice = Row->GetDisplayedDiceValues();
            CurrentDice.Pop();
        }

        DissolveVal = -1.f;
        DestroyPlayerIdx = -1;

        GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
        OnDestructionAnimComplete.Broadcast();

        return;
    }

    /* Update dice dissolve */
    FSlateBrush Brush = DestroyImg->GetBrush();
    UObject* ResourceObject = Brush.GetResourceObject();

    if (UMaterialInstanceDynamic* DynamicMat = Cast<UMaterialInstanceDynamic>(ResourceObject))
    {
        DynamicMat->SetScalarParameterValue(FName("Dissolve"), DissolveVal);
    }
}

void UMPLeaderboardWidget::StartCountingAnimation()
{
	if (!Root || !DiceTexturesData)
	{
		UE_LOG(LogTemp, Error, TEXT("Counting animation failed"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Counting Animation Started"));

	SetupCountingAnim();

	if (ActiveAnimIcons.Num() == 0)
	{
		CountingState = EAnimState::Finished;
		OnCountingAnimComplete.Broadcast();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		CountTimerHandle,
		this,
		&UMPLeaderboardWidget::PlayCountingAnim,
		TickDelta,
		true
	);
}

void UMPLeaderboardWidget::SetupCountingAnim()
{
	CountingState = EAnimState::Running;

	// Setup animation target (end position)
	FGeometry Geometry = Root->GetCachedGeometry();
	FVector2D WidgetSize = Geometry.GetLocalSize();
	AnimEnd = FVector2D(WidgetSize.X * 0.5f, 32.0f);

	// Clear previous animation data
	ActiveAnimIcons.Empty();

    // Iterate through all player rows
    for (const TPair<int32, UPlayerRowWidget*>& Pair : PlayerRowWidgets)
    {
        int32 PlayerID = Pair.Key;
        UPlayerRowWidget* Row = Pair.Value;

        if (!Row) continue;

        // Get dice values and widgets from this row
        const TArray<int32>& DiceValues = Row->GetDisplayedDiceValues();
        TArray<UImage*> DiceImages = Row->GetDiceIconWidgets();

        if (DiceValues.Num() != DiceImages.Num())
        {
            UE_LOG(LogTemp, Error, TEXT("[Player %d] Values/Images mismatch (%d vs %d)"),
                PlayerID, DiceValues.Num(), DiceImages.Num());
            continue;
        }

        UE_LOG(LogTemp, Warning, TEXT("[Player %d] Processing %d dice"), PlayerID, DiceValues.Num());

        // Check each die
        for (int32 i = 0; i < DiceValues.Num(); i++)
        {
            int32 DiceNum = DiceValues[i];

            // Check if this die matches the acceptable bet
            if (!AcceptableBets.Contains(DiceNum))
            {
                continue; // Skip non-matching dice
            }

            UImage* IconImage = DiceImages[i];
            if (!IconImage)
            {
                UE_LOG(LogTemp, Error, TEXT("Null image for dice %d"), i);
                continue;
            }

            // Force layout to ensure geometry is valid
            IconImage->ForceLayoutPrepass();

            // Get position in canvas coordinates
            FGeometry IconGeometry = IconImage->GetCachedGeometry();
            FGeometry CanvasGeometry = Root->GetCachedGeometry();

            FVector2D AbsolutePosition = IconGeometry.GetAbsolutePositionAtCoordinates(FVector2D::ZeroVector);
            FVector2D IconPos = CanvasGeometry.AbsoluteToLocal(AbsolutePosition);
            FVector2D Size = IconGeometry.GetLocalSize();


            // Create flying star image
			UImage* StarImage = NewObject<UImage>(this);
			if (StarImage)
            {
                StarImage->SetBrushFromTexture(StarTexture);
            }

            // Generate random bezier control point for arc motion
            FVector2D ControlPoint = RandomBezierControlPoint(IconPos, AnimEnd);

            // Queue this dice for animation
            ActiveAnimIcons.Add(FCountingAnimData(StarImage, IconPos, ControlPoint, DiceNum, IconImage));
        }
    }
}

void UMPLeaderboardWidget::PlayCountingAnim()
{
	if (CountingState != EAnimState::Running)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountTimerHandle);
		OnCountingAnimComplete.Broadcast();
	}

	bool bWait = false;
	bool bAllFinished = true;

    for (FCountingAnimData& AnimData : ActiveAnimIcons)
    {
        if (!AnimData.FlyImage) continue;

        switch (AnimData.State)
        {
        case EAnimState::NotStarted:
        {
            bAllFinished = false;
            if (bWait) break;

            // Start pulse animation
            if (AnimData.Pulse == EAnimState::NotStarted)
            {
                AnimData.Pulse = EAnimState::Running;
                // CurrentAnimIcon = &AnimData;
                CurrentAnimIconIdx = &AnimData - ActiveAnimIcons.GetData();

                GetWorld()->GetTimerManager().SetTimer(
                    PulseTimerHandle,
                    this,
                    &UMPLeaderboardWidget::PulseImage,
                    0.016f,
                    true
                );
            }

            // Wait for pulse to finish
            if (AnimData.Pulse != EAnimState::Finished)
            {
                bWait = true;
                break;
            }

            // Add flying image to canvas
            if (UCanvasPanelSlot* ImageSlot = Root->AddChildToCanvas(AnimData.FlyImage))
            {
                ImageSlot->SetPosition(AnimData.Start);
                ImageSlot->SetSize(FVector2D(30, 30));
            }


            // Add glow effect
            if (WhiteGlowMat)
            {
                UMaterialInstanceDynamic* WhiteGlowMID = UMaterialInstanceDynamic::Create(WhiteGlowMat, this);
                WhiteGlowMID->SetTextureParameterValue(FName("DiceTexture"), DiceTexturesData->DiceTextures[AnimData.DiceNum]);

                AnimData.Glow = NewObject<UImage>(this);
                AnimData.Glow->SetBrushFromMaterial(WhiteGlowMID);
                AnimData.Glow->SetRenderScale(FVector2D(1.75, 1.75));

                if (UCanvasPanelSlot* GlowSlot = Root->AddChildToCanvas(AnimData.Glow))
                {
                    GlowSlot->SetPosition(AnimData.Start);

                    if (AnimData.PulseImage)
                    {
                        FGeometry PulseGeometry = AnimData.PulseImage->GetCachedGeometry();
                        GlowSlot->SetSize(PulseGeometry.GetLocalSize());
                    }
                }
            }

            AnimData.State = EAnimState::Running;
            break;
        }

        case EAnimState::Running:
        {
            bAllFinished = false;

            // Update elapsed time and compute alpha
            AnimData.Elapsed += TickDelta;
            float Alpha = FMath::Clamp(AnimData.Elapsed / Duration, 0.f, 1.f);

            // Easing in/out
            float EaseAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.5f);

            // Finding the next point using Bezier curve motion 
            FVector2D P0 = FMath::Lerp(AnimData.Start, AnimData.ControlPoint, EaseAlpha);
            FVector2D P1 = FMath::Lerp(AnimData.ControlPoint, AnimEnd, EaseAlpha);
            AnimData.Progress = FMath::Lerp(P0, P1, EaseAlpha);

            // Wiggle effect
            AnimData.Progress.X += FMath::Sin(EaseAlpha * PI * 6) * WiggleAmount;

            // Rotate towards motion direction
            FVector2D Direction = (AnimEnd - AnimData.Progress);
            float AngleRadians = FMath::Atan2(Direction.Y, Direction.X);
            float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
            AnimData.FlyImage->SetRenderTransformAngle(AngleDegrees);

            // Update position
            if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(AnimData.FlyImage->Slot))
            {
                CanvasSlot->SetPosition(AnimData.Progress);
            }

            // Check if finished
            if (FMath::IsNearlyEqual(Alpha, 1.f, 0.05f))
            {
                AnimData.State = EAnimState::Finished;
                AnimData.FlyImage->SetVisibility(ESlateVisibility::Hidden);

                // Increment counter in BetWidget
                if (BetWidget)
                {
                    BetWidget->IncCounter(MinCount);
                }
            }

            break;
        }

        case EAnimState::Finished:
        {
            // Do nothing
            break;
        }
        }
    }

    // Check if all animations finished
    if (bAllFinished)
    {
        CountingState = EAnimState::Finished;
        //CurrentAnimIcon = nullptr;
        CurrentAnimIconIdx = INDEX_NONE;
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(PulseTimerHandle);
        }
    }
}

void UMPLeaderboardWidget::PulseImage()
{
    if (!GetWorld() || !IsValid(this))
    {
        return;
    }

    if (!ActiveAnimIcons.IsValidIndex(CurrentAnimIconIdx))
    {
        GetWorld()->GetTimerManager().ClearTimer(PulseTimerHandle);
        return;
    }

    FCountingAnimData& AnimData = ActiveAnimIcons[CurrentAnimIconIdx];

    if (AnimData.Pulse == EAnimState::Finished)
    {
        GetWorld()->GetTimerManager().ClearTimer(PulseTimerHandle);
        CurrentAnimIconIdx = INDEX_NONE;
        return;
    }

    float DeltaTime = GetWorld()->GetDeltaSeconds();

    if (AnimData.bPulseScale)
    {
        float MaxScale = 1.5f;
        AnimData.PulseScale += DeltaTime * PulseSpeed;

        if (AnimData.PulseScale >= MaxScale)
        {
            AnimData.PulseScale = MaxScale;
            AnimData.bPulseScale = false;
        }
    }
    else
    {
        float MinScale = 1.0f;
        AnimData.PulseScale -= DeltaTime * PulseSpeed;

        if (AnimData.PulseScale <= MinScale)
        {
            AnimData.PulseScale = MinScale;
            AnimData.bPulseScale = true;
            AnimData.Pulse = EAnimState::Finished;
        }
    }

    // Apply scale to pulse image
    if (UImage* PulseImg = AnimData.PulseImage)
    {
        if (IsValid(PulseImg))
        {
            float Scale = AnimData.PulseScale;
            PulseImg->SetRenderScale(FVector2D(Scale, Scale));
        }
    }
}

FVector2D UMPLeaderboardWidget::RandomBezierControlPoint(const FVector2D& Start, const FVector2D& End)
{
	// Arc Motion with Bezier Curves
	float T = FMath::RandRange(0.25f, 0.75f);
	FVector2D BasePoint = FMath::Lerp(Start, End, T);

	float Offset = FMath::RandRange(-300.f, 300.f);
	FVector2D Dir = End - Start;
	FVector2D Perp = FVector2D(-Dir.Y, Dir.X).GetSafeNormal();

	return BasePoint + Perp * Offset;
}