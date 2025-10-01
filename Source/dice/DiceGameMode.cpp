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
	
	/* Display the currenet bet */
	// TODO: add to all players?
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	BetWidget = CreateWidget<UBetWidget>(PlayerController, BetWidgetClass);
	if (BetWidget) {
		BetWidget->AddToViewport();
		BetWidget->ResetCurrentBetText();
	}
}

void ADiceGameMode::SubmitBet(FBet& PlayerBet)
{
	int32 NumFaces = PlayerBet.NumFaces;
	int32 Face = PlayerBet.Face;
	int32 PlayerIdx = PlayerBet.PlayerIdx;

	CurrentBet = new FBet(NumFaces, Face, PlayerIdx);

	BetWidget->SetCurrentBetText(PlayerBet.NumFaces, PlayerBet.Face, PlayerBet.PlayerIdx);
	ToggleNextPlayer();
}


/*** 
Current variant : 
   the player may bid a higher quantity of any particular face,
   or the same quantity of a higher face 
***/
bool ADiceGameMode::VerifyBet(FBet& NewBet)
{
	// TODO: Different Variant, Don't allow the player to play the bet in the first place if incorrect

	// If no current bet, its valid
	if (!CurrentBet) return true;
	
	// Higher Quantity
	if (NewBet.NumFaces > CurrentBet->NumFaces)
	{
		return true;
	}

	// Equal Quantity => Higher Face
	if (NewBet.NumFaces == CurrentBet->NumFaces && NewBet.Face > CurrentBet->Face)
	{
		return true;
	}

	// Conditions were not met, hence not valid
	return false;
}

void ADiceGameMode::SubmitChallenge(int32 PlayerIdx)
{
	UE_LOG(LogTemp, Warning, TEXT("Received a challenge!"))
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

	// Sets the player as the current player
	Players[CurrentPlayer]->StartPlayerTurn();

	// TODO: changes in mulitplayer scenario
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		// Sets the current camera to the player
		PlayerController->SetViewTargetWithBlend(Players[CurrentPlayer], .5f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Player Controller!"));
		return;
	}	
}
