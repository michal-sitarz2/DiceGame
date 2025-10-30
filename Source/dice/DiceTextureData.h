// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DiceTextureData.generated.h"

/**
 * 
 */
UCLASS()
class DICE_API UDiceTextureData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<int32, UTexture2D*> DiceTextures;

};
