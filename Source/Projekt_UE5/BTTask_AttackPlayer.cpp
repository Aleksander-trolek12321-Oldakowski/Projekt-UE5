#include "BTTask_AttackPlayer.h"
#include "EnemyAIController.h"
#include "ABaseEnemyCharacter.h"

UBTTask_AttackPlayer::UBTTask_AttackPlayer()
{
    NodeName = "Attack Player";
}

EBTNodeResult::Type UBTTask_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AEnemyAIController* C = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
    if (!C) return EBTNodeResult::Failed;

    bool bStarted = C->TryAttackTarget();
    return bStarted ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
