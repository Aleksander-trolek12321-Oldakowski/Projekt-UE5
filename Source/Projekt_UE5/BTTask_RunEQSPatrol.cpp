#include "BTTask_RunEQSPatrol.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UBTTask_RunEQSPatrol::UBTTask_RunEQSPatrol()
{
    NodeName = TEXT("Run EQS Patrol");
}

EBTNodeResult::Type UBTTask_RunEQSPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (!EQSQuery)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_RunEQSPatrol: No EQSQuery assigned"));
        return EBTNodeResult::Failed;
    }

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    APawn* Pawn = AICon->GetPawn();
    UWorld* World = AICon->GetWorld();
    if (!World || !Pawn) return EBTNodeResult::Failed;

    UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(World, EQSQuery, Pawn, EEnvQueryRunMode::AllMatching, nullptr);
    if (!QueryInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_RunEQSPatrol: RunEQSQuery returned null"));
        return EBTNodeResult::Failed;
    }

    TArray<FVector> Locations;
    const bool bGotLocations = QueryInstance->GetQueryResultsAsLocations(Locations);

    if (!bGotLocations || Locations.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_RunEQSPatrol: EQS returned no locations"));
        return EBTNodeResult::Failed;
    }

    const int32 ChosenIndex = FMath::RandRange(0, Locations.Num() - 1);
    const FVector ChosenLocation = Locations[ChosenIndex];

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(PatrolPointKey.SelectedKeyName, ChosenLocation);
        UE_LOG(LogTemp, Log, TEXT("UBTTask_RunEQSPatrol: Set patrol point to %s"), *ChosenLocation.ToString());
        return EBTNodeResult::Succeeded;
    }

    UE_LOG(LogTemp, Warning, TEXT("UBTTask_RunEQSPatrol: No BlackboardComponent found"));
    return EBTNodeResult::Failed;
}
