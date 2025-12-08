// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UIEnums.h"
#include "BetWidget.h"
#include "DiceTextureData.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "LeaderboardWidget.generated.h"


USTRUCT()
struct FPlayerUIData
{
	GENERATED_BODY()

	UHorizontalBox* IconBox = nullptr;
	UBorder* PlayerBox = nullptr;
	TArray<int32> Dice;

	FPlayerUIData() = default;
	FPlayerUIData(UHorizontalBox* InIconBox, UBorder* InPlayerBox)
		: IconBox(InIconBox)
		, PlayerBox(InPlayerBox)
	{}
};

USTRUCT()
struct FFlyingAnimData
{
	GENERATED_BODY()

	UImage* FlyImage = nullptr;
	FVector2D Start = FVector2D(0, 0);
	FVector2D Progress = FVector2D(0, 0);
	FVector2D ControlPoint = FVector2D(0, 0);
	EAnimState State = EAnimState::NotStarted;
	int32 DiceNum = 0;
	
	float Elapsed = 0.f;

	/* Pulse Animation Data */
	UImage* PulseImage = nullptr; 
	EAnimState Pulse = EAnimState::NotStarted;
	bool bPulseScale = true;
	float PulseScale = 1.0f;

	UImage* Glow = nullptr;

	FFlyingAnimData() = default;
	FFlyingAnimData(UImage* InFlyImage, FVector2D InStart, FVector2D InControlPoint, int32 InDiceNum, UImage* InPulseImage)
		: FlyImage(InFlyImage)
		, Start(InStart)
		, Progress(InStart)
		, ControlPoint(InControlPoint)
		, DiceNum(InDiceNum)
		, PulseImage(InPulseImage)
	{}
};

UCLASS()
class DICE_API ULeaderboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* Material for the Dice Icons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledaerboard")
	UMaterialInterface* IconMat;

	// Setup and update of the leaderboard dynamically
	void SetupLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, int32 PlayerIdx);
	void UpdateLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, int32 PlayerIdx);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leaderboard")
	float BorderOffset = 140.f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float TickDelta = 0.016f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float Speed = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float Duration = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float WiggleAmount = 12.f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float PulseSpeed = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UMaterialInterface* WhiteGlowMat;

	void ClearAnimData();
	void DestroyLoserUIDice(int32 LoserIdx, float DisVal);
	void StartCountingAnimation(TArray<int32>& AcceptableBets, EAnimState& InCountingState, UBetWidget* InBetWidget, int32 InMinCount);


protected:
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* Root;

	UFUNCTION()
	static FVector2D RandomBezierControlPoint(const FVector2D& Start, const FVector2D& End);

private:
	// TODO: 
	const FVector2D LeaderboardStart = FVector2D(76.f, 900.f);

	/* UI Components */ 
	UPROPERTY()
	TMap<int32, FPlayerUIData> PlayerUIData;

	/* Textures data */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UDiceTextureData* DiceTexturesData;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UTexture2D* StarTexture;

	//////////////////////////
    /** Counting Animation **/
	//////////////////////////

	/* Count required for the bet to be successful */
	int32 MinCount;

	/* Pointer to the global Bet UMG */
	UBetWidget* BetWidget;

	/* List of icons currently flying */
	TArray<FFlyingAnimData> ActiveAnimIcons;

	/* Current icon pulsing*/
	FFlyingAnimData* CurrentAnimIcon = nullptr;

	/* List of acceptable bets (i.e., what constitutes to counting) */
	TArray<int32> AcceptableBets;

	/* Timer Handles for Counting Animations */
	FTimerHandle CountingTimerHandle;
	FTimerHandle PulseTimerHandle;

	/* State of the counting animation from GameMode  */
	EAnimState* CountingState = nullptr;

	/* End location (where the counter is) */
	FVector2D AnimEnd;

	/* Animations */
	void PlayCountingAnimation();
	void PulseImage();

};

