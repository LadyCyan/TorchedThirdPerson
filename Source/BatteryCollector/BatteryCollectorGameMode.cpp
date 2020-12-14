// Copyright Epic Games, Inc. All Rights Reserved.

#include "BatteryCollectorGameMode.h"
#include "BatteryCollectorCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "BatteryCollector.h"
#include "Blueprint/UserWidget.h"

ABatteryCollectorGameMode::ABatteryCollectorGameMode()
{
	
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	//base decay rate
	DecayRate = 0.05f;
}

void ABatteryCollectorGameMode::BeginPlay() 
{
	Super::BeginPlay();

	//set the score to beat
	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		MaxPower = (MyCharacter->GetInitialPower()) * 1.25f;
	}

	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

void ABatteryCollectorGameMode::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	//check that we are using the battery collector character
	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter && MyCharacter->IsDecaying()==true && !MyCharacter->TorchLitCount() == true)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Decaying True"));
		//if the character's power is positive
		if (MyCharacter->GetCurrentPower() > 0) 
		{

			//decrease the character's power using the decay rate
			MyCharacter->UpdatePower(-DeltaTime * DecayRate * (MyCharacter->GetInitialPower()));
		}
		else {
			UGameplayStatics::OpenLevel(this, "MainMenu");
		}
	} if (MyCharacter->TorchLitCount() == true) {

		UGameplayStatics::OpenLevel(this, "WinScreen");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Game Should end here"));
	}
}


float ABatteryCollectorGameMode::GetMaxPower() const
{
	return MaxPower;
}
