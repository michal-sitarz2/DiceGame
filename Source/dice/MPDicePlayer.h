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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiceDestructionComplete);
	UPROPERTY(BlueprintAssignable)
	FOnDiceDestructionComplete OnDiceDestructionComplete;

	void StartDestructionAnimation();

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

	/** Root Component **/
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	/** Camera **/
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* PlayerCamera;

	/** Light **/
	UPROPERTY(VisibleAnywhere)
	class USpotLightComponent* PlayerLight;


private:
	FRotator GetDiceRotation(int32 FaceVal) const;
	
	////////////////////////
	/** Dice Destruction **/
	////////////////////////

	void PlayDestructionAnimation();

	/* Reference to the dice being destroyed */
	AActor* DiceDestroy = nullptr;

	/* Dissolve step speed */
	float DissolveStep = 0.05f;

	/* Starting Dissolve value for dice destruction */
	float DissolveVal = -1.f;

	/* Timer Handle for Destruction animation */
	FTimerHandle DestroyTimerHandle;

	/* List of material instances for the die about to be destroyed */
	TArray<UMaterialInstanceDynamic*> Materials;
};
