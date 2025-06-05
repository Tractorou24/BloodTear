// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "Equipment/Inventory.h"
#include "Characters/Player/EnsPlayerCharacter.h"
#include "Equipment/BaseArmor.h"
#include "Equipment/BaseWeapon.h"
#include "GAS/EnsSkillBase.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY(LogInventory);

UInventory::UInventory()
{
    PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UInventory::BeginPlay()
{
    Super::BeginPlay();

    ensure(Weapons.Num() > 0);
    ensure(Weapons.Num() == Armors.Num());

    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn)
    {
        UE_LOG(LogInventory, Error, TEXT("Inventory component must be attached to a pawn."));
        return;
    }

    // Get the player controller
    const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
    if (!PlayerController)
    {
        UE_LOG(LogInventory, Error, TEXT("Inventory component must be attached to a player controller."));
        return;
    }

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
    {
        EnhancedInputComponent->BindAction(ScrollAction, ETriggerEvent::Triggered, this, &UInventory::SwapEquipmentSet);

        EnhancedInputComponent->BindActionValueLambda(LightSetAction, ETriggerEvent::Started, [&](const FInputActionValue&) { HandleSet(0); });
        EnhancedInputComponent->BindActionValueLambda(MediumSetAction, ETriggerEvent::Started, [&](const FInputActionValue&) { HandleSet(1); });
        EnhancedInputComponent->BindActionValueLambda(HeavySetAction, ETriggerEvent::Started, [&](const FInputActionValue&) { HandleSet(2); });

        EnhancedInputComponent->BindActionValueLambda(LightSetSkill,
                                                      ETriggerEvent::Started,
                                                      [&](const FInputActionValue&) {
                                                          HandleSet(0);
                                                          HandleMainAbility();
                                                      });
        EnhancedInputComponent->BindActionValueLambda(MediumSetSkill,
                                                      ETriggerEvent::Started,
                                                      [&](const FInputActionValue&) {
                                                          HandleSet(1);
                                                          HandleMainAbility();
                                                      });
        EnhancedInputComponent->BindActionValueLambda(HeavySetSkill,
                                                      ETriggerEvent::Started,
                                                      [&](const FInputActionValue&) {
                                                          HandleSet(2);
                                                          HandleMainAbility();
                                                      });
    }

    if (AEnsPlayerCharacter* PlayerCharacter = Cast<AEnsPlayerCharacter>(Pawn))
        PlayerCharacter->OnLevelUp.AddDynamic(this, &UInventory::OnLevelUp);
    else
        UE_LOG(LogInventory, Error, TEXT("Inventory component must be attached to a player."));

    // Initialize the equipment set (at index 0)
    UpdateEquippedSet();
}

void UInventory::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    SwapCooldownTimer += DeltaTime;
}

ABaseWeapon* UInventory::GetCurrentWeapon()
{
    return AttachedWeapon;
}

ABaseArmor* UInventory::GetCurrentArmor()
{
    return AttachedArmor;
}

FGameplayAbilitySpecHandle& UInventory::GetMainAbilitySpecHandle()
{
    return MainAbilityHandle;
}

void UInventory::OnLevelUp(const int64 /*NewLevel*/)
{
    UpdateEquippedSet();
}

void UInventory::SwapEquipmentSet(const FInputActionValue& Value)
{
    HandleSet((CurrentEquipmentIndex + Weapons.Num() + (Value.Get<float>() > 0 ? 1 : -1)) % Weapons.Num());
}

void UInventory::UpdateEquippedSet()
{
    const AEnsPlayerCharacter* Character = Cast<AEnsPlayerCharacter>(GetOwner());
    ensure(Character && "Cannot get inventory owner as our custom player character");

    // Destroy previous weapon if it exists
    if (AttachedWeapon || AttachedArmor)
    {
        // Remove abilities using stored handles
        if (UAbilitySystemComponent* AbilitySystem = Character->FindComponentByClass<UAbilitySystemComponent>())
            if (MainAbilityHandle.IsValid())
                AbilitySystem->ClearAbility(MainAbilityHandle);

        Character->GetMesh()->MoveIgnoreActors.Remove(AttachedWeapon);
        Character->GetMesh()->MoveIgnoreActors.Remove(AttachedArmor);

        AttachedArmor->Destroy();
        AttachedWeapon->Destroy();

        AttachedWeapon = nullptr;
        AttachedArmor = nullptr;
    }

    // Spawn and attach new weapon
    if (auto& WeaponClass = Weapons[CurrentEquipmentIndex])
    {
        AttachedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponClass, FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
        AttachedWeapon->InitAndAttach(Character);

        Character->GetMesh()->MoveIgnoreActors.Add(AttachedWeapon);
        if (UAbilitySystemComponent* AbilitySystem = Character->FindComponentByClass<UAbilitySystemComponent>())
            MainAbilityHandle = AbilitySystem->GiveAbility(FGameplayAbilitySpec(AttachedWeapon->MainSkill, Character->GetCurrentLevel(), 0));

        UE_LOG(LogInventory, Log, TEXT("Equipped weapon: %s"), *WeaponClass->GetName());
        OnSwapWeapon.Broadcast(GetCurrentWeapon());
    }

    if (auto& ArmorClass = Armors[CurrentEquipmentIndex])
    {
        AttachedArmor = GetWorld()->SpawnActor<ABaseArmor>(ArmorClass, FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
        const bool bIsAttached = AttachedArmor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachedArmor->SocketName);
        if (!bIsAttached)
            UE_LOG(LogInventory, Error, TEXT("Failed to attach armor to the character mesh."));

        Character->GetMesh()->MoveIgnoreActors.Add(AttachedArmor);
        UE_LOG(LogInventory, Log, TEXT("Equipped armor: %s"), *ArmorClass->GetName());
    }
}

void UInventory::HandleMainAbility()
{
    ensure(AttachedWeapon->MainSkill != nullptr && "MainSkill must be set for the weapon.");

    const auto* Character = CastChecked<AEnsPlayerCharacter>(GetOwner());
    if (Character->IsDead())
        return;

    if (UAbilitySystemComponent* AbilitySystem = Character->FindComponentByClass<UAbilitySystemComponent>())
        AbilitySystem->TryActivateAbilityByClass(AttachedWeapon->MainSkill);
}

void UInventory::HandleSet(const uint8_t SetIndex)
{
    auto* Character = CastChecked<AEnsPlayerCharacter>(GetOwner());
    if (SwapCooldownTimer <= SwapCooldown || CurrentEquipmentIndex == SetIndex || Character->IsDead())
        return;

    // Cancel the current attack if any
    if (UAbilitySystemComponent* AbilitySystem = Cast<ACharacter>(GetOwner())->FindComponentByClass<UAbilitySystemComponent>())
    {
        const FGameplayTagContainer Container(FGameplayTag::RequestGameplayTag(FName("Ability.BaseAttack")));
        AbilitySystem->CancelAbilities(&Container);
    }

    SwapCooldownTimer = 0;
    CurrentEquipmentIndex = SetIndex;
    UpdateEquippedSet();
}
