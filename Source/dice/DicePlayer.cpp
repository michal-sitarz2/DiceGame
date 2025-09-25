// Fill out your copyright notice in the Description page of Project Settings.


#include "DicePlayer.h"
#include "Camera/CameraComponent.h"
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
	this->PlayerID = NewPlayerID;
	this->DiceCount = 5;
}

// Called when the game starts or when spawned
void ADicePlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADicePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

