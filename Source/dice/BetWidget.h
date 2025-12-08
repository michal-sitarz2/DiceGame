// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiceTextureData.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UIEnums.h"
#include "BetWidget.generated.h"


UCLASS()
class DICE_API UBetWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetCurrentBetText(int32 NumFaces, int32 FaceNumber, int32 PlayerIdx);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void ResetCurrentBetText();

	void ChallengeStart(EAnimState& InChallengeState); // , int32 TotalDiceCounter);

	void IncCounter(int32 Desired);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void TriggerStreakBurstFX();

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* BetNumText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* BetChallengeText;

	UPROPERTY(meta = (BindWidget))
	class UImage* DiceIcon;

	UPROPERTY(meta = (BindWidget))
	class UImage* NoneText;

	UPROPERTY(meta = (BindWidget))
	class UImage* ChallengePopUp;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* PulseAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* PulseSpecialAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* DiceIconPulse;

	UPROPERTY(meta = (BindWidget))
	class UImage* RedRays;

	UPROPERTY(meta = (BindWidget))
	class UImage* WhiteRays;

	UPROPERTY(meta = (BindWidget))
	class UImage* RadialPulse;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UMaterialInterface* RedRaysMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UMaterialInterface* WhiteRaysMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UMaterialInterface* PulseMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	USoundBase* CountingSound;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	USoundBase* SpecialSound;

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UDiceTextureData* DiceTexturesData;


	// Callback to Remove Challenge notification
	void ChallengeStop();

	// Flag used in GameMode tick
	EAnimState* ChallengeState = nullptr;

	int BetCounter;
};

