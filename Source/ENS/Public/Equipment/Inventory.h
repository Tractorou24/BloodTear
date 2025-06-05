// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"

#include "Inventory.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwapWeapon, ABaseWeapon*, NewWeapon);

/**
 * \brief Component that manages the player's inventory and equipment.
 */
UCLASS()
class ENS_API UInventory : public UActorComponent
{
    GENERATED_BODY()

public:
    explicit UInventory();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /// \brief Get currently attached weapon.
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    [[nodiscard]] class ABaseWeapon* GetCurrentWeapon();

    /// \brief Get currently attached armor.
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    [[nodiscard]] class ABaseArmor* GetCurrentArmor();

    /// \brief Get the ability handle for main ability of current weapon.
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    [[nodiscard]] FGameplayAbilitySpecHandle& GetMainAbilitySpecHandle();

    /// \brief Called when a weapon is swapped, with the new weapon.
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnSwapWeapon OnSwapWeapon;

private:
    /// \brief Weapon classes available in the inventory.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TArray<TSubclassOf<ABaseWeapon>> Weapons;

    /// \brief Armor classes available in the inventory.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TArray<TSubclassOf<class ABaseArmor>> Armors;

    /// \brief Index of the currently equipped weapon in the \ref Weapons.
    uint8_t CurrentEquipmentIndex = 0;

    /// \brief Currently attached weapon instance (or `nullptr `if none).
    UPROPERTY(Transient)
    ABaseWeapon* AttachedWeapon = nullptr;

    /// \brief Currently attached armor instance (or `nullptr `if none).
    UPROPERTY(Transient)
    ABaseArmor* AttachedArmor = nullptr;

#pragma region Swap
    /// \brief Cooldown between 2 swaps.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    float SwapCooldown = 0.5f;

    /// \brief Timer for swap cooldown
    float SwapCooldownTimer = 0.0f;

    /// \brief Input action for weapon scrolling/switching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UInputAction* ScrollAction = nullptr;

    /// \brief Input action for switching to 1st weapon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* LightSetAction = nullptr;

    /// \brief Input action for switching to 2nd weapon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* MediumSetAction = nullptr;

    /// \brief Input action for switching to 3rd weapon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* HeavySetAction = nullptr;

    /**
     * \brief Called when the player level up. It must give the ability with the new level.
     * \param NewLevel The nex player level.
     */
    UFUNCTION()
    void OnLevelUp(int64 NewLevel);

    /// \brief Handles weapon switching logic when scroll input is received
    void SwapEquipmentSet(const struct FInputActionValue& Value);

    /// \brief Updates the currently equipped set
    void UpdateEquippedSet();

    /**
     * \brief Switches to the set at index \p SetIndex.
     * \param SetIndex The index of the set to switch to.
     */
    void HandleSet(uint8_t SetIndex);
#pragma endregion

#pragma region Attacks
    /// \brief Input action for switching to 1st weapon and launching the skill
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* LightSetSkill = nullptr;

    /// \brief Input action for switching to 2nd weapon and launching the skill
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* MediumSetSkill = nullptr;

    /// \brief Input action for switching to 3rd weapon and launching the skill
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* HeavySetSkill = nullptr;

    /// \brief Handles the main ability input event.
    void HandleMainAbility();

    /// \brief Handle for the main ability of the current set.
    FGameplayAbilitySpecHandle MainAbilityHandle;
#pragma endregion
};
