// Fill out your copyright notice in the Description page of Project Settings.


#include "MPDicePlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SpotLightComponent.h"
#include "MPDiceGameState.h"
#include "MPDicePlayerState.h"
#include "Net/UnrealNetwork.h"

// TODO: Sets default values
AMPDicePlayer::AMPDicePlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Camera setup
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(Root);
	PlayerCamera->SetRelativeLocation(FVector(-125.f, 0.f, 140.f));
	PlayerCamera->SetWorldRotation(FRotator(-40.f, 0.f, 0.f));

	// Spotlight setup
	PlayerLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("PlayerLight"));
	PlayerLight->SetupAttachment(Root);
	PlayerLight->SetRelativeLocation(FVector(0.f, 0.f, 250.f));
	PlayerLight->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	PlayerLight->Intensity = 2000.f;

	// Front Wall 
	UBoxComponent* FrontWall = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontWall"));
	FrontWall->SetupAttachment(Root);
	FrontWall->InitBoxExtent(FVector(100.f, 1.f, 100.f));
	FrontWall->SetRelativeLocation(FVector(0.f, -99.f, 0.f));

	// Back Wall
	UBoxComponent* BackWall = CreateDefaultSubobject<UBoxComponent>(TEXT("BackWall"));
	BackWall->SetupAttachment(Root);
	BackWall->InitBoxExtent(FVector(100.f, 1.f, 100.f));
	BackWall->SetRelativeLocation(FVector(0.f, 99.f, 0.f));

	// Left Wall
	UBoxComponent* LeftWall = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWall"));
	LeftWall->SetupAttachment(Root);
	LeftWall->InitBoxExtent(FVector(1.f, 100.f, 100.f));
	LeftWall->SetRelativeLocation(FVector(-99.f, 0.f, 0.f));

	// Right Wall
	UBoxComponent* RightWall = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(Root);
	RightWall->InitBoxExtent(FVector(1.f, 100.f, 100.f));
	RightWall->SetRelativeLocation(FVector(99.f, 0.f, 0.f));

	// Ceiling
	UBoxComponent* CeilingBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("CeilingBounds"));
	CeilingBounds->SetupAttachment(Root);
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

void AMPDicePlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPDicePlayer, DiceVisuals);
}

void AMPDicePlayer::StartDestructionAnimation()
{
	if (DiceVisuals.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No dice to remove or invalid count"));
		OnDiceDestructionComplete.Broadcast();
		return;
	}

	// Get the last actor
	DiceDestroy = DiceVisuals.Pop();

	// Save the Materials
	Materials.Empty();
	if (UStaticMeshComponent* Mesh = DiceDestroy->GetComponentByClass<UStaticMeshComponent>())
	{
		const int32 MaterialCount = Mesh->GetNumMaterials();
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			if (UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(i)) 
				Materials.Add(Mat);
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&AMPDicePlayer::PlayDestructionAnimation,
		0.016f,
		true
	);
}

void AMPDicePlayer::PlayDestructionAnimation()
{
	if (!DiceDestroy)
	{
		UE_LOG(LogTemp, Error, TEXT("[Player] Skipping Destruction - no dice to destroy"));
		GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
		OnDiceDestructionComplete.Broadcast();
		return;
	}

	DissolveVal += DissolveStep;

	if (FMath::IsNearlyEqual(DissolveVal, 0.75f, 0.01f))
	{
		// Destroy the dice
		DiceDestroy->Destroy();
		DiceDestroy = nullptr;
		DissolveVal = -1.f;

		GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);

		UE_LOG(LogTemp, Error, TEXT("Dice Destroy Broadcasting"));
		OnDiceDestructionComplete.Broadcast();

		return;
	}

	// Update the dissolve values
	for (auto* Mat : Materials) // 3D object
	{
		if (!Mat) continue;
		Mat->SetScalarParameterValue(FName("Dissolve"), DissolveVal);
	}
}

void AMPDicePlayer::UpdateDiceCounts(int32 DiceCount)
{
	UE_LOG(LogTemp, Warning, TEXT("Updating Dice visuals (from %d to %d)"), DiceCount, DiceVisuals.Num());

	// TODO: Animation
	/* Remove Extra Dice */
	while (DiceVisuals.Num() > DiceCount)
	{
		AActor* Dice = DiceVisuals.Pop();
		if (Dice)
		{
			Dice->Destroy();
			UE_LOG(LogTemp, Warning, TEXT("Dice Destroyed"));
		}
	}

	/* Add Missing Dice */
	while (DiceVisuals.Num() < DiceCount)
	{
		const int32 DiceIdx = DiceVisuals.Num();

		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Y -= 50.f - (DiceIdx * 25.f);
		SpawnLocation.Z = 10.f;

		FActorSpawnParameters Params;
		Params.Owner = this;

		AActor* NewDice = GetWorld()->SpawnActor<AActor>(
			DiceClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			Params
		);

		if (NewDice) DiceVisuals.Add(NewDice);
	}
}

void AMPDicePlayer::UpdateDiceVisuals(const TArray<int32>& DiceVals)
{
	// TODO: Animation
	

	if (DiceVals.Num() != DiceVisuals.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Update Dice Visuals"));
		return;
	}

	for (int32 DiceIdx = 0; DiceIdx < DiceVals.Num(); DiceIdx++)
	{
		if (AActor* DiceActor = DiceVisuals[DiceIdx])
		{
			UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(
				DiceActor->GetDefaultSubobjectByName(TEXT("Dice"))
			);

			if (!MeshComp)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to Update Dice Visuals (missing mesh component `Dice`)"));
				return;
			}

			FRotator DiceRotation = GetDiceRotation(DiceVals[DiceIdx]);
			MeshComp->SetRelativeRotation(DiceRotation);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Updating Dice Visuals"));
}

void AMPDicePlayer::HighlightDice(TArray<int32>& DiceVals, int32 FaceVal)
{
	if (DiceVisuals.Num() != DiceVals.Num()) return;

	for (int DiceIdx = 0; DiceIdx < DiceVisuals.Num(); DiceIdx++)
	{
		float StencilVal = (FaceVal == DiceVals[DiceIdx]) ? 2.f : 1.f;

		UStaticMeshComponent* MeshComp = DiceVisuals[DiceIdx]->FindComponentByClass<UStaticMeshComponent>();
		MeshComp->SetCustomDepthStencilValue(StencilVal);
	}
}

void AMPDicePlayer::UndoHighlightDice()
{
	for (AActor* Dice : DiceVisuals)
	{
		UStaticMeshComponent* MeshComp = Dice->FindComponentByClass<UStaticMeshComponent>();
		MeshComp->SetCustomDepthStencilValue(1.f);
	}
}

FRotator AMPDicePlayer::GetDiceRotation(int32 FaceVal) const
{
	switch (FaceVal)
	{
		case 1: return FRotator(0.f, 0.f, 180.f);
		case 2: return FRotator(0.f, 0.f, 270.f);
		case 3: return FRotator(90.f, 90.f, 0.f);
		case 4: return FRotator(270.f, 0.f, 0.f);
		case 5: return FRotator(0.f, 0.f, 90.f);
		default: return FRotator::ZeroRotator;
	}
}