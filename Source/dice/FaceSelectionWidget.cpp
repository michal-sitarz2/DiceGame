// Fill out your copyright notice in the Description page of Project Settings.

#include "FaceSelectionWidget.h"
#include "Blueprint/WidgetTree.h"

void UFaceSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (PredSlider)
    {
        // Bind the callback for when the slider changes
        PredSlider->OnValueChanged.AddDynamic(this, &UFaceSelectionWidget::OnSliderValueChanged);
    }
}

void UFaceSelectionWidget::SetupSlider(float InMin, float InMax, float InInitialValue)
{
    MinValue = InMin;
    MaxValue = InMax;

    if (PredSlider)
    {
        PredSlider->SetMinValue(MinValue);
        PredSlider->SetMaxValue(MaxValue);
        PredSlider->SetValue(InInitialValue);
    }

    OnSliderValueChanged(InInitialValue);
}

void UFaceSelectionWidget::OnSliderValueChanged(float Value)
{
    if (PredText)
    {
        FString ValueString = FString::Printf(TEXT("%d"), (int) Value);
        PredText->SetText(FText::FromString(ValueString));
    }

    NumOfFaces = (int) Value;
}