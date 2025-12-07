#include "AttributesComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UAttributesComponent::UAttributesComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    Health = MaxHealth;
    Stamina = MaxStamina;
}

void UAttributesComponent::BeginPlay()
{
    Super::BeginPlay();
    Health = FMath::Clamp(Health, 0.f, MaxHealth);
    Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);
}

void UAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableStaminaRegen) return;

    TimeSinceLastStaminaUse += DeltaTime;
    if (TimeSinceLastStaminaUse < StaminaRegenDelay) return;

    if (Stamina >= MaxStamina) return;

    float RegenPerSec = StaminaCost.StaminaRegenRate;
    Stamina = FMath::Clamp(Stamina + RegenPerSec * DeltaTime, 0.f, MaxStamina);
    OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void UAttributesComponent::ApplyDamage(float Damage)
{
    if (Damage <= 0.f) return;
    Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(Health, MaxHealth);

    if (Health <= 0.f)
    {
        OnDeath.Broadcast(GetOwner());
    }
}

void UAttributesComponent::SetStamina(float NewStamina)
{
    Stamina = FMath::Clamp(NewStamina, 0.f, MaxStamina);
    OnStaminaChanged.Broadcast(Stamina, MaxStamina);

    if (Stamina <= 0.f)
    {
    }
}

bool UAttributesComponent::CanPayStaminaCost(float Cost) const
{
    return Stamina >= Cost;
}

void UAttributesComponent::PayStamina(float Cost)
{
    if (Cost <= 0.f) return;

    Stamina = FMath::Clamp(Stamina - Cost, 0.f, MaxStamina);
    TimeSinceLastStaminaUse = 0.f;
    OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}
