// ItemBase.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionInterface.h"
#include "ItemBase.generated.h"

UCLASS()
class PROJEKT_UE5_API AItemBase : public AActor, public IInteractionInterface
{
    GENERATED_BODY()

public:
    AItemBase();

    UFUNCTION(BlueprintCallable, Category = "Item")
    virtual void PickUp(APawn* InstigatorPawn);

    virtual void Interact_Implementation(APawn* InstigatorPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* ItemMesh;
};
