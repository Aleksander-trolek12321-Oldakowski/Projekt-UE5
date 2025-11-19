#include "AttributesComponent.h"
#include "GameFramework/Actor.h"

UAttributesComponent::UAttributesComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    Health = MaxHealth;
}

void UAttributesComponent::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
}

float UAttributesComponent::GetHealth() const
{
    return Health;
}

float UAttributesComponent::GetMaxHealth() const
{
    return MaxHealth;
}

void UAttributesComponent::SetHealth(float NewHealth)
{
    Health = FMath::Clamp(NewHealth, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(Health, MaxHealth);
    if (Health <= 0.f)
    {
        AActor* Owner = GetOwner();
        OnDeath.Broadcast(Owner);
    }
}

float UAttributesComponent::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.f) return Health;
    Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(Health, MaxHealth);
    if (Health <= 0.f)
    {
        OnDeath.Broadcast(GetOwner());
    }
    return Health;
}
