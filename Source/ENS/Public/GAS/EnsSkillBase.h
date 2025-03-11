// Copyright (c) 2024-2025, Equipment'N Slash contributors. All rights reserved.

#pragma once

#include "GAS/EnsGameplayAbilityBase.h"

#include "CoreMinimal.h"

#include "EnsSkillBase.generated.h"

/**
 * \brief Base class for al weapon/armor skills.
 *
 * It is only used as a common inheritance point for all skills and are them implemented in blueprint.
 */
UCLASS()
class ENS_API UEnsSkillBase : public UEnsGameplayAbilityBase
{
    GENERATED_BODY()

public:
    /// \brief The damage applied at each level of the skill. (0 for the base attack, 1 for the first level, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    TArray<float> DamagePerLevel;

    /// \brief The type of actor to spawn (defines collision & range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill");
    TArray<TSubclassOf<AActor>> CollisionActorPerLevel;

    /// \brief The animation montage to play when the skill is activated (at any point defined in the skill, not only at the beginning).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    UAnimMontage* AnimationMontage;
};

/**
 * \brief Helper class to get values from a \ref TSubclassOf a \ref UEnsSkillBase.
 */
UCLASS()
class ENS_API USkillBaseBlueprintLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * \brief Gets the damage a skill would apply for a specific level.
     * \param Skill The skill to get the value from.
     * \param Level The level to get the value for.
     * \return The damage value for Skill at \p Level.
     */
    UFUNCTION(BlueprintCallable)
    static float SkillDamageFromLevel(TSubclassOf<UEnsSkillBase> Skill, uint8 Level);

    /**
     * \brief Gets the collision actor associated with the \p Skill.
     * \param Skill The skill to egt the actor from.
     * \param Level The level to get the actor for.
     */
    UFUNCTION(BlueprintCallable)
    static TSubclassOf<AActor> SkillCollisionActorFromLevel(TSubclassOf<UEnsSkillBase> Skill, uint8 Level);
};
