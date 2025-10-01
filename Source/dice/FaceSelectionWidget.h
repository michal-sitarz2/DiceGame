// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "FaceSelectionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaceChosen, int32, FaceValue);

UCLASS()
class DICE_API UFaceSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** Event Dispatcher for a face number selected**/
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Events")
	FOnFaceChosen OnFaceChosen;

	/** Number of faces selected **/
	int32 NumOfFaces = 1;

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void DecFacesNum();

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void IncFacesNum();

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* FaceNumTextBlock;

private:
	void SetFaceNumBlockText();
};
