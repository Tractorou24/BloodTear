// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "GAS/EnsGameplayAbilityBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToActorForce.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToActorForce.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToActorForce.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Characters/Enemies/EnsEnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"

DEFINE_LOG_CATEGORY(LogEnsGameplayAbilityBase)

void UEnsGameplayAbilityBase::BeginDestroy()
{
    Super::BeginDestroy();
}

void UEnsGameplayAbilityBase::SetKnockBackForces(const TArray<FScalableFloat>& InKnockbackForces)
{
    KnockbackForces = InKnockbackForces;
}

AController* UEnsGameplayAbilityBase::GetControllerFromActorInfo() const
{
    if (!CurrentActorInfo)
        return nullptr;

    // Look for a player controller in the actor info.
    if (AController* PlayerController = CurrentActorInfo->PlayerController.Get())
        return PlayerController;

    // Look for a player controller or pawn in the owner chain.
    AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
    while (TestActor)
    {
        if (AController* Controller = Cast<AController>(TestActor))
            return Controller;
        if (const APawn* Pawn = Cast<APawn>(TestActor))
            return Pawn->GetController();

        TestActor = TestActor->GetOwner();
    }
    return nullptr;
}

const FGameplayTagContainer* UEnsGameplayAbilityBase::GetCooldownTags() const
{
    FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
    MutableTags->Reset(); // MutableTags writes to the TempCooldownTags on the CDO so clear it in case the ability cooldown tags change (moved to a different slot)
    const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
    if (ParentTags)
    {
        MutableTags->AppendTags(*ParentTags);
    }
    MutableTags->AppendTags(CooldownTags);
    return MutableTags;
}

void UEnsGameplayAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
    if (CooldownGE)
    {
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
        SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTags);
        SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Ability.Cooldown")), CooldownDuration.GetValueAtLevel(GetAbilityLevel()));
        FActiveGameplayEffectHandle AGEHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
    }
}

void UEnsGameplayAbilityBase::ApplyKnockback(AActor* Target, uint8 Level, uint8 ComboStep)
{
    if (KnockbackForces.IsValidIndex(ComboStep))
    {
        AEnsEnemyBase* TargetCharacter = Cast<AEnsEnemyBase>(Target);
        const float KnockbackForce = KnockbackForces[ComboStep].GetValueAtLevel(Level);
        const FVector TargetLocation = Target->GetActorLocation();
        const FVector TargetKnockedbackLocation = ((TargetLocation - GetAvatarActorFromActorInfo()->GetActorLocation()).GetSafeNormal() * 0.33 + GetAvatarActorFromActorInfo()->GetActorForwardVector()).GetSafeNormal() * KnockbackForce;
        TargetCharacter->GetCharacterMovement()->Launch(TargetKnockedbackLocation);
    }
    else
    {
        UE_LOG(LogEnsGameplayAbilityBase, Error, TEXT("Can't find any knockback scalablefloat at the provided ComboStep."));
    }
}
