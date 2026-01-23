
#pragma once

#include "CoreMinimal.h"
#include "BetWidget.h"
#include "FaceSelectionWidget.h"
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
	void Server_NotifyChallengeAnimComplete();

	UFUNCTION(Client, Reliable)
	void Client_ReceiveOwnDice(const TArray<int32>& DiceValues);

	UFUNCTION(Client, Reliable)
	void Client_NotifyTurnStart();

	UFUNCTION(Client, Reliable)
	void Client_NotifyTurnEnd();

	UFUNCTION(Client, Reliable)
	void Client_ShowChallengeAnim();

protected:
	void SetupInputComponent() override;
	void OnPossess(APawn* InPawn) override;
	
	void RequestDiceRoll();
	void RequestChallenge();
	void RequestBetSubmission();

	UFUNCTION()
	void OnChallengeAnimComplete();
	void DiceVisualizationComplete();

	TArray<int32> ActiveDiceValues;

private:

	bool bIsActive;

	// bool IsActivePlayer() const;
	int32 GetPlayerIdx() const;

	void RevealUI();
	void HideUI();

	UFUNCTION()
	void HandleFaceChosen(int32 FaceVal);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFaceSelectionWidget> FaceSelectionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UBetWidget> BetWidgetClass;

	UFaceSelectionWidget* ActiveFaceSelectionWidget = nullptr;
	UBetWidget* ActiveBetWidget = nullptr;

};
