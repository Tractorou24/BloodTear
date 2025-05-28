// Copyright (c) 2024-2025, BloodTear contributors. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
};
