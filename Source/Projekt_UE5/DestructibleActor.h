#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatInterface.h"
#include "DestructibleActor.generated.h"

class UStaticMeshComponent;
class UAttributesComponent;

UCLASS()
class PROJEKT_UE5_API ADestructibleActor : public AActor, public ICombatInterface
{
    GENERATED_BODY()

public:
    ADestructibleActor();

    virtual void GetHit_Implementation(AActor* InstigatorActor, float Damage, const FVector& ImpactPoint) override;

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* StaticMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UAttributesComponent* Attributes;

    UPROPERTY(EditAnywhere, Category="Destructible")
    int32 HitsToDestroy = 2;

    int32 CurrentHits = 0;

    UFUNCTION()
    void OnDeath(AActor* OwningActor);
};
