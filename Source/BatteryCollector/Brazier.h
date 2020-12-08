// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Brazier.generated.h"

UCLASS()
class BATTERYCOLLECTOR_API ABrazier : public AActor
{
	GENERATED_BODY()
	
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* BrazierSphere;

public:	
	// Sets default values for this actor's properties
	ABrazier();

	//bool for off and on
	bool Light;

	UFUNCTION(VisibleAnywhere, BlueprintPure, Category = "Brazier")
		bool LightBrazier();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE class USphereComponent* GetBrazierSphere() const { return BrazierSphere; }
};
