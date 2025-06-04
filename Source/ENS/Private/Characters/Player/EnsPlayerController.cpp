// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "Characters/Player/EnsPlayerController.h"
#include "Characters/Player/EnsPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AEnsPlayerController::AEnsPlayerController()
{
	// Show mouse cursor, so we can click to move
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AEnsPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AEnsPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	auto* PlayerCharacter = Cast<AEnsPlayerCharacter>(GetCharacter());
	if (!PlayerCharacter)
	{
		UE_LOG(LogPlayerController, Error, TEXT("Failed to get the player character for setting actor abilities."));
		return;
	}

	PlayerCharacter->GetAbilitySystemComponent()->InitAbilityActorInfo(PlayerCharacter, InPawn);

	PlayerCharacter->OnDeathDelegate.AddUniqueDynamic(this, &AEnsPlayerController::OnDeath);
}

void AEnsPlayerController::OnDeath_Implementation()
{
	if (Lives > 1)
	{
		Lives--;
		LostLife();
	} else
		Restart();
}

void AEnsPlayerController::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update mouse position (if in game)
	if (GetWorld() && (GetWorld()->WorldType == EWorldType::Game || GetWorld()->WorldType == EWorldType::GamePreview || GetWorld()->WorldType == EWorldType::PIE))
	{
		// Get the current viewport size
		FVector2D ViewportSize;
		if (!GEngine && !GEngine->GameViewport)
		{
			UE_LOG(LogPlayerController, Error, TEXT("Failed to get the viewport size. No engine or viewport."));
			return;
		}
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		GetMousePosition(RelativeMousePosition.X, RelativeMousePosition.Y);
		RelativeMousePosition /= ViewportSize;
	}
}

void AEnsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		Subsystem->AddMappingContext(DefaultMappingContext, 0);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogPlayerController, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
		return;
	}

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEnsPlayerController::Move);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AEnsPlayerController::Attack);
}

void AEnsPlayerController::Move(const FInputActionValue& Value)
{
	auto* PlayerCharacter = Cast<AEnsPlayerCharacter>(GetCharacter());
	PlayerCharacter->Move(Value.Get<FVector2D>());
}

void AEnsPlayerController::Attack()
{
	Cast<AEnsPlayerCharacter>(GetCharacter())->BaseAttack();
}
