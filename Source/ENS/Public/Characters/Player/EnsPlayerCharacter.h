// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "Characters/EnsCharacterBase.h"

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"

#include "EnsPlayerCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerCharacter, Log, All);

/// \brief Delegate used when a player reaches a new level.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int64, NewLevel);

/**
 * \brief The player character in the game.
 *
 * It provides basic functionalities and integrates input, movement, camera, abilities...
 * It is the default character in the \ref EnsGameMode.
 */
UCLASS()
class ENS_API AEnsPlayerCharacter : public AEnsCharacterBase, public IGenericTeamAgentInterface
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    /// \brief Constructs a character and setups its components.
    explicit AEnsPlayerCharacter();

    /// \copydoc AEnsCharacterBase::BaseAttack
    UFUNCTION(BlueprintCallable)
    virtual void BaseAttack() override;

    /// \brief Called when the player attack an enemy
    UFUNCTION(BlueprintImplementableEvent)
    void OnBaseAttack();

    /// \copydoc AEnsCharacterBase::OnDeath
    virtual void OnDeath(AEnsCharacterBase* SourceActor) override;

    /// \brief Checks if the player is currently dead (i.e. is in death animation).
    [[nodiscard]] bool IsDead() const { return bIsDead; }

    /**
     * \brief Adds \p Amount to the player experience and level-up if required.
     * \param Amount The number of experience points to add.
     */
    UFUNCTION(BlueprintCallable)
    void IncreaseXp(int64 Amount);

    /// \brief Gets the current level of the player.
    UFUNCTION(BlueprintCallable)
    [[nodiscard]] int64 GetCurrentLevel() const;

    /// \brief Gets the maximum level of the player.
    UFUNCTION(BlueprintCallable)
    [[nodiscard]] int64 GetMaxLevel() const;

    /// \brief Called when the player levels up (with the new level).
    UPROPERTY(BlueprintAssignable)
    FOnLevelUp OnLevelUp;

    /// \brief Gets the inventory component of the player.
    [[nodiscard]] class UInventory* GetInventoryComponent() const;

    /**
     * \brief Sets the team the current player is in.
     * \param NewTeamID The id of the new team.
     */
    UFUNCTION(BlueprintCallable)
    virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

    /// \brief Gets the id of the team the player is currently in.
    UFUNCTION(BlueprintCallable)
    virtual FGenericTeamId GetGenericTeamId() const override;

    /// \brief Vector used as forward direction.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
    FVector ForwardVector;

    /// \brief Vector used as right direction.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
    FVector RightVector;

    /**
     * \brief Move player using \ref ForwardVector and \ref RightVector scaled by the values of \p MovementVector.
     * \param MovementVector Scale of movement.
     */
    void Move(const FVector2D& MovementVector);

    /// \brief Last vector used for movement, used for dash direction.
    UPROPERTY(BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
    FVector2D LastMovementVector;

private:
    FGenericTeamId TeamId;

    /// \brief The component for managing inventory
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    class UInventory* Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraShakeSourceComponent* CameraShakeSourceComponent;

    /// \brief The attribute set to hold potions data.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
    class UEnsPotionAttributeSet* PotionAttributeSet;

#pragma region BaseAttck
    /// \brief Whether the player is attacking or not.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Attack", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking = false;

    /// \brief Whether an attack is buffered or not.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Attack", meta = (AllowPrivateAccess = "true"))
    bool bIsAttackBuffered = false;

    /// \brief Whether an attack can be buffered or not. (e.g. between collision is spawned and destroyed)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Attack", meta = (AllowPrivateAccess = "true"))
    bool bCanBuffer = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Attack", meta = (AllowPrivateAccess = "true"))
    bool bIsReattacking = false;
#pragma endregion

#pragma region Levels
    /// \brief The current player experience.
    UPROPERTY(BlueprintReadOnly, Category = "Levels", meta = (AllowPrivateAccess = "true"))
    int64 CurrentExperience = 0;

    /// \brief The number of points at which the level `index + 1` is reached.
    UPROPERTY(EditDefaultsOnly, Category = "Levels", meta = (AllowPrivateAccess = "true"))
    TArray<int64> ExperienceLevelTransitions;
#pragma endregion

    bool bIsDead = false;
};
