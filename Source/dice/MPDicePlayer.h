// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MPDicePlayer.generated.h"

UCLASS()
class DICE_API AMPDicePlayer : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMPDicePlayer();

	void UpdateDiceCounts(int32 DiceCount);
	void UpdateDiceVisuals(const TArray<int32>& DiceVals);

	void HighlightDice(TArray<int32>& DiceVals, int32 FaceVal);
	void UndoHighlightDice();

	UPROPERTY(Replicated)
	TArray<AActor*> DiceVisuals;

protected:
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Dice")
	TSubclassOf<AActor> DiceClass;


private:
	/** Root Component **/
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	/** Camera **/
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* PlayerCamera;

	/** Light **/
	UPROPERTY(VisibleAnywhere)
	class USpotLightComponent* PlayerLight;

	FRotator GetDiceRotation(int32 FaceVal) const;
};
