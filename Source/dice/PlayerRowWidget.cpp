
#include "PlayerRowWidget.h"
#include "MPDicePlayerState.h"
#include "Styling/SlateBrush.h"
#include "Components/HorizontalBoxSlot.h"

void UPlayerRowWidget::SetupHandlers(AMPDicePlayerState* PS)
{
    PS->OnDiceRevealed.AddUObject(
        this,
        &UPlayerRowWidget::HandleDiceReveal
    );

    PS->OnDiceHidden.AddUObject(
        this,
        &UPlayerRowWidget::HandleDiceHiding
    );
}

void UPlayerRowWidget::HandleDiceReveal(const TArray<int32>& Dice)
{
    UE_LOG(LogTemp, Warning, TEXT("[UI] Handle Dice Reveal"));
    UpdateDiceIcons(Dice);

    OnRowDiceRevealed.Broadcast(this);
}

void UPlayerRowWidget::HandleDiceHiding(int32 Num)
{
    UE_LOG(LogTemp, Warning, TEXT("[UI] Handle Dice Hiding"));
    TArray<int32> Zeros;
    Zeros.Init(0, Num);

    UpdateDiceIcons(Zeros);
    DisplayedDice.Empty();
}

TArray<UImage*> UPlayerRowWidget::GetDiceIconWidgets()
{
    TArray<UImage*> DiceImgs;

    if (!DiceIconBox) return DiceImgs;

    TArray<UWidget*> Children = DiceIconBox->GetAllChildren();

    for (UWidget* Child : Children)
    {
        if (UImage* ImageWidget = Cast<UImage>(Child))
        {
            DiceImgs.Add(ImageWidget);
        }
    }

    return DiceImgs;
}

void UPlayerRowWidget::SetPlayerName(const FString PlayerName)
{
	if (!PlayerNameText) return;

	PlayerNameText->SetText(FText::FromString(PlayerName));
}

void UPlayerRowWidget::UpdateDiceIcons(const TArray<int32>& DiceValues)
{
    if (!DiceIconBox || !DiceIconBox) return;

    FString JoinedString = FString::JoinBy(DiceValues, TEXT(", "), [](int32 Element) {
        return FString::Printf(TEXT("%d"), Element);
        });
    UE_LOG(LogTemp, Warning, TEXT("Updating Dice Icons: %s"), *JoinedString);

    DiceIconBox->ClearChildren();

    DisplayedDice = DiceValues;
    DisplayedDice.Sort();

    for (int32 DieValue : DisplayedDice)
    {
        // Create icon
        UImage* DiceImage = NewObject<UImage>(this);
        if (!DiceImage) continue;

        
        UTexture2D* DiceTexture = DiceTexturesData->DiceTextures[DieValue];

        FSlateBrush Brush;
        if (IconMat)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(IconMat, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetTextureParameterValue(FName("DiceTexture"), DiceTexture);
                Brush.SetResourceObject(DynamicMaterial);
            }
        }
        else
        {
            Brush.SetResourceObject(DiceTexture);
        }

        Brush.ImageSize = FVector2D(60.f, 60.f);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        Brush.Tiling = ESlateBrushTileType::NoTile;
        DiceImage->SetBrush(Brush);


        // Add to horizontal box
        if (UHorizontalBoxSlot* HSlot = DiceIconBox->AddChildToHorizontalBox(DiceImage))
        {
            HSlot->SetPadding(FMargin(5.f, 15.f, 0.f, 15.f));
            HSlot->SetSize(ESlateSizeRule::Automatic);
        }
    }
}