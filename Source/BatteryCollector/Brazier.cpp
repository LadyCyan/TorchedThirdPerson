// Fill out your copyright notice in the Description page of Project Settings.


#include "Brazier.h"
#include "BatteryCollector.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "particles/ParticleEmitter.h"

// Sets default values
ABrazier::ABrazier()
{
	GetMesh()->SetSimulatePhysics(true);

	Fire = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Fire_Partilce_Brazer"));
	Fire->SetupAttachment(RootComponent);
	//Create LightSphere
	BrazierSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BrazierSphere"));
	BrazierSphere->SetupAttachment(RootComponent);

	if (IsActive() == false)
	{
		BrazierSphere->SetSphereRadius(100.0f);
	}
}

void ABrazier::WasTriggered_Implementation() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("We got here!"));
	//Use the base pickup behavior
	Super::WasTriggered_Implementation();

	bIsActive = true;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, IsActive() ? "True" : "False");

	if (IsActive() == false) 
	{
		BrazierSphere->SetSphereRadius(100.0f);
	}
	else
	{
		BrazierSphere->SetSphereRadius(1000.0f);
	}

}

