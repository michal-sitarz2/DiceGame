// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "FaceSelectionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaceChosen, int32, FaceValue);

UCLASS()
class DICE_API UFaceSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	/** Event Dispatcher for a face number selected**/
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Events")
	FOnFaceChosen OnFaceChosen;

	// Setups the slider dynamically
	void SetupSlider(float InMin, float InMax, float InInitialValue = 0.0f);

	int NumOfFaces;

protected:
	UPROPERTY(meta = (BindWidget))
	class USlider* PredSlider;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PredText;

private:
	UFUNCTION()
	void OnSliderValueChanged(float Value);

	float MinValue;
	float MaxValue;
};
