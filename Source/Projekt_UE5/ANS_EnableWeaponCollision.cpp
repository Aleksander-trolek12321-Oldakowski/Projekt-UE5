#include "ANS_EnableWeaponCollision.h"
#include "WeaponBase.h"
#include "ABasePlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_EnableWeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    ABasePlayerCharacter* MyChar = Cast<ABasePlayerCharacter>(Owner);
    if (MyChar)
    {
        MyChar->OnAttackNotifyBegin();

        if (AWeaponBase* W = MyChar->GetEquippedWeapon())
        {
            FVector Start = W->GetActorLocation();
            FVector End = Start + (Owner->GetActorForwardVector() * ExtraTraceDistance);
            W->DoMeleeTrace(Start, End);
        }
    }
}

void UAnimNotifyState_EnableWeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    ABasePlayerCharacter* MyChar = Cast<ABasePlayerCharacter>(Owner);
    if (MyChar)
    {
        MyChar->OnAttackNotifyEnd();
    }
}
