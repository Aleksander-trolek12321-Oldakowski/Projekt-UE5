#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "WeaponBase.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class PROJEKT_UE5_API AWeaponBase : public AItemBase
{
    GENERATED_BODY()

public:
    AWeaponBase();

    virtual void BeginPlay() override;

    virtual void PickUp(APawn* InstigatorPawn) override;

    UFUNCTION()
    void EnableHitBox();

    UFUNCTION()
    void DisableHitBox();

    void DoMeleeTrace(const FVector& Start, const FVector& End);

    UFUNCTION()
    void OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void DealMeleeDamage(AActor* OtherActor, const FVector& ImpactPoint);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    UStaticMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    UBoxComponent* HitBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float Damage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float TraceDistance = 120.f;

    TSet<AActor*> AlreadyHitActors;
};
