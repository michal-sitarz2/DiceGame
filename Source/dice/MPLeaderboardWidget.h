// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"
#include "DiceTextureData.h"
#include "PlayerRowWidget.h"
#include "UIEnums.h"
#include "MPLeaderboardWidget.generated.h"

USTRUCT()
struct FCountingAnimData
{
    GENERATED_BODY()

    UImage* FlyImage = nullptr;
    UImage* Glow = nullptr;
    UImage* PulseImage = nullptr;

    FVector2D Start = FVector2D::ZeroVector;
    FVector2D ControlPoint = FVector2D::ZeroVector;
    FVector2D Progress = FVector2D::ZeroVector;

    int32 DiceNum = 0;

    EAnimState State = EAnimState::NotStarted;
    EAnimState Pulse = EAnimState::NotStarted;

    float Elapsed = 0.f;
    float PulseScale = 1.0f;
    bool bPulseScale = true;

    FCountingAnimData() = default;

    FCountingAnimData(UImage* InFlyImage, FVector2D InStart, FVector2D InControlPoint, int32 InDiceNum, UImage* InPulseImage)
        : FlyImage(InFlyImage)
        , PulseImage(InPulseImage)
        , Start(InStart)
        , ControlPoint(InControlPoint)
        , DiceNum(InDiceNum)
    { }
};


UCLASS()
class DICE_API UMPLeaderboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupPlayers();
	void UpdatePlayerDice(int32 PlayerIdx, const TArray<int32>& DiceVals);
	
    void PrepCountingAnimation(const TArray<int32>& InAcceptableBets, UBetWidget* InBetWidget, int32 InMinCount);
    void StartDestructionAnimation(int32 InDestroyPlayerIdx);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountingAnimComplete);
	UPROPERTY(BlueprintAssignable, Category = "Animation")
	FOnCountingAnimComplete OnCountingAnimComplete;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestructionAnimComplete);
    UPROPERTY(BlueprintAssignable, Category = "Animation")
    FOnDestructionAnimComplete OnDestructionAnimComplete;

protected:
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* LeaderboardList;

    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* Root;
    
    UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPlayerRowWidget> PlayerRowWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UTexture2D* StarTexture;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UMaterialInterface* WhiteGlowMat;

    UPROPERTY(EditDefaultsOnly, Category = "Data")
    UDiceTextureData* DiceTexturesData;

private:

    UFUNCTION()
    void RowRevealReady(UPlayerRowWidget* Row);
    TArray<UPlayerRowWidget*> RevealedRowSet;

    //////////////////////////
    /** Counting Animation **/
    //////////////////////////

	UPROPERTY()
	TMap<int32, UPlayerRowWidget*> PlayerRowWidgets;

    TArray<FCountingAnimData> ActiveAnimIcons;
    FCountingAnimData* CurrentAnimIcon = nullptr;

    UPROPERTY()
    UBetWidget* BetWidget = nullptr;

    TArray<int32> AcceptableBets;
    int32 MinCount = 0;
    FVector2D AnimEnd = FVector2D::ZeroVector;

	FTimerHandle CountTimerHandle;
	FTimerHandle PulseTimerHandle;

    EAnimState CountingState = EAnimState::NotStarted;

    /* Animation Parameters */
    float Duration = 2.0f;
    float TickDelta = 0.016f;
    float WiggleAmount = 10.f;
    float PulseSpeed = 2.5f;
    
    /* Animation Functions */
    
    void SetupCountingAnim();
    void StartCountingAnimation();
    void PlayCountingAnim();
    
    void PulseImage();
    FVector2D RandomBezierControlPoint(const FVector2D& Start, const FVector2D& End);

    ////////////////////////
    /** Dice Destruction **/
    ////////////////////////

    void PlayDestructionAnimation();

    UImage* DestroyImg = nullptr;
    int32 DestroyPlayerIdx = -1;
    FTimerHandle DestroyTimerHandle;
    float DissolveVal = -1.f;
    float DissolveStep = 0.05f;
};
