// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#include "Characters/Enemies/EnsEnemyBase.h"

#include "Characters/Player/EnsPlayerCharacter.h"
#include "Characters/Player/EnsPlayerController.h"
#include "GAS/AttributeSets/EnsHealthAttributeSet.h"
#include "GAS/EnsAbilitySystemComponent.h"
#include "UI/EnemyInfo.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

DEFINE_LOG_CATEGORY(LogEnemy)

AEnsEnemyBase::AEnsEnemyBase()
{
    // Actor team
    TeamId = FGenericTeamId(1);
    TeamId.ResetAttitudeSolver();

    // Interactions
    InteractZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractCollider"));
    InteractZone->SetupAttachment(RootComponent);

    InteractionClickZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ClickCollider"));
    InteractionClickZone->SetupAttachment(RootComponent);

    UIEnemyInfoComponent = CreateDefaultSubobject<UWidgetComponent>(FName("UIEnemyInfoComponent"));
    UIEnemyInfoComponent->SetupAttachment(RootComponent);
    UIEnemyInfoComponent->SetRelativeLocation(FVector(0, 0, 120));
    UIEnemyInfoComponent->SetWidgetSpace(EWidgetSpace::World);
    UIEnemyInfoComponent->SetDrawSize(FVector2D(500, 500));

    UIEnemyInfoClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/Widget/WB_EnemyInfo.WB_EnemyInfo_C"));
}

void AEnsEnemyBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    if (TeamId != NewTeamID)
        TeamId = NewTeamID;
}

FGenericTeamId AEnsEnemyBase::GetGenericTeamId() const
{
    return TeamId;
}

void AEnsEnemyBase::InitUI()
{
    // Setup UI for Locally Owned Players only, not AI or the server's copy of the PlayerControllers
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->IsLocalPlayerController())
    {
        if (UIEnemyInfoClass)
        {
            UIEnemyInfo = CreateWidget<UEnemyInfo>(PC, UIEnemyInfoClass);
            if (UIEnemyInfo && UIEnemyInfoComponent)
            {
                UIEnemyInfoComponent->SetWidget(UIEnemyInfo);

                // Setup the floating status bar
                UIEnemyInfo->OnHealthChanged(HealthAttributeSet->GetHealth() / HealthAttributeSet->GetMaxHealth());
            }
        }
    }
}

void AEnsEnemyBase::HealthChanged(const FOnAttributeChangeData& Data)
{
    Super::HealthChanged(Data);
    UIEnemyInfo->OnHealthChanged(Data.NewValue / HealthAttributeSet->MaxHealth.GetBaseValue());
}

// Called when the game starts or when spawned
void AEnsEnemyBase::BeginPlay()
{
    InitUI();

    Super::BeginPlay();

    if (!AbilitySystemComponent)
    {
        UE_LOG(LogEnemy, Error, TEXT("Cannot initialize enemy %s with no AbilitySystemComponent"), *GetName());
        return;
    }

    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    AddStartupEffects();
}

void AEnsEnemyBase::OnDeath(AEnsCharacterBase* SourceActor)
{
    // Try to increase xp if the source actor is a player
    if (auto* PlayerCharacter = Cast<AEnsPlayerCharacter>(SourceActor))
        PlayerCharacter->IncreaseXp(GivenExperience);

    OnDeath_Blueprint();
    OnEnemyDestroyed.Broadcast();

    // Set ragdoll physics
    if (auto* CurrentMesh = GetMesh())
    {
        CurrentMesh->SetSimulatePhysics(true);
        CurrentMesh->SetCollisionProfileName(FName("Ragdoll"));
    }

    // Launch FX
    if (DeathEffect == nullptr)
    {
        UE_LOG(LogEnemy, Error, TEXT("Death effect is not set for enemy %s"), *GetName());
        Destroy();
        return;
    }

    auto* ItemInstanceEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DeathEffect, GetActorLocation());
    ItemInstanceEffect->Activate();

    // Destroy the enemy & stop the system when the effect is finished
    FTimerDelegate TimerCallback;
    TimerCallback.BindLambda([ItemInstanceEffect, this] {
        ItemInstanceEffect->Deactivate();
        Destroy();
    });

    FTimerHandle Handle;
    GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, DeathEffectDuration, false);
}

void AEnsEnemyBase::Attacked(AActor* Source)
{
    const auto* PlayerController = Cast<AEnsPlayerController>(Source);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("Enemy %s attacked by something that is not a player. Ignoring."), *GetName());
        return;
    }

    Cast<AEnsCharacterBase>(PlayerController->GetCharacter())->BaseAttack();
}
