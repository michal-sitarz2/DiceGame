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

void ADiceGameMode::SubmitChallenge(int32 ChallengerIdx)
{
	if (!CurrentBet) 
	{
		// Debug message
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, TEXT("No bet to challenge!"));
		return;
	}

	// Increment the counter of dice faces
	int32 Counter = 0;
	int32 CheckFace = CurrentBet->Face;
	for (ADicePlayer* Player : Players)
	{
		for (int32 DiceFace : Player->DiceRolls)
		{
			// TODO: JOKER variant
			if (DiceFace == CheckFace)
			{
				Counter++;
			}
		}
	}

	int32 PredNumFaces = CurrentBet->NumFaces;

	// Player that made the bet lost if predicted too many faces, otherwise challenger loses
	int32 LoserIdx = PredNumFaces > Counter ? CurrentBet->PlayerIdx : ChallengerIdx;
	
	// Debug message
	GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, FString::Printf(TEXT("Player %d lost the round"), LoserIdx));

	// Decrease the number of die the losing player has
	Players[LoserIdx]->DiceCount--;


	// Reroll all the player dice
	// TODO: Deal with a player that has zero dice? 
	// in theory should not roll any dice and toggle should skip them
	for (ADicePlayer* Player : Players)
	{
		Player->RollDice();
	}

	// Reset the current bet
	CurrentBet = nullptr;
	BetWidget->ResetCurrentBetText();

	// Switch to player who lost
	ToggleNextPlayer(LoserIdx, true, 1.f);
}

void ADiceGameMode::SubmitBet(FBet& PlayerBet)
{
	int32 NumFaces = PlayerBet.NumFaces;
	int32 Face = PlayerBet.Face;
	int32 PlayerIdx = PlayerBet.PlayerIdx;

	CurrentBet = new FBet(NumFaces, Face, PlayerIdx);

	BetWidget->SetCurrentBetText(NumFaces, Face, PlayerIdx);
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

// TODO: will have to decide based on winners/losers, and changes in multiplayer
void ADiceGameMode::ToggleNextPlayer(int32 PlayerIdx, bool Overwrite, float BlendSpeed)
{
	CheckEndGame();

	if (Overwrite) // Sets the player that was passed
	{
		CurrentPlayer = PlayerIdx;
	}
	if (!Overwrite || Players[CurrentPlayer]->DiceCount == 0) // Set the next available player
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
		PlayerController->SetViewTargetWithBlend(Players[CurrentPlayer], BlendSpeed);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Player Controller!"));
		return;
	}	
}

void ADiceGameMode::CheckEndGame()
{
	TArray<int> InPlay;
	for (ADicePlayer* Player : Players)
	{
		if (Player->DiceCount > 0)
		{
			InPlay.Add(Player->PlayerID);
		}

		if (InPlay.Num() > 1) return; // More than one player in play
	}

	if (InPlay.Num() == 1)
	{
		// Debug message
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Winner: %d"), InPlay[0]));

		// TODO: Differs for multiplayer
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		PlayerController->SetPause(true);
	}
}