// Fill out your copyright notice in the Description page of Project Settings.


#include "DicePlayer.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SpotLightComponent.h"

// Sets default values
ADicePlayer::ADicePlayer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	this->PlayerID = -1;

	// Root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Camera setup
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->SetRelativeLocation(FVector(-110.f, 0.f, 140.f));
	PlayerCamera->SetWorldRotation(FRotator(-40.f, 0.f, 0.f));

	// Spotlight setup
	PlayerLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("PlayerLight"));
	PlayerLight->SetupAttachment(RootComponent);
	PlayerLight->SetRelativeLocation(FVector(0.f, 0.f, 250.f));
	PlayerLight->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	PlayerLight->Intensity = 2000.f;
}

void ADicePlayer::PlayerSetup(int32 NewPlayerID)
{
	// Set the player ID, and initialize the number of cubes
	this->PlayerID = NewPlayerID;
	this->DiceCount = 5;

	// Roll Initial Dice
	RollDice();
}

void ADicePlayer::RollDice()
{
	DiceRolls.Empty();
	for (int32 DiceIdx = 0; DiceIdx < DiceCount; DiceIdx++)
	{
		/* Generate a random dice face */
		auto Face = FMath::RandRange(1, 6);

		/* Spawns the Player Actor and sets it up */
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Y -= 50.f - (DiceIdx * 25.f); // Dice offset
		SpawnLocation.Z = 10.f; // Raise onto the ground
		FActorSpawnParameters Params;
		Params.Owner = this;
		AActor* NewDice = GetWorld()->SpawnActor<AActor>(
			DiceClass,
			SpawnLocation,
			GenerateDiceRot(Face),
			Params
		);

		DiceRolls.Add(Face);
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

// Called when the game starts or when spawned
void ADicePlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

