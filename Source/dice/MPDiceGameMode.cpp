
#include "MPDiceGameMode.h"
#include "MPDiceGameState.h"
#include "MPDicePlayer.h"
#include "MPDicePlayerController.h"
#include "MPDicePlayerState.h"


AMPDiceGameMode::AMPDiceGameMode() : TurnIdx(0) 
{ 
	GameStateClass = AMPDiceGameState::StaticClass();
	PlayerStateClass = AMPDicePlayerState::StaticClass();
}

void AMPDiceGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AMPDicePlayerState* NewPlayerState = NewPlayer->GetPlayerState<AMPDicePlayerState>())
	{
		NewPlayerState->PlayerIdx = GameState->PlayerArray.Num() - 1;
		UE_LOG(LogTemp, Warning, TEXT("Player %d joined"), NewPlayerState->PlayerIdx);
	}

	// TODO: GameStart when
	if (GameState->PlayerArray.Num() >= 2) StartGame();
}

void AMPDiceGameMode::StartGame()
{
	TurnIdx = 0;

	if (AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>())
	{
		MPGameState->Phase = ETurnPhase::Rolling;
		UE_LOG(LogTemp, Warning, TEXT("Game Started"));
	}
}

void AMPDiceGameMode::PlayerRollComplete(APlayerController* PlayerController)
{
	if (!HasAuthority()) return;

	if (AMPDicePlayerState* MPPlayerState = PlayerController->GetPlayerState<AMPDicePlayerState>())
	{
		MPPlayerState->bRolled = true;
		UE_LOG(LogTemp, Warning, TEXT("Player %d roll complete"), MPPlayerState->PlayerIdx);

		// Check if everyone is done
		CheckStartTurn();
	}
}

void AMPDiceGameMode::CheckStartTurn()
{
	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();
	if (!MPGameState) return;

	for (auto& Player : MPGameState->PlayerArray)
	{
		AMPDicePlayerState* MPPlayer = Cast<AMPDicePlayerState>(Player);
		if (!MPPlayer->bRolled) return;
	}

	// TODO: Broadcast here? Change to turn start?
	if (!MPGameState->bGameStarted)
	{
		MPGameState->bGameStarted = true;
		MPGameState->BroadcastGameStarted();
	}

	// TODO: Check if enough players!!! (LOBBY)

	StartTurn();
}

void AMPDiceGameMode::StartTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("Starting the Turn!"));

	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();
	if (!MPGameState) return;

	MPGameState->Phase = ETurnPhase::Playing;

	for (auto& Player : MPGameState->PlayerArray)
	{
		AMPDicePlayerState* MPPlayer = Cast<AMPDicePlayerState>(Player);
		MPPlayer->bRolled = false;
	}

	if (!NextPlayer)
	{
		NextTurn(ETurnModeSelect::Random);
	}
	else
	{
		int32 SpecificPlayerIdx = NextPlayer->PlayerIdx;
		NextPlayer = nullptr;
		NextTurn(ETurnModeSelect::Specific, SpecificPlayerIdx);
	}
}

void AMPDiceGameMode::NextTurn(ETurnModeSelect SelectionMode, int32 SpecificPlayerIdx)
{
	if (!HasAuthority()) return;

	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();
	if (!MPGameState) return;
	auto& Players = MPGameState->PlayerArray;

	if (Players.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No players in game!"));
		return;
	}

	// Helper lambda to check if a player is valid and active
	auto IsPlayerInPlay = [](APlayerState* PS) -> bool
		{
			if (!PS) return false;

			AMPDicePlayerState* DicePS = Cast<AMPDicePlayerState>(PS);
			if (!DicePS) return false;

			// Check if player is still in the game
			return DicePS->DiceCount > 0;
		};

	// Get list of active player indices
	TArray<int32> ActivePlayerIndices;
	for (int32 i = 0; i < Players.Num(); i++)
	{
		if (IsPlayerInPlay(Players[i]))
		{
			ActivePlayerIndices.Add(i);
		}
	}

	if (ActivePlayerIndices.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No active players remaining!"));
		return;
	}

	int32 NewTurnIdx = TurnIdx;

	switch (SelectionMode)
	{
		case ETurnModeSelect::Random:
		{
			// Pick a random active player
			int32 RandomActiveIdx = FMath::RandRange(0, ActivePlayerIndices.Num() - 1);
			NewTurnIdx = ActivePlayerIndices[RandomActiveIdx];

			UE_LOG(LogTemp, Warning, TEXT("[Random] Selected player %d from %d active players"),
				NewTurnIdx, ActivePlayerIndices.Num());
			break;
		}

		case ETurnModeSelect::Next:
		{
			// Find current player's position in active players list
			int32 CurrentActiveIdx = ActivePlayerIndices.IndexOfByKey(TurnIdx);

			if (CurrentActiveIdx == INDEX_NONE)
			{
				// Current player is not active, start from first active player
				NewTurnIdx = ActivePlayerIndices[0];
				UE_LOG(LogTemp, Warning, TEXT("[Next] Current player %d not active, starting from %d"),
					TurnIdx, NewTurnIdx);
			}
			else
			{
				// Move to next active player (circular)
				int32 NextActiveIdx = (CurrentActiveIdx + 1) % ActivePlayerIndices.Num();
				NewTurnIdx = ActivePlayerIndices[NextActiveIdx];
				UE_LOG(LogTemp, Warning, TEXT("[Next] Moving from player %d to %d"),
					TurnIdx, NewTurnIdx);
			}
			break;
		}

		case ETurnModeSelect::Specific:
		{
			if (SpecificPlayerIdx == -1)
			{
				UE_LOG(LogTemp, Error, TEXT("[Specific] Player index cannot be -1"));
				return;
			}

			if (!Players.IsValidIndex(SpecificPlayerIdx))
			{
				UE_LOG(LogTemp, Error, TEXT("[Specific] Invalid player index: %d"), SpecificPlayerIdx);
				return;
			}

			if (!IsPlayerInPlay(Players[SpecificPlayerIdx]))
			{
				UE_LOG(LogTemp, Error, TEXT("[Specific] Player %d is not active (eliminated or disconnected)"),
					SpecificPlayerIdx);
				return;
			}

			NewTurnIdx = SpecificPlayerIdx;
			UE_LOG(LogTemp, Warning, TEXT("[Specific] Setting turn to player %d"), NewTurnIdx);
			break;
		}
	}

	// Final validation
	if (!Players.IsValidIndex(NewTurnIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid turn index calculated: %d"), NewTurnIdx);
		return;
	}

	if (!IsPlayerInPlay(Players[NewTurnIdx]))
	{
		UE_LOG(LogTemp, Error, TEXT("Selected player %d is not active!"), NewTurnIdx);
		return;
	}

	// Update turn
	TurnIdx = NewTurnIdx;
	MPGameState->ActivePlayer = Cast<AMPDicePlayerState>(Players[TurnIdx]);

	NotifyPlayerTurn(TurnIdx);

	UE_LOG(LogTemp, Warning, TEXT("Turn switched to Player %d (%d active players total)"),
		TurnIdx, ActivePlayerIndices.Num());
}

void AMPDiceGameMode::NotifyRoundRestart() const
{
	if (!HasAuthority()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			AMPDicePlayerController* MPPC = Cast<AMPDicePlayerController>(PC);

			if (!MPPC) continue;

			MPPC->Client_NotifyTurnRestart();
		}
	}
}

void AMPDiceGameMode::NotifyPlayerTurn(int32 PlayerIdx) const
{
	if (!HasAuthority()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			AMPDicePlayerState* PS = PC->GetPlayerState<AMPDicePlayerState>();
			AMPDicePlayerController* MPPC = Cast<AMPDicePlayerController>(PC);

			if (!PS || !MPPC) continue;

			// Notify which player is the current player
			if (PS->PlayerIdx == PlayerIdx) // This is the active player - show their UI
			{	 
				MPPC->Client_NotifyTurnStart();
			}
			else // Not their turn - hide UI
			{
				MPPC->Client_NotifyTurnEnd();
			}
		}
	}
}

void AMPDiceGameMode::RollDice(AMPDicePlayerState* PlayerState)
{
	if (!PlayerState || !HasAuthority()) return;

	if (AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>())
	{
		if (MPGameState->Phase != ETurnPhase::Rolling)
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot roll dice. Wrong phase"));
			return;
		}
	}

	// TODO: Skip players without dice

	PlayerState->DiceValues.Empty();
	PlayerState->bRolled = true;

	// Spawn the die depending on how many the player has
	for (int32 DiceIdx = 0; DiceIdx < PlayerState->DiceCount; DiceIdx++)
	{
		/* Generate a random dice face */
		const int32 RandomFace = FMath::RandRange(1, 6);
		PlayerState->DiceValues.Add(RandomFace);

		UE_LOG(LogTemp, Warning, TEXT("Player %d roll: %d"), PlayerState->PlayerIdx, RandomFace);
	}

	DiceToOwner(PlayerState);
}


void AMPDiceGameMode::RevealDice()
{
	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();
	if (!MPGameState) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMPDicePlayerController* PC = Cast<AMPDicePlayerController>(It->Get());
		if (!PC) continue;

		AMPDicePlayerState* MPPlayer = PC->GetPlayerState<AMPDicePlayerState>();
		if (!MPPlayer) continue;

		// TODO:
		TArray<int32> AcceptableBets;
		AcceptableBets.Add(MPGameState->CurrentBet.Face);
		PC->Client_PrepAnimCounting(AcceptableBets, MPGameState->CurrentBet.Quantity);
		
		MPPlayer->RevealDice();
	}
}

void AMPDiceGameMode::HideDice()
{
	AMPDiceGameState* GS = GetGameState<AMPDiceGameState>();
	if (!GS) return;

	for (APlayerState* Player : GS->PlayerArray)
	{
		AMPDicePlayerState* MPPlayer = Cast<AMPDicePlayerState>(Player);
		if (!MPPlayer) continue;

		MPPlayer->HideDice();
	}
}

void AMPDiceGameMode::DiceToOwner(AMPDicePlayerState* PlayerState) const
{
	if (!PlayerState) return;

	APlayerController* DicePC = Cast<APlayerController>(PlayerState->GetOwner());
	if (!DicePC) return;

	if (AMPDicePlayerController* MPDicePC = Cast<AMPDicePlayerController>(DicePC))
	{
		MPDicePC->Client_ReceiveOwnDice(PlayerState->DiceValues);
	}
}

// TODO: PERUDO (jokers) -> switch game modes?
// TODO: Extra Rules
// TODO: Get Acceptable Bets
bool AMPDiceGameMode::IsValidBet(int32 Quantity, int32 Face)
{
	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();

	if (!MPGameState) return false;

	// Verify face within range
	if (Face > 6 || Face < 1 || Quantity < 1) return false;

	// Rule #1: Quantity Increased
	if (Quantity > MPGameState->CurrentBet.Quantity) return true;
	
	// Rule #2: Same Quantity, but Higher Face
	if (Quantity == MPGameState->CurrentBet.Quantity && Face > MPGameState->CurrentBet.Face) return true;

	return false;
}

void AMPDiceGameMode::OnPlayerBet(APlayerController* Bettor, int32 Quantity, int32 FaceVal)
{
	if (!IsValidBet(Quantity, FaceVal))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Bet!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Bet submitted!\nFace: %d \nQuantity: %d \n"), Quantity, FaceVal);
	if (AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>())
	{
		if (!Bettor) return;
		
		
		MPGameState->CurrentBet = FCurrentBet(
			Quantity, 
			FaceVal, 
			Bettor->GetPlayerState<AMPDicePlayerState>()
		);

		MPGameState->BroadcastBetChanged();
	}

	NextTurn();
}

void AMPDiceGameMode::OnPlayerChallenge(APlayerController* InChallenger)
{
	UE_LOG(LogTemp, Warning, TEXT("Challenge submitted!"));

	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();
	if (!MPGameState) return;

	/* Check if bet exists */
	if (!MPGameState->CurrentBet.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Challenge failed, no bet"));
		return;
	}
	
	Challenger = InChallenger;
	MPGameState->Phase = ETurnPhase::Waiting;

	// Challenge Animation
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMPDicePlayerController* PC = Cast<AMPDicePlayerController>(It->Get());
		if (PC)
		{
			PC->Client_NotifyTurnEnd();
			PC->Client_StartAnimChallenge();
		}
	}
}

int AMPDiceGameMode::CountCurrentBet() const
{
	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();
	if (!MPGameState) return 0;

	int DiceCount = 0;
	for (auto& Player : MPGameState->PlayerArray)
	{
		AMPDicePlayerState* MPPlayer = Cast<AMPDicePlayerState>(Player);
		if (!MPPlayer) continue;

		for (int32 FaceVal : MPPlayer->DiceValues)
		{
			if (FaceVal == MPGameState->CurrentBet.Face)
				DiceCount++;
		}
	}
	return DiceCount;
}

void AMPDiceGameMode::OnAnimChallengeComplete(APlayerController* InPlayerController)
{
	if (!CheckAnimComplete(InPlayerController)) return;
	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Challenge Animation Complete"));

	// Check winner/looser of the round
	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();
	if (!MPGameState) return;
	
	int DiceCount = CountCurrentBet();

	AMPDicePlayerState* BettingPlayer = Cast<AMPDicePlayerState>(MPGameState->CurrentBet.BettingPlayer);
	Loser = (DiceCount >= MPGameState->CurrentBet.Quantity) 
		? BettingPlayer : Challenger->GetPlayerState<AMPDicePlayerState>();

	Loser->DiceCount--;
	Challenger = nullptr;
	NextPlayer = Loser; 
	
	// TODO: Separate Reveal and Counting Animation?
	RevealDice();
}

void AMPDiceGameMode::DestroyDice()
{
	if (!HasAuthority()) return;

	AMPDiceGameState* GS = GetGameState<AMPDiceGameState>();
	if (!GS) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMPDicePlayerController* PC = Cast<AMPDicePlayerController>(It->Get());
		if (!PC) return;
		
		PC->Client_StartAnimDestruction(Loser->PlayerIdx);
	}
}

void AMPDiceGameMode::OnAnimCountingComplete(APlayerController* InPlayerController)
{
	if (!CheckAnimComplete(InPlayerController)) return;
	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Counting Animation Complete"));
	
	DestroyDice();
}


void AMPDiceGameMode::OnAnimDestroyComplete(APlayerController* InPlayerController)
{
	// UE_LOG("[GameMode] Destroy Animations Complete");

	if (!CheckAnimComplete(InPlayerController)) return;
	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Destroy Animations Complete"));
	
	// TODO: check
	AMPDicePlayer* LoserDicePawn = Loser->GetDicePawn();
	if (!LoserDicePawn) return;
	LoserDicePawn->UpdateDiceCounts(Loser->DiceCount);

	EndOfRound();
}

bool AMPDiceGameMode::CheckAnimComplete(APlayerController* InPlayerController)
{
	CompletedAnimationPlayers.Add(InPlayerController);

	if (CompletedAnimationPlayers.Num() < GetNumPlayers()) 
		return false;
	
	CompletedAnimationPlayers.Empty();
	return true;

}

void AMPDiceGameMode::EndOfRound()
{
	AMPDiceGameState* MPGameState = GetGameState<AMPDiceGameState>();
	if (!MPGameState) return;

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] End of Round"));

	//// TODO: END GAME
	//int32 PlayerCounter = 0;
	//for (auto& Player : MPGameState->PlayerArray)
	//{
	//	AMPDicePlayerState* MPPlayer = Cast<AMPDicePlayerState>(Player);
	//	if (MPPlayer->DiceCount >= 0) PlayerCounter++;

	//	if (PlayerCounter > 1) break;
	//}

	//if (PlayerCounter <= 1)
	//{
	//	MPGameState->Phase = ETurnPhase::Finished;
	//	return;
	//}


	HideDice();

	// Reset bet values
	Loser = nullptr;
	MPGameState->CurrentBet.Reset();
	MPGameState->BroadcastBetChanged();
	MPGameState->Phase = ETurnPhase::Rolling;

	NotifyRoundRestart();
	NotifyPlayerTurn(INDEX_NONE); // Hides UI for all players
}