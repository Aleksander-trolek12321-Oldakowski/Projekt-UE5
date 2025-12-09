#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BTTask_RunEQSPatrol.generated.h"

UCLASS()
class PROJEKT_UE5_API UBTTask_RunEQSPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_RunEQSPatrol();

    UPROPERTY(EditAnywhere, Category="EQS")
    class UEnvQuery* EQSQuery;

    UPROPERTY(EditAnywhere, Category="Blackboard")
    FBlackboardKeySelector PatrolPointKey;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
