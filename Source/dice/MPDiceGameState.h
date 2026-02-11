// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TurnPhase.h"
#include "MPDiceGameState.generated.h"

USTRUCT(BlueprintType)
struct FCurrentBet
{
    GENERATED_BODY()

    UPROPERTY()
    int32 Quantity = 0;

    UPROPERTY()
    int32 Face = 0;

    UPROPERTY()
    TObjectPtr<APlayerState> BettingPlayer = nullptr;

    FCurrentBet() = default;

    FCurrentBet(int32 InQuantity, int32 InFace, TObjectPtr<APlayerState> InBettingPlayer)
        : Quantity(InQuantity)
        , Face(InFace)
        , BettingPlayer(InBettingPlayer) { }

    void Reset() 
    {
        Quantity = 0;
        Face = 0;
        BettingPlayer = nullptr;
    }

    bool IsValid() const { return Quantity > 0 && Face > 0 && BettingPlayer; }
};

class AMPDicePlayerState;

UCLASS()
class DICE_API AMPDiceGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
    AMPDiceGameState();

    UPROPERTY(Replicated)
    ETurnPhase Phase; // Current phase of the game

    UPROPERTY(Replicated)
    AMPDicePlayerState* ActivePlayer; // Player currently playing

    UPROPERTY(ReplicatedUsing = OnRep_CurrentBet)
    FCurrentBet CurrentBet;

    DECLARE_MULTICAST_DELEGATE(FOnGameStarted);
    FOnGameStarted OnGameStarted;

    UPROPERTY(ReplicatedUsing = OnRep_GameStarted)
    bool bGameStarted = false;

    DECLARE_MULTICAST_DELEGATE_OneParam(FOnBetChanged, const FCurrentBet&);
    FOnBetChanged OnBetChanged;

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    int32 InitDiceNum = 5; // Default number of starting dice

    int32 GetTotalDice();

    void BroadcastBetChanged();
    void BroadcastGameStarted();

protected:
    UFUNCTION()
    void OnRep_CurrentBet() { OnBetChanged.Broadcast(CurrentBet); }

    UFUNCTION()
    void OnRep_GameStarted() { OnGameStarted.Broadcast(); }

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

};
