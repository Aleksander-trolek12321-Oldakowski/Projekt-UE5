// ABasePlayerCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "EquipInterface.h"
#include "ABasePlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UInteractionComponent;
class AWeaponBase;

UCLASS()
class PROJEKT_UE5_API ABasePlayerCharacter : public ACharacter, public IEquipInterface
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
};
