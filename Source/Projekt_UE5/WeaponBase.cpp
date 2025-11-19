#include "WeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CombatInterface.h"
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
    HitBox->SetGenerateOverlapEvents(true);


    HitBox->SetCollisionObjectType(ECC_WorldDynamic);
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    HitBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
    UE_LOG(LogTemp, Log, TEXT("Weapon: HitBox ENABLED (object %s)"), *GetNameSafe(HitBox));
}

void AWeaponBase::DisableHitBox()
{
    if (!HitBox) return;
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitBox->SetGenerateOverlapEvents(false);
    AlreadyHitActors.Empty();
    UE_LOG(LogTemp, Log, TEXT("Weapon: HitBox DISABLED (object %s)"), *GetNameSafe(HitBox));
}


void AWeaponBase::OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == GetOwner()) return;
    if (AlreadyHitActors.Contains(OtherActor)) return;

    UE_LOG(LogTemp, Log, TEXT("Weapon: OnHitBoxBeginOverlap hit actor: %s (comp %s)"), *GetNameSafe(OtherActor), *GetNameSafe(OtherComp));
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

    UE_LOG(LogTemp, Log, TEXT("Weapon: OnHitBoxBeginOverlap hit actor: %s (comp %s)"), *GetNameSafe(OtherActor), *GetNameSafe(OtherComp));
    DrawDebugSphere(GetWorld(), ImpactPoint, 8.f, 8, FColor::Red, false, 2.0f);

    if (OtherActor->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
    {
        AActor* InstigatorActor = GetOwner() ? GetOwner() : this;
        UE_LOG(LogTemp, Log, TEXT("Weapon: calling Execute_GetHit on %s"), *GetNameSafe(OtherActor));
        ICombatInterface::Execute_GetHit(OtherActor, InstigatorActor, Damage, ImpactPoint);
        return;
    }

    UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);
    UE_LOG(LogTemp, Log, TEXT("Weapon: ApplyDamage on %s"), *GetNameSafe(OtherActor));
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
    if (!GetWorld()) return;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (GetOwner()) Params.AddIgnoredActor(GetOwner());
    Params.bTraceComplex = true;

    const FVector BoxExtent = FVector(12.f, 20.f, 8.f);
    FCollisionShape Shape = FCollisionShape::MakeBox(BoxExtent);

    bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn, Shape, Params);

    DrawDebugBox(GetWorld(), (Start + End) * 0.5f, BoxExtent, FQuat::Identity, bHit ? FColor::Red : FColor::Green, false, 1.5f, 0, 2.0f);
    DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 1.5f, 0, 1.0f);

    if (!bHit || !Hit.GetActor()) return;

    AActor* HitActor = Hit.GetActor();
    if (AlreadyHitActors.Contains(HitActor)) return;
    AlreadyHitActors.Add(HitActor);

    const FVector ImpactPoint = Hit.ImpactPoint;

    if (HitActor->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
    {
        ICombatInterface::Execute_GetHit(HitActor, GetOwner() ? GetOwner() : this, Damage, ImpactPoint);
        UE_LOG(LogTemp, Log, TEXT("Weapon: DoMeleeTrace Execute_GetHit on %s"), *GetNameSafe(HitActor));
    }
    else
    {
        UGameplayStatics::ApplyDamage(HitActor, Damage, GetInstigatorController(), this, nullptr);
        UE_LOG(LogTemp, Log, TEXT("Weapon: DoMeleeTrace ApplyDamage on %s"), *GetNameSafe(HitActor));
    }
}
