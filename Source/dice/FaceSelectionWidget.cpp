// Fill out your copyright notice in the Description page of Project Settings.


#include "FaceSelectionWidget.h"

void UFaceSelectionWidget::SetFaceNumBlockText()
{
    const FText& Text = FText::FromString(FString::Printf(TEXT("%d"), NumOfFaces));
    if (FaceNumTextBlock)
    {
        FaceNumTextBlock->SetText(Text);
    }
}

void UFaceSelectionWidget::DecFacesNum()
{
    NumOfFaces = FMath::Max(1, NumOfFaces - 1);
    SetFaceNumBlockText();
}

void UFaceSelectionWidget::IncFacesNum()
{
    NumOfFaces++;
    SetFaceNumBlockText();
}