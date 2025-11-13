#include "ItemBase.h"
#include "Components/StaticMeshComponent.h"

AItemBase::AItemBase()
{
    PrimaryActorTick.bCanEverTick = false;

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    SetRootComponent(ItemMesh);
    ItemMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AItemBase::PickUp(APawn* InstigatorPawn)
{
    UE_LOG(LogTemp, Log, TEXT("AItemBase::PickUp called by %s"), *GetNameSafe(InstigatorPawn));
    Destroy();
}

void AItemBase::Interact_Implementation(APawn* InstigatorPawn)
{
    PickUp(InstigatorPawn);
}
