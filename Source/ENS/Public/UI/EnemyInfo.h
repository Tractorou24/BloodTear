// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "EnemyInfo.generated.h"

/**
 *
 */
UCLASS()
class ENS_API UEnemyInfo : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void OnHealthChanged(float healthPercent);

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void OnDeath();
};
