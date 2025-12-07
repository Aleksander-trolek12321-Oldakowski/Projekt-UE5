#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "AttributesComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, OwningActor);

USTRUCT(BlueprintType)
struct FStaminaCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
    float StaminaRegenRate = 10.f; // points per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
    float StaminaCost_Attack = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
    float StaminaCost_Jump = 15.f;

    FStaminaCost() {}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJEKT_UE5_API UAttributesComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAttributesComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Health
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attributes")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Attributes")
    float Health = 100.f;

private:
    UPROPERTY(EditAnywhere, Category="Attributes")
    float MaxStamina = 100.f;

    UPROPERTY(VisibleAnywhere, Category="Attributes")
    float Stamina = 100.f;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
    FStaminaCost StaminaCost;

    UPROPERTY(BlueprintAssignable, Category="Attributes")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Attributes")
    FOnStaminaChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category="Attributes")
    FOnDeath OnDeath;

    // Health API
    UFUNCTION(BlueprintCallable, Category="Attributes")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintCallable, Category="Attributes")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable, Category="Attributes")
    void ApplyDamage(float Damage);

    // Stamina API
    UFUNCTION(BlueprintCallable, Category="Stamina")
    float GetStamina() const { return Stamina; }

    UFUNCTION(BlueprintCallable, Category="Stamina")
    float GetMaxStamina() const { return MaxStamina; }

    UFUNCTION(BlueprintCallable, Category="Stamina")
    void SetStamina(float NewStamina);

    UFUNCTION(BlueprintCallable, Category="Stamina")
    bool CanPayStaminaCost(float Cost) const;

    UFUNCTION(BlueprintCallable, Category="Stamina")
    void PayStamina(float Cost);

    // Regeneration control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
    bool bEnableStaminaRegen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stamina")
    float StaminaRegenDelay = 1.0f;

private:
    float RegenAccumulator = 0.f;
    float TimeSinceLastStaminaUse = 0.f;
};
