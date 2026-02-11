
#include "MPDicePlayerController.h"
#include "MPDiceGameMode.h"
#include "MPDiceGameState.h"
#include "MPDicePlayer.h"
#include "MPDicePlayerState.h"
#include "TurnPhase.h"

void AMPDicePlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    bIsActive = false;

    TArray<FString> FailedUI;
    if (IsLocalController())
    {
        UE_LOG(LogTemp, Warning, TEXT("Setting up UI"));
        /* Face Selection Widget */
        ActiveFaceSelectionWidget = CreateWidget<UFaceSelectionWidget>(this, FaceSelectionWidgetClass);
        if (ActiveFaceSelectionWidget)
        {
            ActiveFaceSelectionWidget->OnFaceChosen.AddDynamic(this, &AMPDicePlayerController::HandleFaceChosen);

            ActiveFaceSelectionWidget->AddToViewport(10);
            ActiveFaceSelectionWidget->SetVisibility(ESlateVisibility::Hidden);
        } 
        else
        {
            FailedUI.Add(TEXT("FaceSelectionWidget"));
        }

        /* Leaderboard Widget */
        ActiveLeaderboardWidget = CreateWidget<UMPLeaderboardWidget>(this, LeaderboardWidgetClass);
        if (ActiveLeaderboardWidget)
        {
            ActiveLeaderboardWidget->OnCountingAnimComplete.AddDynamic(this, &AMPDicePlayerController::OnAnimCountingComplete);

            ActiveLeaderboardWidget->AddToViewport(0);
            ActiveLeaderboardWidget->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            FailedUI.Add(TEXT("LeaderboardWidget"));
        }

        /* Bet Widget */
        ActiveBetWidget = CreateWidget<UBetWidget>(this, BetWidgetClass);
        if (ActiveBetWidget) {
            
            ActiveBetWidget->OnChallengeAnimComplete.AddDynamic(this, &AMPDicePlayerController::OnAnimChallengeComplete);
            
            ActiveBetWidget->AddToViewport(1);
            ActiveBetWidget->ResetCurrentBetText();
        }
        else
        {
            FailedUI.Add(TEXT("BetWidget"));
        }
    }

    if (!IsLocalController() || !FailedUI.IsEmpty())
    {
        FString Joined = FString::Join(FailedUI, TEXT(", "));
        UE_LOG(LogTemp, Error, TEXT("Failed setting up UI; %s"), *Joined);
    }

    if (AMPDiceGameState* GS = GetWorld()->GetGameState<AMPDiceGameState>())
    {
        UE_LOG(LogTemp, Warning, TEXT("Setup: HandleGameStart"));
        GS->OnGameStarted.AddUObject(this, &AMPDicePlayerController::HandleGameStarted);
    }
}

void AMPDicePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("StartDiceRoll", IE_Pressed, this, &AMPDicePlayerController::RequestDiceRoll);
    InputComponent->BindAction("SubmitBet", IE_Pressed, this, &AMPDicePlayerController::RequestBetSubmission);
    InputComponent->BindAction("Challenge", IE_Pressed, this, &AMPDicePlayerController::RequestChallenge);
}

void AMPDicePlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!HasAuthority()) return;

    AMPDiceGameState* MPGameState = GetWorld()->GetGameState<AMPDiceGameState>();
    AMPDicePlayerState* MPPlayerState = GetPlayerState<AMPDicePlayerState>();
    AMPDicePlayer* DicePawn = Cast<AMPDicePlayer>(InPawn);

    if (MPGameState && DicePawn && MPPlayerState)
    {
        MPPlayerState->DiceCount = MPGameState->InitDiceNum;
        DicePawn->UpdateDiceCounts(MPGameState->InitDiceNum);
    }
}

void AMPDicePlayerController::HandleGameStarted()
{
    if (ActiveLeaderboardWidget)
    {
        ActiveLeaderboardWidget->SetVisibility(ESlateVisibility::Visible);
        ActiveLeaderboardWidget->SetupPlayers();
        ActiveLeaderboardWidget->UpdatePlayerDice(GetPlayerIdx(), ActiveDiceValues);
    }
}


// TODO: Setup the UI slider 
// [minimum dynamically when pressed on buttons, disable buttons player can't select]
void AMPDicePlayerController::RevealUI()
{
    if (ActiveFaceSelectionWidget && IsLocalController())
    {
        ActiveFaceSelectionWidget->SetVisibility(ESlateVisibility::Visible);
        ActiveFaceSelectionWidget->SetIsEnabled(true);

        AMPDiceGameState* MPGameState = GetWorld()->GetGameState<AMPDiceGameState>();
        ActiveFaceSelectionWidget->SetupSlider(1, MPGameState->GetTotalDice());

        bShowMouseCursor = true;
    }
}

void AMPDicePlayerController::HideUI()
{
    if (ActiveFaceSelectionWidget && IsLocalController())
    {
        ActiveFaceSelectionWidget->ResetSlider();
        ActiveFaceSelectionWidget->ResetButtonClicks();

        ActiveFaceSelectionWidget->SetVisibility(ESlateVisibility::Hidden);
        ActiveFaceSelectionWidget->SetIsEnabled(false);

        bShowMouseCursor = false;
    }
}

void AMPDicePlayerController::HandleFaceChosen(int32 FaceVal)
{
    UE_LOG(LogTemp, Warning, TEXT("Face Selected: %d"), FaceVal);
    ActiveFaceSelectionWidget->FaceSelected = FaceVal;

    AMPDicePlayer* DicePawn = GetPawn<AMPDicePlayer>();
    if (!DicePawn) return;
    
    DicePawn->HighlightDice(ActiveDiceValues, FaceVal);
}

/********************************/
/** Server, Client: Animations **/
/********************************/
void AMPDicePlayerController::OnAnimChallengeComplete()
{
    Server_NotifyAnimChallengeComplete();
}

void AMPDicePlayerController::OnAnimCountingComplete()
{
    Server_NotifyAnimCountingComplete();
}

void AMPDicePlayerController::Server_NotifyAnimChallengeComplete_Implementation()
{
    AMPDiceGameMode* GM = GetWorld()->GetAuthGameMode<AMPDiceGameMode>();
    if (GM)
    {
        GM->OnAnimChallengeComplete(this);
    }
}

void AMPDicePlayerController::Server_NotifyAnimCountingComplete_Implementation()
{
    AMPDiceGameMode* GM = GetWorld()->GetAuthGameMode<AMPDiceGameMode>();
    if (GM)
    {
        GM->OnAnimCountingComplete(this);
    }
}

void AMPDicePlayerController::Client_StartAnimChallenge_Implementation()
{
    if (ActiveBetWidget)
    {
        ActiveBetWidget->ChallengeStart();
    }
}


// TODO: Not used
void AMPDicePlayerController::Client_PrepAnimCounting_Implementation(const TArray<int32>& InAcceptableBets, int32 InBetQuantity)
{
    if (!ActiveLeaderboardWidget || !ActiveBetWidget) return;

    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Preparing counting animation"));

    ActiveLeaderboardWidget->PrepCountingAnimation(InAcceptableBets, ActiveBetWidget, InBetQuantity);
}



/****************************/
/** Server: Bet Submission **/
/****************************/
void AMPDicePlayerController::RequestBetSubmission()
{
    if (!bIsActive || !ActiveFaceSelectionWidget) return;
    Server_RequestBetSubmission(ActiveFaceSelectionWidget->NumOfFaces, ActiveFaceSelectionWidget->FaceSelected);
}

void AMPDicePlayerController::Server_RequestBetSubmission_Implementation(int32 NumOfFaces, int32 FaceSelected)
{
    UE_LOG(LogTemp, Warning, TEXT("Bet Request"));
    
    AMPDiceGameMode* MPGameMode = GetWorld()->GetAuthGameMode<AMPDiceGameMode>();
    if (!MPGameMode) return;

    AMPDiceGameState* MPGameState = GetWorld()->GetGameState<AMPDiceGameState>();
    if (!MPGameState && MPGameState->Phase != ETurnPhase::Playing) return;

    MPGameMode->OnPlayerBet(this, NumOfFaces, FaceSelected);
}

/***********************/
/** Server: Challenge **/
/********************#**/
void AMPDicePlayerController::RequestChallenge()
{
    if (!bIsActive) return;
    Server_RequestChallenge();
}

void AMPDicePlayerController::Server_RequestChallenge_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Challenge Request"));

    AMPDiceGameMode* MPGameMode = GetWorld()->GetAuthGameMode<AMPDiceGameMode>();
    if (!MPGameMode) return;

    AMPDiceGameState* MPGameState = GetWorld()->GetGameState<AMPDiceGameState>();
    if (!MPGameState && MPGameState->Phase != ETurnPhase::Playing) return;

    // TODO:
    MPGameMode->OnPlayerChallenge(this);
}


/********************************/
/** Server: Request Dice Roll **/
/********************************/
void AMPDicePlayerController::RequestDiceRoll()
{
    Server_RequestDiceRoll();
}

void AMPDicePlayerController::Server_RequestDiceRoll_Implementation()
{
    // TODO: Check if already rolled

    AMPDiceGameState* MPGameState = GetWorld()->GetGameState<AMPDiceGameState>();
    if (!MPGameState) return;

    if (MPGameState->Phase != ETurnPhase::Rolling) return;

    AMPDiceGameMode* MPGameMode = GetWorld()->GetAuthGameMode<AMPDiceGameMode>();
    if (!MPGameMode) return;

    if (AMPDicePlayerState* MPPlayerState = GetPlayerState<AMPDicePlayerState>())
    {
        MPGameMode->RollDice(MPPlayerState);
    }
}

/********************************/
/** Client: Receive Dice Rolls **/
/********************************/
void AMPDicePlayerController::Client_ReceiveOwnDice_Implementation(const TArray<int32>& DiceValues)
{
    AMPDicePlayer* DicePawn = GetPawn<AMPDicePlayer>();
    if (DicePawn)
    {
        ActiveDiceValues = DiceValues;
        DicePawn->UpdateDiceVisuals(ActiveDiceValues);


        // TODO: Animation
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(
            TimerHandle,
            this,
            &AMPDicePlayerController::DiceVisualizationComplete,
            0.1f,
            false
        );
    }

    if (ActiveLeaderboardWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Updating Player Dice for Player %d"), GetPlayerIdx());
        ActiveLeaderboardWidget->UpdatePlayerDice(GetPlayerIdx(), ActiveDiceValues);
    }
}

/*************************/
/** Dice Visualziations **/
/*************************/
void AMPDicePlayerController::DiceVisualizationComplete()
{
    Server_NotifyRollComplete();
}

void AMPDicePlayerController::Server_NotifyRollComplete_Implementation()
{
    if (AMPDiceGameMode* MPGameMode = GetWorld()->GetAuthGameMode<AMPDiceGameMode>())
    {
        MPGameMode->PlayerRollComplete(this);
    }
}


// TODO: Add boolean bActive flag
void AMPDicePlayerController::Client_NotifyTurnStart_Implementation()
{
    bIsActive = true;
    RevealUI();
}

void AMPDicePlayerController::Client_NotifyTurnEnd_Implementation()
{
    bIsActive = false;
    HideUI();

    if (AMPDicePlayer* DicePawn = GetPawn<AMPDicePlayer>())
    {
        DicePawn->UndoHighlightDice();
    }
}



/**********************/
/** Helper Functions **/
/**********************/

int32 AMPDicePlayerController::GetPlayerIdx() const
{
    if (AMPDicePlayerState* MPPlayerState = GetPlayerState<AMPDicePlayerState>())
    {
        return MPPlayerState->PlayerIdx;
    }

    return INDEX_NONE;
}
