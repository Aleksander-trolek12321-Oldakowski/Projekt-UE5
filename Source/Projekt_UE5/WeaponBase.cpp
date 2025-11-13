// WeaponBase.cpp
#include "WeaponBase.h"
#include "EquipInterface.h"
#include "UInteractionComponent.h"

AWeaponBase::AWeaponBase()
{
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(RootComponent);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AWeaponBase::PickUp(APawn* InstigatorPawn)
{
    if (!InstigatorPawn) return;

    if (InstigatorPawn->GetClass()->ImplementsInterface(UEquipInterface::StaticClass()))
    {
        IEquipInterface::Execute_EquipItem(InstigatorPawn, this, InstigatorPawn);
    }
    else
    {
        Destroy();
    }
}
