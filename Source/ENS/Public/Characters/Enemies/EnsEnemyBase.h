// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "Characters/EnsCharacterBase.h"
#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"

#include "EnsEnemyBase.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogEnemy, Log, All);

/// \brief Delegate called when an enemy is destroyed.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDestroyed);

/**
 * \brief Base class for any enemy in the game.
 *
 * This adds:
 *   - the generic UI (health, ...)
 *   - the team agent for the AI system
 *   - the interaction system (which allows to be damaged by the player)
 */
UCLASS()
class ENS_API AEnsEnemyBase : public AEnsCharacterBase, public IGenericTeamAgentInterface
{
    GENERATED_BODY()

public:
    explicit AEnsEnemyBase();

    /**
     * \brief Sets the team the current enemy is in.
     * \param NewTeamID The id of the new team.
     */
    UFUNCTION(BlueprintCallable)
    virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

    /// \brief Gets the id of the team the enemy is currently in.
    UFUNCTION(BlueprintCallable)
    virtual FGenericTeamId GetGenericTeamId() const override;

    /// \brief Called when the enemy is destroyed.
    UPROPERTY(BlueprintAssignable)
    FOnEnemyDestroyed OnEnemyDestroyed;

protected:
    virtual void BeginPlay() override;

    /// \brief Kills the enemy.
    virtual void OnDeath(AEnsCharacterBase* SourceActor) override;

	/// \copydoc AEnsCharacterBase::HealthChanged
	virtual void HealthChanged(const FOnAttributeChangeData& Data) override;

    /**
     * @brief OnDeath blueprint event called by OnDeath native class
     * Unreal does not accepts Blueprint Implementable/Native events to be virtual
     */
    UFUNCTION(BlueprintImplementableEvent)
    void OnDeath_Blueprint();

private:
    /**
     * \brief Called when the enemy is attacked.
     * \param Source The actor that attacked.
     */
    UFUNCTION()
    void Attacked(AActor* Source);

	/**
    * \brief Initializes the UI for the enemy.
	 */
	UFUNCTION()
	void InitUI();

    FGenericTeamId TeamId;

    /// \brief The interaction component.
    UPROPERTY(EditAnywhere, Category = "Interactions", meta = (AllowPrivateAccess = "true"))
    class UEnsMouseInteractableComponent* MouseInteractableComponent = nullptr;

    /// \brief The interaction zone of the enemy (i.e. where it can interact after moving)
    UPROPERTY(EditAnywhere, Category = "Interactions", meta = (AllowPrivateAccess = "true"))
    class UBoxComponent* InteractZone = nullptr;

    /// \brief The zone the player can click into to interact with the enemy.
    UPROPERTY(EditAnywhere, Category = "Interactions", meta = (AllowPrivateAccess = "true"))
    class UBoxComponent* InteractionClickZone = nullptr;

	/// \brief The class used to display the enemy's information in the UI.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UEnemyInfo> UIEnemyInfoClass;

	/// \brief The widget component used to display the enemy's information in the UI.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* UIEnemyInfoComponent = nullptr;

    /// \brief The number of XP points gained by killing this enemy.
    UPROPERTY(EditDefaultsOnly, Category = "Levels", meta = (AllowPrivateAccess = "true"))
    int64 GivenExperience = 10;

	UPROPERTY()
	UEnemyInfo* UIEnemyInfo = nullptr;
};
