// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "Equipment/BaseWeapon.h"
#include "Characters/Player/EnsPlayerCharacter.h"

ABaseWeapon::ABaseWeapon()
{
    LeftHandMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandWeaponMesh"));
    LeftHandMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RightHandMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandWeaponMesh"));
    RightHandMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseWeapon::InitAndAttach(const AEnsPlayerCharacter* Character)
{
    ensure(Character != nullptr && "Character must not be null");

    const auto Level = Character->GetCurrentLevel() - 1;
    auto LeftHandMesh = MeshLeftHandPerLevel.IsValidIndex(Level) ? MeshLeftHandPerLevel[Level] : nullptr;
    auto RightHandMesh = MeshRightHandPerLevel.IsValidIndex(Level) ? MeshRightHandPerLevel[Level] : nullptr;

    ensure(RightHandMesh != nullptr && "Left hand weapon mesh must not be null"); // Ensure the left hand mesh is valid
    RightHandMeshComponent->SetStaticMesh(RightHandMesh);
    RightHandMeshComponent->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightHandSocketName);

    if (LeftHandMesh)
    {
        LeftHandMeshComponent->SetStaticMesh(LeftHandMesh);
        LeftHandMeshComponent->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftHandSocketName);
    }
}

FAnimData ABaseWeapon::GetCurrentAnimationData()
{
    return BaseAttackAnimationMontages[AttackComboIndex];
}

void ABaseWeapon::IncrementCombo()
{
    AttackComboIndex = (AttackComboIndex + 1 ) % BaseAttackAnimationMontages.Num();
}
