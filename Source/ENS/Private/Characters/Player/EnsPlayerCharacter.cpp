// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "Characters/Player/EnsPlayerCharacter.h"
#include "Characters/Enemies/EnsEnemyBase.h"
#include "Equipment/BaseWeapon.h"
#include "Equipment/Inventory.h"
#include "GAS/AttributeSets/EnsPotionAttributeSet.h"
#include "GAS/EnsAbilitySystemComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Navigation/PathFollowingComponent.h"

DEFINE_LOG_CATEGORY(LogPlayerCharacter)

AEnsPlayerCharacter::AEnsPlayerCharacter()
{
    Tags.Add(FName("Player"));

    // Init capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Init movement
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to movement direction
    GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
    GetCharacterMovement()->bConstrainToPlane = true;
    GetCharacterMovement()->bSnapToPlaneAtStart = true;

    Inventory = CreateDefaultSubobject<UInventory>(TEXT("Inventory"));

    PotionAttributeSet = CreateDefaultSubobject<UEnsPotionAttributeSet>(TEXT("PotionAttributeSet"));

    // Set up actor team
    TeamId = FGenericTeamId(0);
    TeamId.ResetAttitudeSolver();
}

void AEnsPlayerCharacter::BaseAttack()
{
    if (!bIsAttacking)
    {
        FGameplayTagContainer Container;
        Container.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.BaseAttack")));
        const bool IsActivated = GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
        if (IsActivated)
            OnBaseAttack();

        bIsAttacking = true;
    }
    else if (bCanBuffer)
        bIsAttackBuffered = true;
}

void AEnsPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogPlayerCharacter, Error, TEXT("Cannot initialize enemy %s with no AbilitySystemComponent"), *GetName());
        return;
    }

    // Check the level up array and validate it
    auto SortedXp = ExperienceLevelTransitions;
    SortedXp.Sort();
    ensure(SortedXp == ExperienceLevelTransitions && "The experience value to transition between levels must be in ascending order.");

    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    AddStartupEffects();
}

void AEnsPlayerCharacter::OnDeath(AEnsCharacterBase* SourceActor)
{
    static bool bCanDie = true;
    if (!bCanDie)
        return;

    APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DisableInput(PlayerController);

    auto Duration = GetMesh()->GetAnimInstance()->Montage_Play(GetInventoryComponent()->GetCurrentWeapon()->DeathAnimationMontage);
    if (Duration == 0.f)
    {
        UE_LOG(LogPlayerCharacter, Warning, TEXT("Failed to play death animation for player %s"), *GetName());
        Duration = 0.1f; // So the timer is called one day (never called at 0)
    }

    FTimerDelegate TimerCallback;
    TimerCallback.BindLambda([=, this] {
        // Find and move to the player start
        const AActor* PlayerStart = GetWorld()->GetAuthGameMode()->FindPlayerStart(GetController());
        SetActorLocation(PlayerStart->GetActorLocation());
        SetActorRotation(PlayerStart->GetActorRotation());

        // Stop all running montages
        GetMesh()->GetAnimInstance()->Montage_Stop(0.f, nullptr);

        EnableInput(PlayerController);
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // Call the parent to reset the attributes
        Super::OnDeath(SourceActor);
        bCanDie = true; // Allow the player to die again
    });

    FTimerHandle DeathTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, TimerCallback, Duration, false);
    bCanDie = false; // Prevents the player from dying multiple times
}

void AEnsPlayerCharacter::IncreaseXp(const int64 Amount)
{
    ensure(Amount > 0 && "Cannot add a negative experience amount.");

    const auto OldLevel = GetCurrentLevel();
    CurrentExperience += Amount;
    UE_LOG(LogPlayerCharacter, Log, TEXT("Added %lld xp to player %s, it now gave %lld xp points."), Amount, *GetName(), CurrentExperience);

    const auto NewLevel = GetCurrentLevel();
    if (NewLevel != OldLevel)
    {
        UE_LOG(LogPlayerCharacter, Log, TEXT("Leveling up player %s from level %lld to %lld"), *GetName(), OldLevel, NewLevel);
        OnLevelUp.Broadcast(NewLevel);
    }
}

int64 AEnsPlayerCharacter::GetCurrentLevel() const
{
    std::size_t Index;
    for (Index = 0; Index < ExperienceLevelTransitions.Num(); Index++)
        if (CurrentExperience < ExperienceLevelTransitions[Index])
            break;
    return Index + 1;
}

UInventory* AEnsPlayerCharacter::GetInventoryComponent() const
{
    return Inventory;
}

void AEnsPlayerCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    if (TeamId != NewTeamID)
    {
        TeamId = NewTeamID;
    }
}

FGenericTeamId AEnsPlayerCharacter::GetGenericTeamId() const
{
    return TeamId;
}

void AEnsPlayerCharacter::Move(const FVector2D& MovementVector)
{
    if (!Controller)
        return;

    AddMovementInput(ForwardVector, MovementVector.X);
    AddMovementInput(RightVector, MovementVector.Y);
}
