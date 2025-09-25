// Fill out your copyright notice in the Description page of Project Settings.


#include "DiceGameMode.h"
#include "DicePlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"


// TODO: Creates one player for now with PlayerID = 0
ADiceGameMode::ADiceGameMode() 
{

}

void ADiceGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!PlayerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerClass is null in DiceGameMode!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	/* Spawns the Player Actor and sets it up */
	FVector SpawnLocation = FVector::ZeroVector;
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
		NewPlayer->PlayerSetup(0);
	}

	/* Switch to the player's camera */
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->SetViewTargetWithBlend(NewPlayer, 0.f);
	}
}