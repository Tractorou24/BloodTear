// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"

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
    FScalableFloat BaseAttackDamage;

    /// \brief The time at which the collision actor spawns after the ability is activated.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    float BaseAttackSpawnTime = 1.f;

    /// \brief The time at which the collision actor is deleted after the ability is activated.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    float BaseAttackDestroyTime = 2.f;

    /// \brief The range of the attack. (i.e. the distance at which the player stops moving and attacks)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    float BaseAttackRange = 20.f;

    /// \brief The curve which controls the distance of the dash over time.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    UCurveFloat* TimeCurve = nullptr;

    /// \brief The distance of the dash before attacking.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    float DashDistance = 100.f;

    /// \brief The duration of the dash.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack");
    float DashTime = 1.0f;
#pragma endregion

#pragma region Skills
    /// \brief The weapon special ability.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills");
    TSubclassOf<class UEnsSkillBase> MainSkill;
#pragma endregion

#pragma region Death Animation
    /// \brief The animation montage to play when the character dies while holding this weapon.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Animation");
    UAnimMontage* DeathAnimationMontage = nullptr;
#pragma endregion

    /// \brief Component representing the weapon model.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly);
    UStaticMeshComponent* Mesh = nullptr;

    /// \brief Name of the socket where the weapon should be attached.
    UPROPERTY(EditAnywhere, BlueprintReadOnly);
    FName SocketName;
};
