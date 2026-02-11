
#pragma once

#include "CoreMinimal.h"
#include "BetWidget.h"
#include "FaceSelectionWidget.h"
#include "MPLeaderboardWidget.h"
#include "GameFramework/PlayerController.h"
#include "MPDicePlayerController.generated.h"


UCLASS()
class DICE_API AMPDicePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_RequestDiceRoll();

	UFUNCTION(Server, Reliable)
	void Server_RequestChallenge();

	UFUNCTION(Server, Reliable)
	void Server_RequestBetSubmission(int32 NumOfFaces, int32 FaceSelected);

	UFUNCTION(Server, Reliable)
	void Server_NotifyRollComplete();

	UFUNCTION(Server, Reliable)
	void Server_NotifyAnimChallengeComplete();

	UFUNCTION(Client, Reliable)
	void Client_ReceiveOwnDice(const TArray<int32>& DiceValues);

	UFUNCTION(Client, Reliable)
	void Client_NotifyTurnStart();

	UFUNCTION(Client, Reliable)
	void Client_NotifyTurnEnd();

	UFUNCTION(Client, Reliable)
	void Client_StartAnimChallenge();

	UFUNCTION(Client, Reliable)
	void Client_PrepAnimCounting(const TArray<int32>& InAcceptableBets, int32 InBetQuantity);

	UFUNCTION(Server, Reliable)
	void Server_NotifyAnimCountingComplete();

protected:
	
	void SetupInputComponent() override;
	void OnPossess(APawn* InPawn) override;
	
	void RequestDiceRoll();
	void RequestChallenge();
	void RequestBetSubmission();

	void DiceVisualizationComplete();

	UFUNCTION()
	void OnAnimChallengeComplete();

	UFUNCTION()
	void OnAnimCountingComplete();

	TArray<int32> ActiveDiceValues;

private:

	bool bIsActive;

	// bool IsActivePlayer() const;
	int32 GetPlayerIdx() const;

	void HandleGameStarted();
	void RevealUI();
	void HideUI();

	UFUNCTION()
	void HandleFaceChosen(int32 FaceVal);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFaceSelectionWidget> FaceSelectionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMPLeaderboardWidget> LeaderboardWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UBetWidget> BetWidgetClass;

	UFaceSelectionWidget* ActiveFaceSelectionWidget = nullptr;
	UMPLeaderboardWidget* ActiveLeaderboardWidget = nullptr;
	UBetWidget* ActiveBetWidget = nullptr;

};
