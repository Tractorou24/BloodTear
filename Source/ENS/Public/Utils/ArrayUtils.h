#pragma once

#include "CoreMinimal.h"

class ArrayUtils
{
public:
    template <typename T>
    static void ShuffleArray(TArray<T>& Array)
    {
        for (int32 i = Array.Num() - 1; i > 0; --i)
        {
            const int32 SwapIndex = FMath::RandRange(0, i);
            Array.Swap(i, SwapIndex);
        }
    }
};
