#include "UInteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "InteractionInterface.h"

UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UInteractionComponent::PrimaryInteract()
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn) return;

    FVector ViewLoc;
    FRotator ViewRot;

    if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
    {
        PC->GetPlayerViewPoint(ViewLoc, ViewRot);
    }
    else
    {
        ViewLoc = Pawn->GetActorLocation();
        ViewRot = Pawn->GetActorRotation();
    }

    const FVector Start = ViewLoc;
    const FVector End = Start + ViewRot.Vector() * TraceDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    UWorld* World = GetWorld();
    if (!World) return;

    const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params);

    // debug
    DrawDebugLine(World, Start, End, bHit ? FColor::Green : FColor::Red, false, 1.5f, 0, 1.0f);

    if (bHit && Hit.GetActor())
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimaryInteract: Hit actor: %s (component %s)"), *Hit.GetActor()->GetName(), *GetNameSafe(Hit.GetComponent()));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimaryInteract: NO HIT. Start=%s End=%s TraceChannel=%d"), *Start.ToString(), *End.ToString(), (int32)TraceChannel);
    }

    if (bHit && Hit.GetActor())
    {
        AActor* HitActor = Hit.GetActor();
        UE_LOG(LogTemp, Warning, TEXT("PrimaryInteract: Hit actor name: %s, class: %s"),
            *HitActor->GetName(), *HitActor->GetClass()->GetName());

        bool bImplements = HitActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass());
        UE_LOG(LogTemp, Warning, TEXT("PrimaryInteract: Implements InteractionInterface: %d"), bImplements ? 1 : 0);

        if (!bImplements)
        {
            AActor* Parent = HitActor->GetAttachParentActor();
            if (Parent)
            {
                bool bParentImpl = Parent->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass());
                UE_LOG(LogTemp, Warning, TEXT("PrimaryInteract: Parent actor: %s class: %s Implements: %d"),
                    *Parent->GetName(), *Parent->GetClass()->GetName(), bParentImpl ? 1 : 0);
            }
        }

        if (bImplements)
        {
            IInteractionInterface::Execute_Interact(HitActor, Pawn);
        }
    }
}
