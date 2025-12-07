// ABasePlayerCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "EquipInterface.h"
#include "CombatInterface.h"
#include "AttributesComponent.h"
#include "SharedTypes.h"
#include "ABasePlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UInteractionComponent;
class AWeaponBase;
class UAttributesComponent;
class AAPlayerController;

UCLASS()
class PROJEKT_UE5_API ABasePlayerCharacter : public ACharacter, public IEquipInterface, public ICombatInterface
{
    GENERATED_BODY()

public:
    ABasePlayerCharacter();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void OnInteract(const FInputActionValue& Value);
    void OnAttack(const FInputActionValue& Value);

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* IA_Move;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* IA_Look;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* IA_Interact;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* IA_Attack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UInteractionComponent* InteractionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
    AWeaponBase* CurrentWeapon;

    virtual void EquipItem_Implementation(AActor* Item, APawn* InstigatorPawn) override;

    FORCEINLINE AWeaponBase* GetEquippedWeapon() const { return CurrentWeapon; }

    UFUNCTION(BlueprintCallable, Category="Combat")
    void PlayAttackMontage();

    UFUNCTION()
    void OnAttackNotifyBegin();

    UFUNCTION()
    void OnAttackNotifyEnd();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
    UAnimMontage* AttackMontage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UAttributesComponent* Attributes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
    EPawnState PawnState = EPawnState::Idle;

    UFUNCTION()
    void OnHealthChanged_Handler(float Current, float Max);

    UFUNCTION()
    void OnStaminaChanged_Handler(float Current, float Max);

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void OnPlayerDeath(AActor* OwningActor);

    virtual void GetHit_Implementation(AActor* InstigatorActor, float Damage, const FVector& ImpactPoint) override;

    FORCEINLINE AAPlayerController* GetBasePC() const { return Cast<AAPlayerController>(GetController()); }

private:
    FTimerHandle StateRestoreTimerHandle;

    void SetPawnStateWithTimeout(EPawnState NewState, float Duration = 2.0f);

    void RestoreStateToIdle();    
};
