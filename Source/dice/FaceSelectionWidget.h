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

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ResetButtonClicks();

	// Setups the slider dynamically
	void SetupSlider(float InMin, float InMax);
	void ResetSlider();

	int32 NumOfFaces;
	int32 FaceSelected;

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
