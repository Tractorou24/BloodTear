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
#include "NavFilters/NavigationQueryFilter.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

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

    PathFollowingComponent = CreateDefaultSubobject<UPathFollowingComponent>(TEXT("PathFollowingComponent"));

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

    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    AddStartupEffects();
}


void AEnsPlayerCharacter::OnDeath()
{
    APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DisableInput(PlayerController);

    // Find and move to the player start
    const AActor* PlayerStart = GetWorld()->GetAuthGameMode()->FindPlayerStart(GetController());
    SetActorLocation(PlayerStart->GetActorLocation());
    SetActorRotation(PlayerStart->GetActorRotation());

    EnableInput(PlayerController);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Call the parent to reset the attributes
    Super::OnDeath();
}

UPathFollowingComponent* AEnsPlayerCharacter::GetPathFollowingComponent() const
{
    return PathFollowingComponent;
}

void AEnsPlayerCharacter::MoveToLocation(const FVector& Location)
{
    TSubclassOf<UNavigationQueryFilter> DefaultNavigationFilterClass;
    FAIMoveRequest MoveReq(Location);
    MoveReq.SetUsePathfinding(true);
    MoveReq.SetAllowPartialPath(true);
    MoveReq.SetProjectGoalLocation(false);
    MoveReq.SetNavigationFilter(DefaultNavigationFilterClass);
    MoveReq.SetAcceptanceRadius(GetInventoryComponent()->GetCurrentWeapon()->BaseAttackRange);
    MoveReq.SetReachTestIncludesAgentRadius(false);
    MoveReq.SetCanStrafe(true);
    MoveTo(MoveReq);
}

void AEnsPlayerCharacter::MoveToActor(const AActor* Actor)
{
    TSubclassOf<UNavigationQueryFilter> DefaultNavigationFilterClass;
    FAIMoveRequest MoveReq(Actor);
    MoveReq.SetUsePathfinding(true);
    MoveReq.SetAllowPartialPath(true);
    MoveReq.SetNavigationFilter(DefaultNavigationFilterClass);
    MoveReq.SetAcceptanceRadius(GetInventoryComponent()->GetCurrentWeapon()->BaseAttackRange);
    MoveReq.SetReachTestIncludesAgentRadius(true);
    MoveReq.SetCanStrafe(true);
    MoveTo(MoveReq);
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

void AEnsPlayerCharacter::MoveTo(const FAIMoveRequest& MoveReq)
{
    FPathFindingQuery PFQuery;
    if (!MoveReq.IsMoveToActorRequest())
    {
        bool bCanRequestMove = true;

        if (MoveReq.GetGoalLocation().ContainsNaN() || FAISystem::IsValidLocation(MoveReq.GetGoalLocation()) == false)
            bCanRequestMove = false;

        if (bCanRequestMove && MoveReq.IsProjectingGoal())
        {
            UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
            const FNavAgentProperties& AgentProps = GetNavAgentPropertiesRef();
            FNavLocation ProjectedLocation;

            if (NavSys && !NavSys->ProjectPointToNavigation(MoveReq.GetGoalLocation(), ProjectedLocation, INVALID_NAVEXTENT, &AgentProps))
                bCanRequestMove = false;

            MoveReq.UpdateGoalLocation(ProjectedLocation.Location);
        }
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    const ANavigationData* NavData = (NavSys == nullptr)
                                         ? nullptr
                                     : MoveReq.IsUsingPathfinding()
                                         ? NavSys->GetNavDataForProps(GetNavAgentPropertiesRef(), GetNavAgentLocation())
                                         : NavSys->GetAbstractNavData();

    if (NavData)
    {
        FVector GoalLocation = MoveReq.GetGoalLocation();
        if (MoveReq.IsMoveToActorRequest())
        {
            const INavAgentInterface* NavGoal = Cast<const INavAgentInterface>(MoveReq.GetGoalActor());
            if (NavGoal)
            {
                const FVector Offset = NavGoal->GetMoveGoalOffset(static_cast<const AActor*>(this));
                GoalLocation = FQuatRotationTranslationMatrix(MoveReq.GetGoalActor()->GetActorQuat(), NavGoal->GetNavAgentLocation()).TransformPosition(Offset);
            }
            else
            {
                GoalLocation = MoveReq.GetGoalActor()->GetActorLocation();
            }
        }
        FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, this, MoveReq.GetNavigationFilter());
        PFQuery = FPathFindingQuery(*this, *NavData, GetNavAgentLocation(), GoalLocation, NavFilter);
        PFQuery.SetAllowPartialPaths(MoveReq.IsUsingPartialPaths());
        PFQuery.SetRequireNavigableEndLocation(MoveReq.IsNavigableEndLocationRequired());
        if (MoveReq.IsApplyingCostLimitFromHeuristic())
        {
            const float HeuristicScale = NavFilter->GetHeuristicScale();
            PFQuery.CostLimit = FPathFindingQuery::ComputeCostLimitFromHeuristic(PFQuery.StartLocation,
                                                                                 PFQuery.EndLocation,
                                                                                 HeuristicScale,
                                                                                 MoveReq.GetCostLimitFactor(),
                                                                                 MoveReq.GetMinimumCostLimit());
        }

        if (PathFollowingComponent)
        {
            PathFollowingComponent->OnPathfindingQuery(PFQuery);
        }
    }

    FNavPathSharedPtr Path;
    if (NavSys)
    {
        FPathFindingResult PathResult = NavSys->FindPathSync(PFQuery);
        if (PathResult.Result != ENavigationQueryResult::Error)
        {
            if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
            {
                if (MoveReq.IsMoveToActorRequest())
                {
                    PathResult.Path->SetGoalActorObservation(*MoveReq.GetGoalActor(), 100.0f);
                }

                PathResult.Path->EnableRecalculationOnInvalidation(true);
                Path = PathResult.Path;
            }
        }
    }

    Path.IsValid() ? PathFollowingComponent->RequestMove(MoveReq, Path) : FAIRequestID::InvalidRequest;
}
