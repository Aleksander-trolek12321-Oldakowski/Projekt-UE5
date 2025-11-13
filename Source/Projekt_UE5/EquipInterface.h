#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EquipInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEquipInterface : public UInterface
{
    GENERATED_BODY()
};

class IEquipInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equip")
    void EquipItem(AActor* Item, APawn* InstigatorPawn);
};
