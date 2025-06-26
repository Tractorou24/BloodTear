// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "GameFramework/CheatManager.h"

#include "MiscCheats.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    Imp = 0,
    SkullFire = 1,
    SkullKamikaze = 2
};

/**
 * \brief The cheat manager for the project.
 */
UCLASS()
class ENS_API UMiscCheats : public UCheatManager
{
    GENERATED_BODY()

#pragma region Player
    /// \brief Make the player die (i.e. loosing one life)
    UFUNCTION(Exec)
    void Die();

    /// \brief Make the player die enough times to restart the game.
    UFUNCTION(Exec)
    void DieForRespawn();

    /// \brief Applies \p Amount damage to the player.
    UFUNCTION(Exec)
    void Damage(int Amount);

    /// \brief Applies \p Amount xp to the player.
    UFUNCTION(Exec)
    void IncreaseXP(int Amount);
#pragma endregion

#pragma region AI
    /**
     * @brief Command to spawn \p Number AI in front of the player.
     * @param AIToSpawn The AI type to spawn, referring to \ref EEnemyType.
     * @param Number How many AI the command has to spawn.
     */
    UFUNCTION(Exec)
    void SpawnAI(EEnemyType AIToSpawn, uint8 Number = 1) const;
#pragma endregion AI
};
