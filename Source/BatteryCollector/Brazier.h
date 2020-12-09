// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "GameFramework/Actor.h"
#include "Brazier.generated.h"

UCLASS()
class BATTERYCOLLECTOR_API ABrazier : public APickup
{
	GENERATED_BODY()
	
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* BrazierSphere;

public:	
	// Sets default values for this actor's properties
	ABrazier();

	/*Override the WasTriggered function-use Implementation because it's a blueprint native event*/
	void WasTriggered_Implementation() override;



	FORCEINLINE class USphereComponent* GetBrazierSphere() const { return BrazierSphere; }
};
