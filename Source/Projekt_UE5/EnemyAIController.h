#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

UCLASS()
class PROJEKT_UE5_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void BeginPlay() override;

    FORCEINLINE class UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
    
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintCallable)
    void SetTargetActor(AActor* NewTarget);

    UFUNCTION(BlueprintCallable)
    void ClearTargetActor();

    UFUNCTION(BlueprintCallable)
    void SetLastKnownLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable)
    bool TryAttackTarget();

    UPROPERTY(EditAnywhere, Category="AI")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
    class UBlackboardComponent* BlackboardComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
    class UBehaviorTreeComponent* BehaviorComp;

protected:
    UPROPERTY(VisibleAnywhere, Category="AI")
    class UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere, Category="AI")
    class UAISenseConfig_Sight* SightConfig;

public:
    FName BB_TargetActor = TEXT("TargetActor");
    FName BB_LastKnown = TEXT("LastKnownPlayerLocation");
    FName BB_NextPatrol = TEXT("NextPatrolPoint");
    FName BB_PawnState = TEXT("PawnState");
    FName BB_IsDead = TEXT("IsDead");
    FName BB_CanSee = TEXT("CanSeePlayer");
};
