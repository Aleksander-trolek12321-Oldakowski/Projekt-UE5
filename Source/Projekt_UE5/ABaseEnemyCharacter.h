#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatInterface.h"
#include "Components/BoxComponent.h"
#include "ABaseEnemyCharacter.generated.h"

UENUM(BlueprintType)
enum class EPawnState : uint8
{
    Idle        UMETA(DisplayName="Idle"),
    InCombat    UMETA(DisplayName="InCombat"),
    Hit         UMETA(DisplayName="Hit")
};

class UAttributesComponent;
class USoundBase;
class UAnimMontage;

UCLASS()
class PROJEKT_UE5_API AABaseEnemyCharacter : public ACharacter, public ICombatInterface
{
    GENERATED_BODY()

public:
    AABaseEnemyCharacter();

    virtual void Tick(float DeltaSeconds) override;

    virtual void GetHit_Implementation(AActor* InstigatorActor, float Damage, const FVector& ImpactPoint) override;

    UFUNCTION()
    void StartAttack();

    UFUNCTION()
    void StopAttack();

protected:
    virtual void BeginPlay() override;

    // Attributes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UAttributesComponent* Attributes;

    // Reakcje
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Anim")
    UAnimMontage* HitMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Anim")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Audio")
    USoundBase* HitSound;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
    EPawnState PawnState = EPawnState::Idle;

    UPROPERTY(EditAnywhere, Category="Enemy|AI")
    float SightRadius = 800.f;

    UPROPERTY(EditAnywhere, Category="Enemy|AI")
    float AttackRange = 150.f;

    UPROPERTY(EditAnywhere, Category="Enemy|AI")
    float AttackRate = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Combat")
    float AttackDamage = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Combat")
    FVector MeleeBoxHalfExtent = FVector(40.f, 40.f, 40.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Combat")
    bool bUseAnimNotifyToDealDamage = false;

    TSet<AActor*> AlreadyHitActors;

    FTimerHandle AttackTimerHandle;

    APawn* PlayerPawn = nullptr;

    UFUNCTION()
    void OnDeath(AActor* OwningActor);

    void CheckForPlayer();

    void EnterHitState();

    void HandleAttackNotify();

    void DoMeleeDamage(const FVector& Start, const FVector& End, const FQuat& Rotation = FQuat::Identity);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
    UBoxComponent* MeleeHitBox;

    UFUNCTION()
    void OnMeleeHitboxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};