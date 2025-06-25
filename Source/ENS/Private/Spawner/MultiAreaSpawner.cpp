// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "Spawner/MultiAreaSpawner.h"
#include "Characters/Player/EnsPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Utils/ArrayUtils.h"
#include "Spawner/SpawnDataRow.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogSpawners);

AMultiAreaSpawner::AMultiAreaSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    GlobalWaveNumber = 0;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AMultiAreaSpawner::UpdateTrigger()
{
    if (!bHasTrigger)
    {
        TriggerBox->DestroyComponent();
        return;
    }
    TriggerBox = NewObject<UBoxComponent>(this, TEXT("TriggerBox"));
    TriggerBox->RegisterComponent();
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetCollisionProfileName(TEXT("OverlapAll"));
    TriggerBox->bSelectable = true;
    TriggerBox->SetBoxExtent(SpawnAreaSize);
    TriggerBox->SetRelativeLocation(GetActorLocation());
    TriggerBox->bIsEditorOnly = false;
    TriggerBox->ShapeColor = FColor::Blue;
}

void AMultiAreaSpawner::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor->IsA<AEnsPlayerCharacter>())
        return;

    bIsActive = true;
    TriggerBox->OnComponentBeginOverlap.Remove(this, "OnTriggerOverlapBegin");
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpawnWave();
}

void AMultiAreaSpawner::UpdateSpawnAreas()
{
    // Remove existing spawn areas
    for (UBoxComponent* Area : SpawnAreas)
    {
        ensure(Area != nullptr && "Area in SpawnArea cannot be nullptr.");
        Area->DestroyComponent();
    }

    SpawnAreas.Empty();

    // Create new spawn areas
    for (int32 i = 0; i < NumSpawnAreas; ++i)
    {
        FString AreaName = FString::Printf(TEXT("SpawnArea%d"), i);
        UBoxComponent* NewArea = NewObject<UBoxComponent>(this, *AreaName);
        NewArea->RegisterComponent();
        NewArea->SetupAttachment(RootComponent);

        // Make the box component selectable and moveable in editor
        NewArea->bSelectable = true;
        NewArea->SetCollisionProfileName(TEXT("NoCollision"));

        // Set visual properties
        NewArea->SetBoxExtent(SpawnAreaSize);
        NewArea->SetRelativeLocation(GetActorLocation());

        // Make the box visible in editor
        NewArea->bIsEditorOnly = false;
        NewArea->ShapeColor = FColor::Green;

        SpawnAreas.Add(NewArea);
    }
}

FVector AMultiAreaSpawner::GetRandomLocationInSpecificArea(const int32 AreaIndex) const
{
    // Get Box Extents (Half-Size of the Box in Local Space)
    FVector BoxExtent = SpawnAreas[AreaIndex]->GetUnscaledBoxExtent();

    // Generate Random Local Position inside the Box Extent
    FVector LocalRandomPoint = FVector(
        FMath::RandRange(-BoxExtent.X, BoxExtent.X),
        FMath::RandRange(-BoxExtent.Y, BoxExtent.Y),
        FMath::RandRange(-BoxExtent.Z, BoxExtent.Z));

    // Transform Local Point to World Space
    return SpawnAreas[AreaIndex]->GetComponentTransform().TransformPosition(LocalRandomPoint);
}

void AMultiAreaSpawner::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (SpawnAreas.Num() == 0)
        UpdateSpawnAreas();
}

void AMultiAreaSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const auto ActiveSpawnAreas = GetActiveSpawnAreas();
    if (bShowDebugBounds)
    {
        if (TriggerBox)
            DrawDebugBox(GetWorld(), TriggerBox->GetComponentLocation(), TriggerBox->GetScaledBoxExtent(), TriggerBox->GetComponentQuat(), FColor::Blue, false, 0.0f, 0, 5.0f);
        for (int i = 0; i < SpawnAreas.Num(); ++i)
            DrawDebugBox(GetWorld(), SpawnAreas[i]->GetComponentLocation(), SpawnAreas[i]->GetScaledBoxExtent(), SpawnAreas[i]->GetComponentQuat(), ActiveSpawnAreas.Contains(i) && bIsActive ? FColor::Green : FColor::Red, false, 0.0f, 0, 5.0f);
    }

    if (!bIsActive)
        return;

    SpawnTimer += DeltaTime;
    if (SpawnTimer > SpawnDelay)
    {
        SpawnTimer = 0;
        SpawnEnemiesInRandomAreas();
    }
    
    Timer -= DeltaTime;
    if (Timer <= 0 && CurrentEnemies <= MaxEnemiesBeforeSpawn)
        SpawnWave();
}

TArray<uint32> AMultiAreaSpawner::GetActiveSpawnAreas() const
{
    TArray<uint32> ActiveSpawnAreas;

    auto* Character = Cast<const AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Character)
    {
        UE_LOG(LogSpawners, Warning, TEXT("Player character not found!"));
        return {};
    }

    for (int i = 0; i < SpawnAreas.Num(); ++i)
        if (FVector::Dist(Character->GetActorLocation(), SpawnAreas[i]->GetComponentLocation()) > SpawnDistance)
            ActiveSpawnAreas.Add(i);

    if (ActiveSpawnAreas.Num() == 0)
        for (int j = 0; j < SpawnAreas.Num(); ++j)
            ActiveSpawnAreas.Add(j);

    return ActiveSpawnAreas;
}

TArray <uint32> AMultiAreaSpawner::GetSpawnAreas() const
{
    TArray<uint32> ActiveSpawnAreas;

    for (int i = 0; i < SpawnAreas.Num(); ++i)
        if (SpawnInformations[i].Num() > 0)
            ActiveSpawnAreas.Add(i);
    
    return ActiveSpawnAreas;
}

void AMultiAreaSpawner::OnEnemyDestroyed()
{
    --CurrentEnemies;
    if (CurrentEnemies == 0)
    {
        if (WaveNumber >= SpawnData->GetRowMap().Num())
            OnSpawnerFinished.Broadcast();
        SpawnWave();
    }
}

void AMultiAreaSpawner::BeginPlay()
{
    Super::BeginPlay();
    for (int32 i = 0; i < SpawnAreas.Num(); ++i)
    {
        SpawnInformations.Add(i, TArray<TSubclassOf<AEnsEnemyBase>>());
    }
    if (bHasTrigger)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMultiAreaSpawner::OnTriggerOverlapBegin);
        return;
    }

    SpawnWave();
}

#if WITH_EDITOR
void AMultiAreaSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AMultiAreaSpawner, NumSpawnAreas))
        UpdateSpawnAreas();

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AMultiAreaSpawner, bHasTrigger))
        UpdateTrigger();
}
#endif

void AMultiAreaSpawner::SpawnWave()
{
    if (WaveNumber >= SpawnData->GetRowMap().Num())
    {
        bIsActive = false;
        return;
    }

    const FSpawnDataRow* Row = SpawnData->FindRow<FSpawnDataRow>(FName(FString::FromInt(++WaveNumber)), "");
    TArray<TSubclassOf<AEnsEnemyBase>> Enemies;
    for (auto EnemyData : Row->EnemiesToSpawn)
    {
        for (int32 i = 0; i < EnemyData.Amount; ++i)
            Enemies.Add(EnemyData.EnemyClass);
        UE_LOG(LogSpawners, Warning, TEXT("Cannot get player character in SpawnEnemies() of spawner %i"), Enemies.Num());
    }

    CurrentEnemies += Enemies.Num();
    ArrayUtils::ShuffleArray(Enemies);
    int32 MaxEnemies = FMath::CeilToInt(0.6f * Enemies.Num());

    TArray<int32> SpawnIndexes;
    for (int32 i = 0; i < SpawnAreas.Num(); ++i)
    {
        SpawnInformations[i].Empty();
        SpawnIndexes.Add(i);
    }
    
    for (const auto& enemy: Enemies)
    {
        bool Placed = false;
        while (!Placed)
        {
            const int32 DistributionIndex = FMath::RandRange(0, SpawnIndexes.Num() - 1);
            if (SpawnInformations[SpawnIndexes[DistributionIndex]].Num() < MaxEnemies || SpawnIndexes.Num() == 1)
            {
                SpawnInformations[SpawnIndexes[DistributionIndex]].Add(enemy);
                Placed = true;
            }
            else
            {
                SpawnIndexes.RemoveAt(DistributionIndex);
            }
        }
    }

    for (const auto& enemyData: SpawnInformations)
    {
        UE_LOG(LogSpawners, Warning, TEXT("Cannot get player character in SpawnEnemies() of spawner %i"),enemyData.Value.Num());
    }

    SpawnDelay = Row->SpawnDelay;
    Timer = Row->Time;
    SpawnTimer = 0;
    GlobalWaveNumber++;
    SpawnEnemiesInRandomAreas();
    OnNewWave.Broadcast(GlobalWaveNumber);
}

void AMultiAreaSpawner::SpawnEnemies(const TSubclassOf<AEnsEnemyBase>& ActorToSpawn, const int32 AreaIndex, const int32 Count)
{
    if (!ActorToSpawn)
        return;

    auto* Character = Cast<const AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Character)
    {
        UE_LOG(LogSpawners, Warning, TEXT("Cannot get player character in SpawnEnemies() of spawner %s"), *GetName());
        return;
    }

    for (int32 i = 0; i < Count; ++i)
    {
        // Spawn the enemy at a random location, facing the player
        FVector SpawnLocation = GetRandomLocationInSpecificArea(AreaIndex);
        FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, Character->GetActorLocation());

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AEnsEnemyBase* Enemy = GetWorld()->SpawnActor<AEnsEnemyBase>(ActorToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
        Enemy->OnEnemyDestroyed.AddDynamic(this, &AMultiAreaSpawner::OnEnemyDestroyed);
    }
}

void AMultiAreaSpawner::SpawnEnemiesInRandomAreas()
{
    auto* Character = Cast<const AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Character)
    {
        UE_LOG(LogSpawners, Warning, TEXT("Player character not found!"));
        return;
    }
    
    auto AvailableSpawners = GetSpawnAreas();

    if (AvailableSpawners.Num() == 0)
        return;

    auto ActiveSpawners = GetActiveSpawnAreas();
    int32 SpawnersToSpawn = FMath::RandRange(1, AvailableSpawners.Num() - 1);
    ArrayUtils::ShuffleArray(AvailableSpawners);
    
    for (int i = 0; i < SpawnersToSpawn; ++i)
    {
        if (!ActiveSpawners.Contains(AvailableSpawners[i]))
        {
            int32 SpawnerIndex = FMath::RandRange(0, ActiveSpawners.Num() - 1);
            SpawnEnemies(SpawnInformations[AvailableSpawners[i]].Pop(), ActiveSpawners[SpawnerIndex], 1);
        }
        else
        {
            SpawnEnemies(SpawnInformations[AvailableSpawners[i]].Pop(), AvailableSpawners[i], 1);
        }
    }
}
