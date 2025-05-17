// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"

#include "EnsPlayerController.generated.h"

/// \brief The custom collision channels used in the game.
namespace ECustomCollisionChannel
{
    /// \brief The default channel for the player character.
    static constexpr ECollisionChannel ECC_Floor = ECC_GameTraceChannel1;
    /// \brief The channel for the player character's abilities.
    static constexpr ECollisionChannel ECC_Interactable = ECC_GameTraceChannel2;
    /// \brief The channel for the enemies in the game.
    static constexpr ECollisionChannel ECC_Player = ECC_GameTraceChannel3;
} // namespace ECustomCollisionChannel

/**
 * \brief The controller used in \ref AEnsPlayerCharacter.
 */
UCLASS()
class ENS_API AEnsPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    explicit AEnsPlayerController();

    /**
     * \brief Called every frame.
     * \param DeltaSeconds The time since the last frame in seconds.
     */
    virtual void Tick(float DeltaSeconds) override;

    /// \brief Returns the current mouse position relative to the viewport or the last known position.
    FVector2D GetRelativeMousePosition() const { return RelativeMousePosition; }

    /// \brief Player lives count
    UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
    int Lives = 1;

    /// \brief Interaction radius on click
    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    float InteractionRadius = 50.0f;

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    /// \brief Method bound to Player death
    UFUNCTION(BlueprintNativeEvent)
    void OnDeath();

    /// \brief Called by OnDeath() if player has no lives left
    UFUNCTION(BlueprintImplementableEvent)
    void Restart();

    /// \brief Called by OnDeath() if player has lives remaining
    UFUNCTION(BlueprintImplementableEvent)
    void LostLife();

    /// \brief Binds the inputs to the corresponding player actions.
    virtual void SetupInputComponent() override;

private:
    /// \brief The default mapping context.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputMappingContext* DefaultMappingContext = nullptr;

    /// \brief The current mouse position relative to the viewport.
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Data, meta = (AllowPrivateAccess = "true"))
    FVector2D RelativeMousePosition = FVector2D::ZeroVector;

    /// \brief The action to attack.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* AttackAction;

    /// \brief The action to move the player.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* MoveAction;

    /// \brief Called when \ref MoveAction is triggered. Used to move the player character.
    void Move(const FInputActionValue& Value);

    /// \brief Called when \ref AttackAction is pressed; Trigger the attack of the player character.
    void Attack();
};
