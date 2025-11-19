#include "ANS_EnableWeaponCollision.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "WeaponBase.h"
#include "Kismet/KismetSystemLibrary.h"

void UAnimNotifyState_EnableWeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (!MeshComp) return;
    AActor* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor) return;

    TArray<AActor*> AttachedActors;
    OwnerActor->GetAttachedActors(AttachedActors);
    for (AActor* A : AttachedActors)
    {
        if (!A) continue;
        if (AWeaponBase* Weapon = Cast<AWeaponBase>(A))
        {
            // Mamy WeaponBase -> włącz hitbox broni
            Weapon->EnableHitBox();
            UE_LOG(LogTemp, Log, TEXT("ANS: Enabled WeaponBase hitbox on owner %s (weapon %s)"), *OwnerActor->GetName(), *Weapon->GetName());
            return;
        }
    }

    TArray<UActorComponent*> BoxComps = OwnerActor->GetComponentsByTag(UBoxComponent::StaticClass(), FName("MeleeHitBox"));
    for (UActorComponent* Comp : BoxComps)
    {
        if (UBoxComponent* Box = Cast<UBoxComponent>(Comp))
        {
            Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Box->SetGenerateOverlapEvents(true);
            UE_LOG(LogTemp, Log, TEXT("ANS: Enabled BoxComponent hitbox on owner %s (component %s)"), *OwnerActor->GetName(), *Box->GetName());
        }
    }

}

void UAnimNotifyState_EnableWeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;
    AActor* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor) return;

    // 1) Szukamy broni i wyłączamy
    TArray<AActor*> AttachedActors;
    OwnerActor->GetAttachedActors(AttachedActors);
    for (AActor* A : AttachedActors)
    {
        if (!A) continue;
        if (AWeaponBase* Weapon = Cast<AWeaponBase>(A))
        {
            Weapon->DisableHitBox();
            UE_LOG(LogTemp, Log, TEXT("ANS: Disabled WeaponBase hitbox on owner %s (weapon %s)"), *OwnerActor->GetName(), *Weapon->GetName());
            return;
        }
    }

    TArray<UActorComponent*> BoxComps = OwnerActor->GetComponentsByTag(UBoxComponent::StaticClass(), FName("MeleeHitBox"));
    for (UActorComponent* Comp : BoxComps)
    {
        if (UBoxComponent* Box = Cast<UBoxComponent>(Comp))
        {
            Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Box->SetGenerateOverlapEvents(false);
            UE_LOG(LogTemp, Log, TEXT("ANS: Disabled BoxComponent hitbox on owner %s (component %s)"), *OwnerActor->GetName(), *Box->GetName());
        }
    }
}
