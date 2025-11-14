#include "WeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "EquipInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);

    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(WeaponMesh);
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitBox->SetGenerateOverlapEvents(true);
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    HitBox->SetBoxExtent(FVector(12.f, 30.f, 8.f));
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    if (HitBox)
    {
        HitBox->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnHitBoxBeginOverlap);
    }
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

void AWeaponBase::EnableHitBox()
{
    if (!HitBox) return;
    AlreadyHitActors.Empty();
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeaponBase::DisableHitBox()
{
    if (!HitBox) return;
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AlreadyHitActors.Empty();
}

void AWeaponBase::OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner()) return;
    if (AlreadyHitActors.Contains(OtherActor)) return;
    AlreadyHitActors.Add(OtherActor);

    FVector ImpactPoint;
    if (SweepResult.IsValidBlockingHit())
    {
        ImpactPoint = FVector(SweepResult.ImpactPoint);
    }
    else
    {
        ImpactPoint = OverlappedComp->GetComponentLocation();
    }

    DealMeleeDamage(OtherActor, ImpactPoint);
}

void AWeaponBase::DealMeleeDamage(AActor* OtherActor, const FVector& ImpactPoint)
{
    if (!OtherActor) return;
    UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);
    DrawDebugSphere(GetWorld(), ImpactPoint, 6.f, 8, FColor::Red, false, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Weapon %s applied %f damage to %s at %s"), *GetNameSafe(this), Damage, *GetNameSafe(OtherActor), *ImpactPoint.ToString());
}

void AWeaponBase::DoMeleeTrace(const FVector& Start, const FVector& End)
{
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    FCollisionShape Shape = FCollisionShape::MakeBox(FVector(12.f, 20.f, 8.f));
    bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn, Shape, Params);
    DrawDebugBox(GetWorld(), (Start + End) * 0.5f, Shape.GetBox(), FQuat::Identity, bHit ? FColor::Orange : FColor::Silver, false, 2.0f);

    if (bHit && Hit.GetActor() && Hit.GetActor() != GetOwner())
    {
        if (!AlreadyHitActors.Contains(Hit.GetActor()))
        {
            AlreadyHitActors.Add(Hit.GetActor());
            DealMeleeDamage(Hit.GetActor(), Hit.ImpactPoint);
        }
    }
}
