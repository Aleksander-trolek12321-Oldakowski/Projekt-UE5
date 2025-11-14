#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_EnableWeaponCollision.generated.h"

UCLASS()
class PROJEKT_UE5_API UAnimNotifyState_EnableWeaponCollision : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Notify")
    float ExtraTraceDistance = 60.f;
};
