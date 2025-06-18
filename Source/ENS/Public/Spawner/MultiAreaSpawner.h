// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "Characters/Enemies/EnsEnemyBase.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MultiAreaSpawner.generated.h"

class UBoxComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogSpawners, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewWave, int, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawnerFinished);

/**
 * \brief A spawner that can have multiple spawn areas and a trigger box to activate it.
 */
UCLASS()
class ENS_API AMultiAreaSpawner : public AActor
{
    GENERATED_BODY()

public:
    explicit AMultiAreaSpawner();

    /**
     * \brief Get a random location within the bounds of the specified area.
     * \param AreaIndex Index of the area (must be in \ref SpawnAreas).
     * \return Location within the bounds of the specified area.
     */
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    FVector GetRandomLocationInSpecificArea(int32 AreaIndex) const;

    /**
     * \brief Callback function called when an enemy is destroyed.
     */
    UFUNCTION()
    void OnEnemyDestroyed();

    /// \brief Event called when a new wave of enemies is spawned. (with the wave number)
    UPROPERTY(BlueprintAssignable, Category = "Spawning")
    FOnNewWave OnNewWave;

    /// \brief Event called when all enemies of the last wave are killed.
    UPROPERTY(BlueprintAssignable, Category = "Spawning")
    FOnSpawnerFinished OnSpawnerFinished;

    /// \brief `true` if the spawner is active, `false` otherwise.
    bool bIsActive;

    /// \brief Data table containing data for spawning waves of enemies.
    UPROPERTY(EditAnywhere, Category = "Spawning")
    UDataTable* SpawnData;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaTime) override;

    /// \brief Display debug bounds in editor.
    UPROPERTY(EditAnywhere, Category = "Spawning")
    bool bShowDebugBounds = true;

#pragma region Spawn
    /// \brief Get indexes of active spawn areas. (i.e. areas that are further that \ref SpawnDistance from the player)
    [[nodiscard]] TArray<uint32> GetActiveSpawnAreas() const;

    /// \brief Get indexes of all areas that have an enemy to spawn according to \ref SpawnInformations.
    [[nodiscard]] TArray<uint32> GetSpawnAreas() const;
    
    /// \brief Number of spawn areas (minimum 1).
    UPROPERTY(EditAnywhere, Category = "Spawning", Meta = (ClampMin = "1"))
    int32 NumSpawnAreas = 1;

    /// \brief Default size of any box created automatically by the spawner (trigger box and spawn areas).
    UPROPERTY(EditAnywhere, Category = "Spawning")
    FVector SpawnAreaSize = FVector(200.0f, 200.0f, 200.0f);

    /// \brief Distance from the player for a spawn area to be deactivated. (i.e. if the player is closer than this distance, the area is not active)
    UPROPERTY(EditAnywhere, Category = "Spawning")
    float SpawnDistance = 500.0f;

    /// \brief Array containing all box components representing spawn areas of the spawner.
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawning")
    TArray<UBoxComponent*> SpawnAreas;
#pragma endregion

#pragma region TriggerBox
    /// \brief Whether the Spawner has a trigger box or not to activate it.
    UPROPERTY(EditAnywhere, Category = "Spawning")
    bool bHasTrigger;

    /// \brief Trigger box used to activate the spawner.
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawning")
    UBoxComponent* TriggerBox;
#pragma endregion

private:
    /// \brief Map containing the enemies to spawn in each spawn area.
    TMap<int32, TArray<TSubclassOf<AEnsEnemyBase>>> SpawnInformations;
    
    UFUNCTION()
    void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /// \brief Update the spawn areas of the spawner.
    void UpdateSpawnAreas();

    /// \brief Update the trigger box of the spawner.
    void UpdateTrigger();

    /// \brief Spawns the wave of enemies specified by `WaveNumber + 1`.
    void SpawnWave();

    /**
     * \brief Spawn a specific number of enemies inside the specified spawn area.
     * \param ActorToSpawn Class of the enemy to spawn.
     * \param AreaIndex Index of the spawn area.
     * \param Count Number of enemies to spawn.
     */
    void SpawnEnemies(const TSubclassOf<AEnsEnemyBase>& ActorToSpawn, int32 AreaIndex, int32 Count);

    /// \brief Spawns enemies one enemy in a random number of spawn areas available.
    void SpawnEnemiesInRandomAreas();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /// \brief The global wave number across all spawners.
    inline static int GlobalWaveNumber = 0;

    /// \brief Delay between each spawn of enemy
    float SpawnDelay = 1.5f;
    
    /// \brief Timer until the next wave of enemies is spawned.
    float Timer = 1000.0f;

    /// \brief Timer used to spawn enemies according to \ref SpawnDelay.
    float SpawnTimer = 0.0f;
    
    uint16 WaveNumber = 0;
    uint32 CurrentEnemies = 0;
    
    
};


