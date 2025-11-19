#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributesComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathSignature, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJEKT_UE5_API UAttributesComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAttributesComponent();

    // Gettery/Settery
    UFUNCTION(BlueprintCallable, Category="Attributes")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, Category="Attributes")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category="Attributes")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category="Attributes")
    float ApplyDamage(float DamageAmount);

    UPROPERTY(BlueprintAssignable, Category="Attributes")
    FOnDeathSignature OnDeath;

    UPROPERTY(BlueprintAssignable, Category="Attributes")
    FOnHealthChangedSignature OnHealthChanged;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attributes")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Attributes")
    float Health;
};
