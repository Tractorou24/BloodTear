// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "Utils/MiscCheats.h"

#include "Kismet/GameplayStatics.h"

#include "Characters/Enemies/EnsEnemyBase.h"
#include "Characters/Player/EnsPlayerCharacter.h"
#include "Characters/Player/EnsPlayerController.h"

void UMiscCheats::Die()
{
    auto* Character = Cast<AEnsPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    Character->OnDeath(Character);
}

void UMiscCheats::DieForRespawn()
{
    const auto* Controller = Cast<AEnsPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    for (int i = 0; i < Controller->Lives; i++)
        Die();
}

void UMiscCheats::SpawnAI(const EEnemyType AIToSpawn, const uint8 Number) const
{
    TSubclassOf<AEnsEnemyBase> AIClassToSpawn;
    switch (AIToSpawn)
    {
        case EEnemyType::Imp:
            AIClassToSpawn = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/Characters/Imp/BP_Imp.BP_Imp_C"));
            break;
        case EEnemyType::SkullFire:
            AIClassToSpawn = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/Characters/SkullFire/BP_SkullFire.BP_SkullFire_C"));
            break;
        case EEnemyType::SkullKamikaze:
            AIClassToSpawn = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/Characters/SkullKamikaze/BP_SkullKamikaze.BP_SkullKamikaze_C"));
            break;
    }

    UWorld* World = GetWorld();
    const ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    FVector LocationToSpawn = Player->GetActorLocation() + Player->GetActorForwardVector() * 300;

    for (int i = 0; i < Number; ++i)
    {
        World->SpawnActor(AIClassToSpawn, &LocationToSpawn);
        LocationToSpawn += FVector(0, 0, 100);
    }
}
