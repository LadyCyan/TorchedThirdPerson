// Copyright Epic Games, Inc. All Rights Reserved.

#include "BatteryCollectorCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Pickup.h"
#include "Components/SphereComponent.h"
#include "BatteryPickup.h"
#include "Brazier.h"

//////////////////////////////////////////////////////////////////////////
// ABatteryCollectorCharacter

ABatteryCollectorCharacter::ABatteryCollectorCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//Create LightSphere
	LightSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LightSphere"));
	LightSphere -> SetupAttachment(RootComponent);
	LightSphere->SetSphereRadius(200.f);
	LightSphere->SetCollisionProfileName(TEXT("Trigger"));

	LightSphere->OnComponentBeginOverlap.AddDynamic(this, &ABatteryCollectorCharacter::OnOverlapBegin);
	LightSphere->OnComponentEndOverlap.AddDynamic(this, &ABatteryCollectorCharacter::OnOverlapEnd);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	
	//Set a base power level for the character
	InitialPower = 200.f;
	CharacterPower = InitialPower;

	//set the dependence of the speed on the power level
	SpeedFactor = 0.75f;
	BaseSpeed = 10.0f;

	//Decaying trigger
	bIsDecaying = 0;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABatteryCollectorCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	InputComponent->BindAction("Trigger", IE_Pressed, this, &ABatteryCollectorCharacter::CollectPickups);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABatteryCollectorCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABatteryCollectorCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABatteryCollectorCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABatteryCollectorCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ABatteryCollectorCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ABatteryCollectorCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ABatteryCollectorCharacter::OnResetVR);
}


void ABatteryCollectorCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ABatteryCollectorCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ABatteryCollectorCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ABatteryCollectorCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABatteryCollectorCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABatteryCollectorCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABatteryCollectorCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

bool ABatteryCollectorCharacter::IsDecaying()
{
	return bIsDecaying == 0;
}

bool ABatteryCollectorCharacter::TorchLitCount() {
	return bTorchLitCount == 4;
}

void ABatteryCollectorCharacter::CollectPickups() {
	//Get all overlapping actors and store them in an array
	TArray<AActor*> CollectedActors;
	LightSphere -> GetOverlappingActors(CollectedActors);

	FString IntAsString = FString::FromInt(CollectedActors.Num());

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, IntAsString);

	//keep track of the collected power
	float CollectedPower = 0;

	//For each actor we collected
	for (int32 iCollected = 0; iCollected < CollectedActors.Num(); ++iCollected)
	{
		//Cast the actor to APickup
		APickup* const TestPickup = Cast<APickup>(CollectedActors[iCollected]);

		//If the cast is successful and the pickup is valid and active
		if (TestPickup && TestPickup -> IsActive() == false) {

			//call the pickups WasTriggered function
			TestPickup->WasTriggered();

			//check to see if a brazier
			ABrazier* const TestBrazier = Cast<ABrazier>(TestPickup);

			CharacterPower = GetInitialPower() * 1.25f;

			if (TestBrazier) {
				//increase the collected power
				bTorchLitCount++;
				FString TorchAsString = FString::FromInt(bTorchLitCount);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("bTorchLitCount = ") + TorchAsString);
			}
			//Deactivate the pickup
			TestPickup->SetActive(true);
		}
		else if (TestPickup)
		{
			CharacterPower = GetInitialPower() * 1.25f;
		}
	}

	if (CollectedPower > 0) {
		UpdatePower(CollectedPower);
	}
}

//reports starting power
float ABatteryCollectorCharacter::GetInitialPower() {
	return InitialPower;
}

//reports current power
float ABatteryCollectorCharacter::GetCurrentPower() {
	return CharacterPower;
}

//called whenver power is increased of decreased
void ABatteryCollectorCharacter::UpdatePower(float PowerChange) {
	//change power
	CharacterPower = CharacterPower + PowerChange;
	//change speed based on power
	//GetCharacterMovement()->MaxWalkSpeed = BaseSpeed + SpeedFactor * CharacterPower;

//call visual effect
	PowerChangeEffect();
}

void ABatteryCollectorCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32
	OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap Begin Method Called"));

	ABrazier* const TestBrazier = Cast<ABrazier>(OtherActor);

	USphereComponent* const TestComponent = Cast<USphereComponent>(OtherComp);

	if (TestBrazier && TestComponent) 
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap Begin"));
		}
		CharacterPower = InitialPower * 1.25f;
		PowerChangeEffect();
		bIsDecaying++;

		FString IntAsString = FString::FromInt(bIsDecaying);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("bIsDecaying = ") + IntAsString);
	}
}


void ABatteryCollectorCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32
	OtherBodyIndex) 
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap End Method Called"));
	ABrazier* const TestBrazier = Cast<ABrazier>(OtherActor);
	USphereComponent* const TestComponent = Cast<USphereComponent>(OtherComp);

	if (TestBrazier && TestComponent)
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap End"));
		}

		bIsDecaying--;

		FString IntAsString = FString::FromInt(bIsDecaying);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,TEXT("bIsDecaying = ") + IntAsString);
	}
}