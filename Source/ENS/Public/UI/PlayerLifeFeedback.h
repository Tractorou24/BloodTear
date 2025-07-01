// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "PlayerLifeFeedback.generated.h"

/**
 *
 */
UCLASS()
class ENS_API UPlayerLifeFeedback : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintImplementableEvent)
    void OnPlayerHealthChanged(float healthPercent);
};
