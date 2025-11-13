// WeaponBase.h
#pragma once
#include "CoreMinimal.h"
#include "ItemBase.h"
#include "WeaponBase.generated.h"

UCLASS()
class PROJEKT_UE5_API AWeaponBase : public AItemBase
{
    GENERATED_BODY()

public:
    AWeaponBase();

    virtual void PickUp(APawn* InstigatorPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* WeaponMesh;
};
