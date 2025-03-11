// Copyright (c) 2024-2025, Equipment'N Slash contributors. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "BaseWeapon.generated.h"

/**
 * \brief Base class for any weapon contained in the inventory and be attached to a character.
 */
UCLASS()
class ENS_API ABaseWeapon : public AActor
{
    GENERATED_BODY()

public:
    explicit ABaseWeapon();

#pragma region BaseAttack
    /// \brief The weapon's basic attack ability.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    TSubclassOf<class UEnsGameplayAbilityBase> BaseAttackAbility;

    /// \brief The type of actor to spawn (defines collision & range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    TSubclassOf<AActor> BaseAttackCollisionActor;

    /// \brief The animation montage to play when the base attack is activated.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    UAnimMontage* BaseAttackAnimationMontage;

    /// \brief The damage applied to the target during the attack.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    float BaseAttackDamage = 10.f;

    /// \brief The time at which the collision actor spawns after the ability is activated.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    float BaseAttackSpawnTime = 1.f;

    /// \brief The time at which the collision actor is deleted after the ability is activated.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    float BaseAttackDestroyTime = 2.f;
#pragma endregion

#pragma region Skills
    /// \brief The weapon current level
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Skills", meta = (ClampMin = "1", ClampMax = "3"))
    uint8 Level = 1;

    /// \brief The weapon special ability.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills");
    TSubclassOf<class UEnsSkillBase> MainSkill;
#pragma endregion

    /// \brief Component representing the weapon model.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly);
    UStaticMeshComponent* Mesh = nullptr;

    /// \brief Name of the socket where the weapon should be attached.
    UPROPERTY(EditAnywhere, BlueprintReadOnly);
    FName SocketName;
};
