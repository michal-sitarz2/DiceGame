// Fill out your copyright notice in the Description page of Project Settings.


#include "DicePlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Components/SpotLightComponent.h"

// Called when the game starts or when spawned
void ADicePlayer::BeginPlay()
{
	Super::BeginPlay();

	DiceDirectionsMap = {
		{ FVector::UpVector, 6 },
		{ FVector::DownVector, 1 },
		{ FVector::ForwardVector, 3 },
		{ FVector::BackwardVector, 4 },
		{ FVector::RightVector, 2 },
		{ FVector::LeftVector, 5 }
	};
}

// Sets default values
ADicePlayer::ADicePlayer()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlayerID = -1;
	bHasSettled = true;
	bIsWaiting = false;
	TimeStationary = 0.f;

	GameMode = Cast<ADiceGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	// Root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Camera setup
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->SetRelativeLocation(FVector(-125.f, 0.f, 140.f));
	PlayerCamera->SetWorldRotation(FRotator(-40.f, 0.f, 0.f));

	// Spotlight setup
	PlayerLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("PlayerLight"));
	PlayerLight->SetupAttachment(RootComponent);
	PlayerLight->SetRelativeLocation(FVector(0.f, 0.f, 250.f));
	PlayerLight->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	PlayerLight->Intensity = 2000.f;

	// Front Wall 
	UBoxComponent* FrontWall = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontWall"));
	FrontWall->SetupAttachment(RootComponent);
	FrontWall->InitBoxExtent(FVector(100.f, 1.f, 100.f));  
	FrontWall->SetRelativeLocation(FVector(0.f, -99.f, 0.f)); 

	// Back Wall
	UBoxComponent* BackWall = CreateDefaultSubobject<UBoxComponent>(TEXT("BackWall"));
	BackWall->SetupAttachment(RootComponent);
	BackWall->InitBoxExtent(FVector(100.f, 1.f, 100.f));
	BackWall->SetRelativeLocation(FVector(0.f, 99.f, 0.f));

	// Left Wall
	UBoxComponent* LeftWall = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWall"));
	LeftWall->SetupAttachment(RootComponent);
	LeftWall->InitBoxExtent(FVector(1.f, 100.f, 100.f));  
	LeftWall->SetRelativeLocation(FVector(-99.f, 0.f, 0.f)); 

	// Right Wall
	UBoxComponent* RightWall = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(RootComponent);
	RightWall->InitBoxExtent(FVector(1.f, 100.f, 100.f));
	RightWall->SetRelativeLocation(FVector(99.f, 0.f, 0.f));

	// Ceiling
	UBoxComponent* CeilingBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("CeilingBounds"));
	CeilingBounds->SetupAttachment(RootComponent);
	CeilingBounds->InitBoxExtent(FVector(100.f, 100.f, 1.f));
	CeilingBounds->SetRelativeLocation(FVector(0.f, 0.f, 99.f));

	// Common settings for all bounds components (make inivisible and add collision)
	TArray<UBoxComponent*> BoundsComponents = { FrontWall, BackWall, LeftWall, RightWall, CeilingBounds }; // FloorBounds, 
	for (UBoxComponent* Bounds : BoundsComponents)
	{
		Bounds->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Bounds->SetCollisionResponseToAllChannels(ECR_Block);
		Bounds->SetGenerateOverlapEvents(false);
		Bounds->SetHiddenInGame(true);
		Bounds->SetVisibility(false);
	}
}

void ADicePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bHasSettled)
	{
		bool bAllStationary = true;
		for (AActor* Dice : DiceActors)
		{
			UStaticMeshComponent* MeshComp = Dice->FindComponentByClass<UStaticMeshComponent>();

			if (!MeshComp)
			{
				UE_LOG(LogTemp, Error, TEXT("Static Mesh missing on the Dice Actor"));
				continue;
			}

			FVector LinVel = MeshComp->GetPhysicsLinearVelocity();
			FVector AngVel = MeshComp->GetPhysicsAngularVelocityInRadians();

			bAllStationary &= (LinVel.Size() < 5.f && AngVel.Size() < 5.f); // Check if the velocities are below the threshold

			if (!bAllStationary) break; // Break early if there is a dice that didn't settle yet
		}


		if (bAllStationary)
		{
			TimeStationary += DeltaTime;

			if (TimeStationary > 0.5f)
			{
				bHasSettled = true;
				SaveFaces();
			}
		}
		else
		{
			TimeStationary = 0.f;
		}
	}

	/* Checks if this player is playing currently and if he is waiting */
	if (bIsPlaying && bIsWaiting) 
	{
		if (GameMode->ReadyToPlay())
		{
			ActiveFaceWidget->SetIsEnabled(true);
			bIsWaiting = false;
		}
	}
}

void ADicePlayer::RemoveDice()
{
	if (DiceActors.Num() == 0 || DiceRolls.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("One or both arrays are empty."));
		return;
	}

	// Get the last actor
	AActor* LastActor = DiceActors.Last();

	// Remove the last elements from both arrays
	DiceActors.Pop();
	DiceRolls.Pop();

	// Destroy the dice
	LastActor->Destroy();
}

bool ADicePlayer::IsWithinBounds(AActor* Dice) const
{
	FVector Location = Dice->GetActorLocation();
	FVector Center = GetActorLocation();
	FVector Extent(105.f, 105.f, 0.f);

	FVector Min = Center - Extent;
	FVector Max = Center + Extent;

	return (Location.X >= Min.X && Location.X <= Max.X &&
		Location.Y >= Min.Y && Location.Y <= Max.Y);
}

void ADicePlayer::SaveFaces()
{
	int32 idx = 0;
	for (AActor* Dice : DiceActors)
	{
		bool ReRoll = false;

		UStaticMeshComponent* MeshComp = Dice->FindComponentByClass<UStaticMeshComponent>();
		FTransform MeshTransform = MeshComp->GetComponentTransform();
		int32 FaceValue = 0;

		if (!IsWithinBounds(Dice)) // TODO: Respawn if dice not within bounds
		{
			FVector NewLocation = GetActorLocation();
			NewLocation.Z = 50.f; // Raise into the air
			Dice->SetActorLocation(NewLocation);
			MeshComp->SetWorldLocation(NewLocation);
			ReRoll = true;
		} 
		else if (MeshComp->GetComponentLocation().Z > 20.f) // TODO: Checks if the Dice lands on top of another
		{
			ReRoll = true;
		}
		else
		{
			float MaxDot = -1.0f;
			for (const TPair<FVector, int32>& Direction : DiceDirectionsMap)
			{
				// Transform local face normal to world space
				FVector WorldFaceNormal = MeshTransform.TransformVector(Direction.Key);

				// Compare to world up (0, 0, 1)
				float Dot = FVector::DotProduct(WorldFaceNormal, FVector::UpVector);
				if (Dot > MaxDot)
				{
					MaxDot = Dot;
					FaceValue = Direction.Value;
				}
			}

			/* If not upright, reroll the dice */
			if (MaxDot < 0.9f) ReRoll = true;
		}

		if (ReRoll)
		{
			bHasSettled = false;
			RollOneDice(Dice);
		}
		
		DiceRolls[idx] = FaceValue;
		idx++;
	}
}
		

void ADicePlayer::PlayerSetup(int32 NewPlayerID)
{
	// Set the player ID, and initialize the number of cubes
	PlayerID = NewPlayerID;
	bIsPlaying = false;

	// Spawn and Roll Initial Dice
	SpawnDice();
	RollDice();
}

void ADicePlayer::SpawnDice()
{
	// Spawn the die depending on how many the player has
	for (int32 DiceIdx = 0; DiceIdx < InitDiceCount; DiceIdx++)
	{
		/* Generate a random dice face */
		auto RandomFace = FMath::RandRange(1, 6);

		/* Spawns the Player Actor and sets it up */
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Y -= 50.f - (DiceIdx * 25.f); // Dice offset
		SpawnLocation.Z = 10.f; // Raise onto the ground
		FActorSpawnParameters Params;
		Params.Owner = this;
		AActor* NewDice = GetWorld()->SpawnActor<AActor>(
			DiceClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			Params
		);

		DiceRolls.Add(RandomFace);
		DiceActors.Add(NewDice);
	}
}

void ADicePlayer::RollOneDice(AActor* Dice)
{
	UStaticMeshComponent* MeshComp = Dice->FindComponentByClass<UStaticMeshComponent>();

	if (!MeshComp)
	{
		UE_LOG(LogTemp, Error, TEXT("Static Mesh missing on the Dice Actor"));
		return;
	}

	FVector Impulse(
		FMath::RandRange(-25.f, 25.f),
		FMath::RandRange(-25.f, 25.f),
		FMath::RandRange(250.f, 500.f)
	);

	FVector Torque(
		FMath::RandRange(-5000.f, 5000.f),
		FMath::RandRange(-5000.f, 5000.f),
		FMath::RandRange(-5000.f, 5000.f)
	);

	MeshComp->AddImpulse(Impulse, NAME_None, true);
	MeshComp->AddTorqueInRadians(Torque, NAME_None, true);
}

void ADicePlayer::RollDice()
{
	bHasSettled = false;
	TimeStationary = 0.f;

	for (AActor* Dice : DiceActors)
	{
		RollOneDice(Dice);
	}
}

FRotator ADicePlayer::GenerateDiceRot(int32 FaceVal)
{
	switch (FaceVal)
	{
	case 1:
		return FRotator(0.f, 0.f, 180.f);
	case 2:
		return FRotator(0.f, 0.f, 270.f);
	case 3:
		return FRotator(90.f, 90.f, 0.f);
	case 4:
		return FRotator(270.f, 0.f, 0.f);
	case 5:
		return FRotator(0.f, 0.f, 90.f);
	default:
		return FRotator::ZeroRotator;
	}
}

void ADicePlayer::StartPlayerTurn()
{
	bIsPlaying = true;
	Face = 0; // default


	// TODO: Will change for multiplayer
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController)
	{
		EnableInput(PlayerController);

		if (InputComponent)
		{
			InputComponent->ClearActionBindings();
			InputComponent->BindAction("SubmitBet", IE_Pressed, this, &ADicePlayer::SubmitBet);
			InputComponent->BindAction("Challenge", IE_Pressed, this, &ADicePlayer::ChallengeBet);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ADicePlayer: InputComponent is null after EnableInput."));
		}

		OnOpenBetUI();
	}
}

/** Callback to show UI for Bidding **/
void ADicePlayer::OnOpenBetUI()
{	
	// Wait for your turn
	if (!bIsPlaying) return;

	// Check if the Widget Class was set
	if (!FaceSelectionWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("FaceSelectionWidgetClass not set on %s"), *GetName());
		return;
	}

	// Check if already open
	if (!ActiveFaceWidget)
	{
		// TODO: Changes for multiplayer
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (!PlayerController) return;

		PlayerController->bShowMouseCursor = true;

		ActiveFaceWidget = CreateWidget<UFaceSelectionWidget>(PlayerController, FaceSelectionWidgetClass);
		GameMode->SetSlider(ActiveFaceWidget);
		
		if (ActiveFaceWidget)
		{
			ActiveFaceWidget->OnFaceChosen.AddDynamic(this, &ADicePlayer::HandleFaceChosen);
			ActiveFaceWidget->AddToViewport();

			ActiveFaceWidget->SetIsEnabled(false); // Currently disabled until all the player dice have settled
			bIsWaiting = true;
		}
	}
}

void ADicePlayer::HandleFaceChosen(int32 FaceValue)
{
	Face = FaceValue;
}

void ADicePlayer::ChallengeBet()
{
	// Close the UI
	OnCloseBetUI();

	// Submit the challenge to the Game Mode
	GameMode->SubmitChallenge(PlayerID);
}

void ADicePlayer::SubmitBet()
{
	if (!ActiveFaceWidget || Face == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Need to make a selection in the UI"));
		return;
	}

	// TODO: Verify the bet (dynamically before the player chooses)
	FBet PlayerBet = FBet(
		ActiveFaceWidget->NumOfFaces,
		Face,
		PlayerID
	);


	// Verify the bet
	bool CorrectBet = GameMode->VerifyBet(PlayerBet);
	if (!CorrectBet)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Incorrect Bet"));
		return;
	}

	// Close the UI
	OnCloseBetUI();

	// Submit the bet 
	GameMode->SubmitBet(PlayerBet);
}

void ADicePlayer::OnCloseBetUI()
{
	ActiveFaceWidget->RemoveFromParent();
	ActiveFaceWidget = nullptr;

	// TODO: Changes in multiplayer
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		PlayerController->bShowMouseCursor = false;
	}
}