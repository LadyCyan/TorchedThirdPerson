// Fill out your copyright notice in the Description page of Project Settings.


#include "Brazier.h"
#include "Components/SphereComponent.h"

// Sets default values
ABrazier::ABrazier()
{

	//Create LightSphere
	BrazierSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BrazierSphere"));
	
	BrazierSphere->SetupAttachment(RootComponent);

	Light = false;

	/*if (ABrazier::Light == false)
	{
		BrazierSphere->SetSphereRadius(200.f);
	}
	else if (ABrazier::Light == true) 
	{
		BrazierSphere->SetSphereRadius(600.f);
	}*/
}


// Called when the game starts or when spawned
void ABrazier::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABrazier::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void ABrazier::LightBrazier() 
{

}

