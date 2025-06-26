// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "EnsAttributeSetBase.h"

#include "EnsPotionAttributeSet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPotionAttributeSet, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPotionUsed, int, Remaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPotionAdded, int, Remaining);

/**
 * \brief The attributes storage for any object having Potion inside the GAS.
 */
UCLASS()
class ENS_API UEnsPotionAttributeSet : public UEnsAttributeSetBase
{
    GENERATED_BODY()

public:
    /**
     * \brief Called after a gameplay effect execution to update its attributes
     * \param Data The data related to the gameplay effect
     */
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    /// \brief Called when the value of \ref HealthQuantity is reduced.
    UPROPERTY(BlueprintAssignable, Category = "HealthPotion")
    FOnPotionUsed OnHealthPotionUsed;

    /// \brief Called when the value of \ref HealthQuantity is increased.
    UPROPERTY(BlueprintAssignable, Category = "HealthPotion")
    FOnPotionAdded OnHealthPotionAdded;
    

    /// \brief The current Health Potion quantity
    UPROPERTY(BlueprintReadOnly, Category = "HealthPotion")
    FGameplayAttributeData HealthQuantity;
    ATTRIBUTE_ACCESSORS(UEnsPotionAttributeSet, HealthQuantity)

    /// \brief The maximum Potion quantity
    UPROPERTY(BlueprintReadOnly, Category = "HealthPotion")
    FGameplayAttributeData HealthMaxQuantity;
    ATTRIBUTE_ACCESSORS(UEnsPotionAttributeSet, HealthMaxQuantity)

    /// \brief A meta attribute that will be used to calculate the number of potion to use (usually 1)
    UPROPERTY(BlueprintReadOnly)
    FGameplayAttributeData PotionUse;
    ATTRIBUTE_ACCESSORS(UEnsPotionAttributeSet, PotionUse)
};
