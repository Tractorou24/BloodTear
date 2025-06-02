// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "Spawner/MultiAreaSpawner.h"
#include "Characters/Player/EnsPlayerCharacter.h"
#include "Components/BoxComponent.h"
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

    Timer -= DeltaTime;
    if (Timer <= 0)
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

TArray<FSpawnDistribution> AMultiAreaSpawner::DistributeEnemies(const int32 Imps, const int32 SkullFire, const int32 SkullKamikaze)
{
    const auto ActiveSpawnAreas = GetActiveSpawnAreas();
    TArray<FSpawnDistribution> Distributions;
    if (SpawnAreas.Num() == 0)
        return Distributions;

    Distributions.SetNum(ActiveSpawnAreas.Num());
    for (int32 i = 0; i < ActiveSpawnAreas.Num(); ++i)
        Distributions[i].Index = ActiveSpawnAreas[i];

    const int32 TotalEnemies = Imps + SkullFire + SkullKamikaze;
    const int32 MaxEnemiesPerArea = FMath::CeilToInt(TotalEnemies * 0.6f);
    TArray<int32> RemainingEnemies;
    for (int32 i = 0; i < Imps; ++i)
        RemainingEnemies.Add(0); // 0 for Imps
    for (int32 i = 0; i < SkullFire; ++i)
        RemainingEnemies.Add(1); // 1 for SkullFire
    for (int32 i = 0; i < SkullKamikaze; ++i)
        RemainingEnemies.Add(2); // 2 for SkullKamikaze

    // Shuffle the array to randomize monster distribution
    for (int32 i = RemainingEnemies.Num() - 1; i > 0; --i)
    {
        const int32 SwapIndex = FMath::RandRange(0, i);
        RemainingEnemies.Swap(i, SwapIndex);
    }

    for (const int32 EnemyType : RemainingEnemies)
    {
        bool Placed = false;
        while (!Placed)
        {
            const int32 DistributionIndex = FMath::RandRange(0, ActiveSpawnAreas.Num() - 1);

            // Check if this area hasn't reached the maximum
            if (Distributions[DistributionIndex].GetTotal() < MaxEnemiesPerArea || ActiveSpawnAreas.Num() == 1)
            {
                switch (EnemyType)
                {
                    case 0:
                        Distributions[DistributionIndex].Imps++;
                        break;
                    case 1:
                        Distributions[DistributionIndex].SkullFire++;
                        break;
                    case 2:
                        Distributions[DistributionIndex].SkullKamikaze++;
                        break;
                    default:
                        break;
                }
                Placed = true;
            }
        }
    }
    return Distributions;
}

void AMultiAreaSpawner::SpawnWave()
{
    if (WaveNumber >= SpawnData->GetRowMap().Num())
    {
        if (!bIsInfinite)
        {
            bIsActive = false;
            return;
        }
        WaveNumber = 0;
    }

    const FSpawnDataRow* Row = SpawnData->FindRow<FSpawnDataRow>(FName(FString::FromInt(++WaveNumber)), "");
    TArray<FSpawnDistribution> SpawnDistributions = DistributeEnemies(Row->Imps, Row->SkullFire, Row->SkullKamikaze);

    for (int32 i = 0; i < SpawnDistributions.Num(); i++)
    {
        SpawnEnemies(ImpClass, SpawnDistributions[i].Index, SpawnDistributions[i].Imps);
        SpawnEnemies(SkullFireClass, SpawnDistributions[i].Index, SpawnDistributions[i].SkullFire);
        SpawnEnemies(SkullKamikazeClass, SpawnDistributions[i].Index, SpawnDistributions[i].SkullKamikaze);
    }
    CurrentEnemies += Row->Imps + Row->SkullFire + Row->SkullKamikaze;
    Timer = Row->Time;

    GlobalWaveNumber++;
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
