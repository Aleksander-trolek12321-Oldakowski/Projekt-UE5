#include "EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/GameplayStatics.h"
#include "ABaseEnemyCharacter.h"

AEnemyAIController::AEnemyAIController()
{
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    BehaviorComp  = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    SightConfig->SightRadius = 900.f;
    SightConfig->LoseSightRadius = 1200.f;
    SightConfig->PeripheralVisionAngleDegrees = 70.f;
    SightConfig->SetMaxAge(5.f);

    SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    UE_LOG(LogTemp, Log, TEXT("AEnemyAIController ctor: PerceptionComp %s, SightConfig set (Radius=%f)"),
           *GetNameSafe(PerceptionComp), SightConfig->SightRadius);
}

void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComp)
    {
        PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
        PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
        UE_LOG(LogTemp, Log, TEXT("AEnemyAIController::BeginPlay on %s - Perception delegates bound"), *GetName());
    }
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTreeAsset && BlackboardComp)
    {
        if (BehaviorTreeAsset->BlackboardAsset)
        {
            BlackboardComp->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);
        }
        BehaviorComp->StartTree(*BehaviorTreeAsset);
        UE_LOG(LogTemp, Log, TEXT("AEnemyAIController::OnPossess called. Pawn possessed = %s\nBehavior tree started: %s"),
               *GetNameSafe(InPawn), *GetNameSafe(BehaviorTreeAsset));
    }
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    UE_LOG(LogTemp, Log, TEXT("OnPerceptionUpdated: Count=%d"), UpdatedActors.Num());
    for (AActor* A : UpdatedActors)
    {
        if (!A) continue;

        FActorPerceptionBlueprintInfo Info;
        PerceptionComp->GetActorsPerception(A, Info);

        UE_LOG(LogTemp, Log, TEXT("  Actor: %s, StimuliCount=%d"), *GetNameSafe(A), Info.LastSensedStimuli.Num());

        bool bSeen = false;
        for (const FAIStimulus& Stim : Info.LastSensedStimuli)
        {
            UE_LOG(LogTemp, Log, TEXT("    Stim: WasSensed=%d Age=%f Strength=%f"), Stim.WasSuccessfullySensed(), Stim.GetAge(), Stim.Strength);
            if (Stim.WasSuccessfullySensed())
            {
                bSeen = true;
                break;
            }
        }

        if (bSeen)
        {
            SetTargetActor(A);
            if (BlackboardComp) BlackboardComp->SetValueAsBool(BB_CanSee, true);
            UE_LOG(LogTemp, Log, TEXT("  -> Setting TargetActor to %s"), *GetNameSafe(A));
        }
        else
        {
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsVector(BB_LastKnown, A->GetActorLocation());
                BlackboardComp->ClearValue(BB_TargetActor);
                BlackboardComp->SetValueAsBool(BB_CanSee, false);
                UE_LOG(LogTemp, Log, TEXT("  -> Lost sight of %s, LastKnown set"), *GetNameSafe(A));
            }
        }
    }
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    UE_LOG(LogTemp, Log, TEXT("OnTargetPerceptionUpdated: Actor=%s WasSensed=%d Age=%f Strength=%f"),
           *GetNameSafe(Actor), Stimulus.WasSuccessfullySensed(), Stimulus.GetAge(), Stimulus.Strength);

    if (Stimulus.WasSuccessfullySensed())
    {
        SetTargetActor(Actor);
        if (BlackboardComp) BlackboardComp->SetValueAsBool(BB_CanSee, true);
        UE_LOG(LogTemp, Log, TEXT("  -> Set TargetActor %s"), *GetNameSafe(Actor));
    }
    else
    {
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsVector(BB_LastKnown, Actor->GetActorLocation());
            BlackboardComp->ClearValue(BB_TargetActor);
            BlackboardComp->SetValueAsBool(BB_CanSee, false);
            UE_LOG(LogTemp, Log, TEXT("  -> Lost sight: LastKnown updated for %s"), *GetNameSafe(Actor));
        }
    }
}

void AEnemyAIController::SetTargetActor(AActor* NewTarget)
{
    if (!BlackboardComp) return;
    BlackboardComp->SetValueAsObject(BB_TargetActor, NewTarget);

    if (NewTarget)
    {
        SetFocus(NewTarget);
    }
    else
    {
        ClearFocus(EAIFocusPriority::Gameplay);
    }

    APawn* P = GetPawn();
    if (P)
    {
        AABaseEnemyCharacter* EnemyPawn = Cast<AABaseEnemyCharacter>(P);
        if (EnemyPawn && NewTarget)
        {
            EnemyPawn->RotateTowardsActor(NewTarget);
        }
    }
}


void AEnemyAIController::ClearTargetActor()
{
    if (!BlackboardComp) return;
    BlackboardComp->ClearValue(BB_TargetActor);
}

void AEnemyAIController::SetLastKnownLocation(const FVector& Location)
{
    if (!BlackboardComp) return;
    BlackboardComp->SetValueAsVector(BB_LastKnown, Location);
}

bool AEnemyAIController::TryAttackTarget()
{
    APawn* P = GetPawn();
    if (!P) return false;

    AABaseEnemyCharacter* Enemy = Cast<AABaseEnemyCharacter>(P);
    if (!Enemy) return false;

    return Enemy->TryAttack();
}
