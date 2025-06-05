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

    void InitAndAttach(const class AEnsPlayerCharacter* Character);

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

    /// \brief The time where player cannot interact regardless of his inputs.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack")
    float BasePrimaryRecoveryTime = 1.f;

    /// \brief The time where player can cancel animation to move or attack again.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Attack")
    float BaseSecondaryRecoveryTime = 1.f;

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

    /// \brief The max speed applied to the character when this weapon is equipped.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"));
    float PlayerMaxSpeed = 1.f;

    /// \brief Component representing the weapon model per level on the right hand.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals");
    TArray<TObjectPtr<UStaticMesh>> MeshRightHandPerLevel;

    /// \brief Component representing the weapon model per level on the left hand.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals");
    TArray<TObjectPtr<UStaticMesh>> MeshLeftHandPerLevel;

    /// \brief Name of the right hand socket where the weapon should be attached.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals");
    FName RightHandSocketName;

    /// \brief Name of the left hand socket where the weapon should be attached.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals");
    FName LeftHandSocketName;

private:
    UStaticMeshComponent* RightHandMeshComponent = nullptr;
    UStaticMeshComponent* LeftHandMeshComponent = nullptr;
};
