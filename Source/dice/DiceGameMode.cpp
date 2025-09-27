// Fill out your copyright notice in the Description page of Project Settings.


#include "DiceGameMode.h"
#include "DicePlayer.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"


ADiceGameMode::ADiceGameMode() {}

void ADiceGameMode::BeginPlay()
{
	Super::BeginPlay();

	// TODO: If InitPlayerNum == 0, Unreal will crush
	if (InitPlayerNum < 2) UE_LOG(LogTemp, Error, TEXT("Not enough players (min 2)"));

	if (!PlayerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerClass is null in DiceGameMode!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;


	/** Spawn all of the players **/
	for (int32 PlayerID = 0; PlayerID < InitPlayerNum; PlayerID++)
	{
		/* Spawns the Player Actor and sets it up */
		FVector SpawnLocation = FVector::ZeroVector;
		SpawnLocation.Y += (PlayerID * 1000); // Player offset 
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FActorSpawnParameters Params;
		Params.Owner = this;

		ADicePlayer* NewPlayer = World->SpawnActor<ADicePlayer>(
			PlayerClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

		if (NewPlayer)
		{
			NewPlayer->PlayerSetup(PlayerID);
			Players.Add(NewPlayer);
		}
	}

	/* Switch to random's player's camera */
	CurrentPlayer = FMath::RandRange(0, Players.Num() - 1);
	ToggleNextPlayer(CurrentPlayer, true);
}

// TODO: will have to decide based on winners/losers, and changes in multiplayer
void ADiceGameMode::ToggleNextPlayer(int32 PlayerIdx, bool Overwrite)
{
	if (Overwrite) // Sets the player that was passed
	{
		CurrentPlayer = PlayerIdx;
	}
	else // Set the next available player
	{
		while (true)
		{
			CurrentPlayer = (CurrentPlayer + 1) % Players.Num();
			ADicePlayer* Player = Players[CurrentPlayer];
			if (Player->DiceCount != 0) break; // Skip players with zero dice
		}

	}

	// TODO: changes in mulitplayer scenario
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		// Sets the current camera to the player
		PlayerController->SetViewTargetWithBlend(Players[CurrentPlayer], .5f);

		/* TODO: Print the Faces (Debugging) */
		TArray<FString> DiceNumbers;
		for (const int32& Value : Players[CurrentPlayer]->DiceRolls)
		{
			DiceNumbers.Add(FString::FromInt(Value));
		}

		UKismetSystemLibrary::PrintString(
			nullptr,
			FString::Printf(TEXT("Player %d: [%s]"), CurrentPlayer, *FString::Join(DiceNumbers, TEXT(", "))),
			true,
			true,
			FLinearColor::White,
			5.f
		);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Player Controller!"));
		return;
	}	
}
