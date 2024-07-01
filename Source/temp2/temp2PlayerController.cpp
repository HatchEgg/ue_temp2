// Copyright Epic Games, Inc. All Rights Reserved.

#include "temp2PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "temp2Character.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

Atemp2PlayerController::Atemp2PlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void Atemp2PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("CreateHome", IE_Pressed, this, &Atemp2PlayerController::OnCreateHome);
	InputComponent->BindAxis("PlaceActor", this, &Atemp2PlayerController::OnPlaceActor);
	InputComponent->BindAction("PlaceActorFinish", IE_Pressed, this, &Atemp2PlayerController::OnPlaceActorFinish);

}

void Atemp2PlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

void Atemp2PlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void Atemp2PlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void Atemp2PlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void Atemp2PlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void Atemp2PlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void Atemp2PlayerController::OnCreateHome()
{
    UWorld* world = this->GetWorld();
	if (world) {
		this->homeActor = world->SpawnActor(AHomeActor::StaticClass(), &FVector(1023,1864,50));
	}
}

void Atemp2PlayerController::OnPlaceActor(float axis)
{
}

void Atemp2PlayerController::OnPlaceActorFinish()
{
}
