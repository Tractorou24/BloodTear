// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "EnsCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitSignature, float, DamageAmount);

/**
 * \brief The base character in the game that can have abilities.
 *
 * It is used for playable characters and mobs.
 */
UCLASS()
class ENS_API AEnsCharacterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AEnsCharacterBase();

    /**
     * \brief Attacks the given \p Enemy using the currently selected weapon basic attack.
     */
    virtual void BaseAttack() {}

    /// \brief Called when the character is hit.
    virtual void OnHit(float DamageAmount);

    /// \brief Called when the character dies.
    virtual void OnDeath();

    UPROPERTY(BlueprintAssignable)
    FOnDeathSignature OnDeathDelegate;

    UPROPERTY(BlueprintAssignable)
    FOnHitSignature OnHitDelegate;

    /// \brief Gets the ability system component stored the character.
    UFUNCTION(BlueprintCallable)
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    /// \brief Cancel the abilities with passed tags
    UFUNCTION(BlueprintCallable)
    void CancelCurrentAbilities(FGameplayTagContainer WithTags);

protected:
    /// \brief Called at the game start to initialize the character.
    virtual void BeginPlay() override;

    /// \brief Adds the startup effects to the character. (e.g. set attributes to default)
    virtual void AddStartupEffects();

    /**
     * \brief Called when the health attribute changes.
     * \param Data The data of the attribute change.
     */
    virtual void HealthChanged(const struct FOnAttributeChangeData& Data) {}

protected:
    /// \brief The ability system component of the character using this state.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
    class UEnsAbilitySystemComponent* AbilitySystemComponent;

    // \brief The effects to apply when the character spawns.
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS")
    TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

    /// \brief The current attribute set of the character using this state.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
    class UEnsHealthAttributeSet* HealthAttributeSet;
};
