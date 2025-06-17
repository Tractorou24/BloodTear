// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "Abilities/GameplayAbility.h"

#include "CoreMinimal.h"

#include "EnsGameplayAbilityBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEnsGameplayAbilityBase, Log, All);

/**
 * \brief Base class for all custom abilities in the project.
 */
UCLASS()
class ENS_API UEnsGameplayAbilityBase : public UGameplayAbility
{
    GENERATED_BODY()
    
    virtual void BeginDestroy() override;

public:
    /// \brief Defines the tags that can put this ability in cooldown.
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
    FGameplayTagContainer CooldownTags;

    /// \brief Sets the cooldown duration to use in \ref ApplyCooldown().
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cooldowns")
    FScalableFloat CooldownDuration;

    /**
     * @brief Sets the knockback forces for the ability
     * @param KnockbackForces The knockback forces to apply
     */
    UFUNCTION(BlueprintCallable)
    void SetKnockBackForces(const TArray<FScalableFloat>& InKnockbackForces);

    /**
     * \brief Gets the controller owing the GAS main component.
     * \return The controller if found, nullptr otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    [[nodiscard]] AController* GetControllerFromActorInfo() const;

    /**
     * \brief Get all the possible cooldown tags. (A merge of the engine's cooldown tags and the custom ones in \ref CooldownTags).
     * \return A tag container containing all the cooldown tags.
     */
    virtual const FGameplayTagContainer* GetCooldownTags() const override;

    /**
     * \brief Called to apply the CooldownEffect if provided by the ability.
     * \param Handle The handle of the ability that will be used.
     * \param ActorInfo The actor info.
     * \param ActivationInfo The info related to the ability activation.
     */
    virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

    /**
     * @brief Applies knockback to the passed target
     * @param Target The actor to apply knockback to
     * @param Level The ability Level to get correct knockback
     * @param ComboStep The combo step to get correct knockback, 0 by default if no combo for the ability
     */
    UFUNCTION(BlueprintCallable)
    virtual void ApplyKnockback(AActor* Target, uint8 Level, uint8 ComboStep = 0);
    
private:
    // Temporary container that we will be the pointer returned by \ref GetCooldownTags().
    // This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
    UPROPERTY(Transient)
    FGameplayTagContainer TempCooldownTags;

    /// \brief Defines the knockback forces depending on level (FScalableFloat) and combo (TArray), should only be accessed through \ref GetKnockbackForce().
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Knockbacks", meta=(AllowPrivateAccess = "true"))
    TArray<FScalableFloat> KnockbackForces;
};
