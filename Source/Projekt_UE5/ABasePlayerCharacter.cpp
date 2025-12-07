#include "ABasePlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "UInteractionComponent.h"
#include "WeaponBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "APlayerController.h"

ABasePlayerCharacter::ABasePlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

    CurrentWeapon = nullptr;

    Attributes = CreateDefaultSubobject<UAttributesComponent>(TEXT("Attributes"));
}

void ABasePlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (DefaultMappingContext)
                {
                    Subsystem->AddMappingContext(DefaultMappingContext, 0);
                }
            }
        }
    }

    OnTakeAnyDamage.AddDynamic(this, &ABasePlayerCharacter::HandleTakeAnyDamage);

    if (Attributes)
    {
        Attributes->OnHealthChanged.AddDynamic(this, &ABasePlayerCharacter::OnHealthChanged_Handler);
        Attributes->OnStaminaChanged.AddDynamic(this, &ABasePlayerCharacter::OnStaminaChanged_Handler);
        Attributes->OnDeath.AddDynamic(this, &ABasePlayerCharacter::OnPlayerDeath);
    }

    if (AAPlayerController* MyPC = GetBasePC())
    {
        if (MyPC->MainHUDWidget)
        {
            MyPC->MainHUDWidget->UpdateHealth(Attributes ? Attributes->GetHealth() : 0.f,
                                            Attributes ? Attributes->GetMaxHealth() : 0.f);
            MyPC->MainHUDWidget->UpdateStamina(Attributes ? Attributes->GetStamina() : 0.f,
                                            Attributes ? Attributes->GetMaxStamina() : 0.f);
            MyPC->MainHUDWidget->UpdatePawnState(PawnState);
        }
    }
}

void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Move)     EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Move);
        if (IA_Look)     EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Look);
        if (IA_Interact) EIC->BindAction(IA_Interact, ETriggerEvent::Started,   this, &ABasePlayerCharacter::OnInteract);
        if (IA_Attack)   EIC->BindAction(IA_Attack, ETriggerEvent::Started,     this, &ABasePlayerCharacter::OnAttack);
    }
}

void ABasePlayerCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    AddMovementInput(GetActorForwardVector(), MovementVector.Y);
    AddMovementInput(GetActorRightVector(),   MovementVector.X);
}

void ABasePlayerCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

void ABasePlayerCharacter::OnInteract(const FInputActionValue& Value)
{
    if (InteractionComponent)
    {
        InteractionComponent->PrimaryInteract();
    }
}

void ABasePlayerCharacter::SetPawnStateWithTimeout(EPawnState NewState, float Duration /*=2.0f*/)
{
    if (PawnState == EPawnState::Dead)
    {
        return;
    }

    PawnState = NewState;

    if (AAPlayerController* PC = GetBasePC())
    {
        if (PC->MainHUDWidget)
        {
            PC->MainHUDWidget->UpdatePawnState(PawnState);
            UE_LOG(LogTemp, Log, TEXT("HUD UpdatePawnState called -> %d"), (int)PawnState);
        }
    }

    if (Duration <= 0.f)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(StateRestoreTimerHandle);
    GetWorldTimerManager().SetTimer(StateRestoreTimerHandle, this, &ABasePlayerCharacter::RestoreStateToIdle, Duration, false);
}

void ABasePlayerCharacter::RestoreStateToIdle()
{
    if (PawnState == EPawnState::Dead) return;

    PawnState = EPawnState::Idle;

    if (AAPlayerController* PC = GetBasePC())
    {
        if (PC->MainHUDWidget)
        {
            PC->MainHUDWidget->UpdatePawnState(PawnState);
            UE_LOG(LogTemp, Log, TEXT("HUD UpdatePawnState called -> Idle (timer)"));
        }
    }
}


void ABasePlayerCharacter::OnHealthChanged_Handler(float Current, float Max)
{
    if (AAPlayerController* PC = GetBasePC())
    {
        if (PC->MainHUDWidget)
        {
            PC->MainHUDWidget->UpdateHealth(Current, Max);
        }
    }
}

void ABasePlayerCharacter::OnStaminaChanged_Handler(float Current, float Max)
{
    if (AAPlayerController* PC = GetBasePC())
    {
        if (PC->MainHUDWidget)
        {
            PC->MainHUDWidget->UpdateStamina(Current, Max);
        }
    }
    if (Attributes)
    {
        float MinimalThreshold = 10.f;
        if (Attributes->GetStamina() <= MinimalThreshold)
        {
            PawnState = EPawnState::Exhausted;
            if (AAPlayerController* PC2 = GetBasePC())
            {
                if (PC2->MainHUDWidget) PC2->MainHUDWidget->UpdatePawnState(PawnState);
            }
        }
        else if (PawnState == EPawnState::Exhausted)
        {
            PawnState = EPawnState::Idle;
            if (AAPlayerController* PC2 = GetBasePC())
            {
                if (PC2->MainHUDWidget) PC2->MainHUDWidget->UpdatePawnState(PawnState);
            }
        }
    }
}

void ABasePlayerCharacter::OnAttack(const FInputActionValue& Value)
{
    if (PawnState == EPawnState::Hit || PawnState == EPawnState::Dead)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attack blocked due to state %d"), (int)PawnState);
        return;
    }

    if (!Attributes)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Attributes component"));
        return;
    }

    float Cost = Attributes->StaminaCost.StaminaCost_Attack;
    if (!Attributes->CanPayStaminaCost(Cost))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough stamina for attack"));
        return;
    }

    Attributes->PayStamina(Cost);

    SetPawnStateWithTimeout(EPawnState::InCombat, 2.0f);

    if (AAPlayerController* PC = GetBasePC())
    {
        if (PC->MainHUDWidget)
        {
            PC->MainHUDWidget->UpdatePawnState(PawnState);
            UE_LOG(LogTemp, Log, TEXT("HUD UpdatePawnState called -> InCombat"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HUD widget null when setting InCombat"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller is not AAPlayerController when setting InCombat"));
    }

    PlayAttackMontage();
}

void ABasePlayerCharacter::PlayAttackMontage()
{
    if (AttackMontage && GetMesh())
    {
        UE_LOG(LogTemp, Log, TEXT("PlayAnimMontage called"));
        PlayAnimMontage(AttackMontage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No AttackMontage or GetMesh()==null"));
    }
}

void ABasePlayerCharacter::OnAttackNotifyBegin()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->EnableHitBox();
        UE_LOG(LogTemp, Log, TEXT("Character: Attack notify begin - HitBox enabled"));
    }
}

void ABasePlayerCharacter::OnAttackNotifyEnd()
{
    PawnState = EPawnState::Idle;

    if (AAPlayerController* PC = GetBasePC())
    {
        if (PC->MainHUDWidget)
        {
            PC->MainHUDWidget->UpdatePawnState(PawnState);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HUD widget null when setting Idle"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller is not AAPlayerController when setting Idle"));
    }

    if (CurrentWeapon)
    {
        CurrentWeapon->DisableHitBox();
        UE_LOG(LogTemp, Log, TEXT("Character: Attack notify end - HitBox disabled"));
    }
}

void ABasePlayerCharacter::EquipItem_Implementation(AActor* Item, APawn* InstigatorPawn)
{
    if (!Item) return;

    AWeaponBase* Weapon = Cast<AWeaponBase>(Item);
    if (!Weapon)
    {
        return;
    }

    const FName SocketName("WeaponSocket");
    Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

    Weapon->SetOwner(this);
    CurrentWeapon = Weapon;

    UE_LOG(LogTemp, Log, TEXT("Equipped weapon: %s"), *GetNameSafe(Weapon));
}

void ABasePlayerCharacter::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (!Attributes) return;

    UE_LOG(LogTemp, Log, TEXT("Player HandleTakeAnyDamage: Damage=%f from %s"), Damage, *GetNameSafe(DamageCauser));

    SetPawnStateWithTimeout(EPawnState::Hit, 2.0f);

    Attributes->ApplyDamage(Damage);
}

void ABasePlayerCharacter::GetHit_Implementation(AActor* InstigatorActor, float Damage, const FVector& ImpactPoint)
{
    if (Attributes)
    {
        Attributes->ApplyDamage(Damage);
    }

    SetPawnStateWithTimeout(EPawnState::Hit, 2.0f);
}

void ABasePlayerCharacter::OnPlayerDeath(AActor* OwningActor)
{
    PawnState = EPawnState::Dead;
    if (AAPlayerController* PC = GetBasePC())
    {
        if (PC->MainHUDWidget) PC->MainHUDWidget->UpdatePawnState(PawnState);
    }

    if (AAPlayerController* PCtr = Cast<AAPlayerController>(GetController()))
    {
        DisableInput(PCtr);
    }
}
