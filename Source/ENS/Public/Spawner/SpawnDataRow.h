// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "Characters/Enemies/EnsEnemyBase.h"
#include "Containers/Map.h"
#include "CoreMinimal.h"

#include "SpawnDataRow.generated.h"

/// \brief Data for spawning an enemy.
USTRUCT(BlueprintType)
struct ENS_API FEnemyData
{
    GENERATED_BODY()

    /// \brief Enemy class to spawn.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) 
    TSubclassOf<AEnsEnemyBase> EnemyClass;

    /// \brief Amount of enemies to spawn.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount;
};

/**
 * \brief Data row of a data table containing data for spawning waves of enemies.
 */
USTRUCT()
struct ENS_API FSpawnDataRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /// \brief Enemies to spawn.
    UPROPERTY(EditAnywhere)
    TArray<FEnemyData> EnemiesToSpawn;
    
    /// \brief Duration of the wave in seconds.
    UPROPERTY(EditAnywhere, meta = (Units = "s"))
    int32 Time;

    /// \brief Delay between each spawn in seconds.
    UPROPERTY(EditAnywhere, meta = (Units = "s"))
    float SpawnDelay = 1.5f;
};
