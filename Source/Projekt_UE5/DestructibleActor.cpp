#include "DestructibleActor.h"
#include "Components/StaticMeshComponent.h"
#include "AttributesComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADestructibleActor::ADestructibleActor()
{
    PrimaryActorTick.bCanEverTick = false;

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    SetRootComponent(StaticMesh);
    StaticMesh->SetCollisionProfileName(TEXT("BlockAll"));

    Attributes = CreateDefaultSubobject<UAttributesComponent>(TEXT("Attributes"));

    CurrentHits = 0;
}

void ADestructibleActor::BeginPlay()
{
    Super::BeginPlay();

    CurrentHits = 0;
    if (Attributes)
    {
        Attributes->OnDeath.AddDynamic(this, &ADestructibleActor::OnDeath);
    }

}

void ADestructibleActor::GetHit_Implementation(AActor* InstigatorActor, float Damage, const FVector& ImpactPoint)
{
    CurrentHits++;
    UE_LOG(LogTemp, Log, TEXT("%s got hit via GetHit. Hits = %d/%d (Damage=%f)"), *GetName(), CurrentHits, HitsToDestroy, Damage);

    DrawDebugSphere(GetWorld(), ImpactPoint, 10.f, 8, FColor::Red, false, 2.0f);

    if (CurrentHits >= HitsToDestroy)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s destroyed after %d hits"), *GetName(), CurrentHits);
        Destroy();
    }
}

void ADestructibleActor::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    UE_LOG(LogTemp, Warning, TEXT("Destructible TEST: GetHit_Implementation called on with Damage= from "));
    Destroy();
    //UE_LOG(LogTemp, Log, TEXT("%s HandleTakeAnyDamage called. Damage=%f, Causer=%s"), *GetName(), Damage, *GetNameSafe(DamageCauser));
    //FVector ImpactPoint = GetActorLocation();
    //GetHit_Implementation(DamageCauser, Damage, ImpactPoint);
}

void ADestructibleActor::OnDeath(AActor* OwningActor)
{
    Destroy();
}
