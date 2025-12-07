// SharedTypes.h
#pragma once

#include "CoreMinimal.h"
#include "SharedTypes.generated.h"

UENUM(BlueprintType)
enum class EPawnState : uint8
{
    Idle        UMETA(DisplayName="Idle"),
    InCombat    UMETA(DisplayName="InCombat"),
    Hit         UMETA(DisplayName="Hit"),
    Exhausted   UMETA(DisplayName="Exhausted"),
    Dead        UMETA(DisplayName="Dead")
};
