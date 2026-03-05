// Fill out your copyright notice in the Description page of Project Settings.


#include "DiceGameMode.h"
#include "DicePlayer.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"


ADiceGameMode::ADiceGameMode() 
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ADiceGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (InitPlayerNum < 2) UE_LOG(LogTemp, Error, TEXT("Not enough players (min 2)"));

	if (!PlayerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerClass is null in DiceGameMode!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	ChallengerIdx = -1;
	LoserIdx = -1;

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
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	BetWidget = CreateWidget<UBetWidget>(PlayerController, BetWidgetClass);
	if (BetWidget) {
		BetWidget->AddToViewport();
		BetWidget->ResetCurrentBetText();
	}

	ResetChallengeAnimFlags();
}

void ADiceGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (AnimChallengeState == EAnimState::Finished && AnimCountingState == EAnimState::NotStarted)
	{ 
		// TODO: Multiplayer
		// TODO: FVector(0,0)
		Players[CurrentPlayer]->StartCountingAnim(CurrentCorrectBets, AnimCountingState, BetWidget, CurrentBet->NumFaces);

	}

	if (AnimCountingState == EAnimState::Finished && AnimDestructionState == EAnimState::NotStarted)
	{
		// Remove Loser's Dice
		Players[LoserIdx]->RemoveDice(AnimDestructionState);

		// TODO: Multiplayer - update UI
		Players[CurrentPlayer]->RemoveDiceUI(AnimDestructionUIState);

		// TODO - Multiplayer: Winner / Loser animation	
		// ActiveLeaderWidget->SetLoser(PlayerID);

	}

	/* At the end of the challenge (after all the animations have finished) toggle next player */
	if (AnimDestructionState == EAnimState::Finished && AnimDestructionUIState == EAnimState::Finished)
	{
		// TODO: Multiplayer Close the UI
		Players[ChallengerIdx]->OnCloseBetUI();

		// Reroll all the player dice
		for (ADicePlayer* Player : Players)
		{
			Player->RollDice();
		}

		// Reset the current bet
		CurrentBet = nullptr;
		BetWidget->ResetCurrentBetText();

		// Remove the acceptable bets
		CurrentCorrectBets.Empty();

		// Switch to player who lost
		ToggleNextPlayer(LoserIdx, true, 1.f);
		
		ChallengerIdx = -1;
		LoserIdx = -1;
		ResetChallengeAnimFlags();
	}
}

void ADiceGameMode::ResetChallengeAnimFlags()
{
	AnimCountingState = EAnimState::NotStarted;
	AnimChallengeState = EAnimState::NotStarted;
	AnimDestructionState = EAnimState::NotStarted;
	AnimDestructionUIState = EAnimState::NotStarted;
}

void ADiceGameMode::SubmitChallenge(int32 Challenger)
{
	if (!CurrentBet) 
	{
		// Debug message
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, TEXT("No bet to challenge!"));
		return;
	}

	// Increment the counter of dice faces
	int32 DiceCounter = 0;
	int32 CheckFace = CurrentBet->Face;
	for (ADicePlayer* Player : Players)
	{
		for (int32 DiceFace : Player->DiceRolls)
		{
			// TODO: JOKER variant
			if (DiceFace == CheckFace) DiceCounter++;
		}
	}

	int32 PredNumFaces = CurrentBet->NumFaces;


	// Player that made the bet lost if predicted too many faces, otherwise challenger loses
	LoserIdx = PredNumFaces > DiceCounter ? CurrentBet->PlayerIdx : Challenger;
	ChallengerIdx = Challenger;

	// Debug message
	GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, FString::Printf(TEXT("Player %d lost the round"), LoserIdx));

	Players[ChallengerIdx]->RevealLeaderboard(ChallengerIdx == LoserIdx);
	BetWidget->ChallengeStart(AnimChallengeState); // , DiceCounter);
}

void ADiceGameMode::SubmitBet(FBet& PlayerBet)
{
	int32 NumFaces = PlayerBet.NumFaces;
	int32 Face = PlayerBet.Face;
	int32 PlayerIdx = PlayerBet.PlayerIdx;

	CurrentBet = new FBet(NumFaces, Face, PlayerIdx);

	// TODO - Variant : Excludes the jokers!
	CurrentCorrectBets.Empty();
	CurrentCorrectBets.Add(Face);


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
	if (!Overwrite || Players[CurrentPlayer]->DiceRolls.Num() == 0) // Set the next available player
	{
		while (true)
		{
			CurrentPlayer = (CurrentPlayer + 1) % Players.Num();
			ADicePlayer* Player = Players[CurrentPlayer];
			if (Player->DiceRolls.Num() != 0) break; // Skip players with zero dice
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

bool ADiceGameMode::ReadyToPlay()
{
	bool bIsReady = true;
	for (ADicePlayer* Player : Players)
	{
		bIsReady &= Player->bHasSettled;
	}

	return bIsReady;
}

// TODO:
void ADiceGameMode::CheckEndGame()
{
	TArray<int> InPlay;
	for (ADicePlayer* Player : Players)
	{
		if (Player->DiceRolls.Num() > 0)
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

void ADiceGameMode::RevealLeaderboard(TMap<int32, TArray<int32>>& Leaderboard, bool Full)
{
	Leaderboard.Empty();

	for (ADicePlayer* Player : Players)
	{
		Leaderboard.Add(Player->PlayerID, { -1,-1,-1,-1,-1 }); // default: no texture
		
		// TODO: Sort the dice rolls

		int32 Count = 0;

		/* Update the leaderboard with the revealed current dice */
		TArray<int32> SortedDiceRolls = Player->DiceRolls;
		SortedDiceRolls.Sort();

		for (int32 Dice : SortedDiceRolls)
		{
			// Black texture (0) if no reveal, else we save the face number
			Leaderboard[Player->PlayerID][Count] = Full ? Dice : 0;
			Count++;
		}
	}
}