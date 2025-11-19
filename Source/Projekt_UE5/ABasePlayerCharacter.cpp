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
        Attributes->OnDeath.AddDynamic(this, &ABasePlayerCharacter::OnPlayerDeath);
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

void ABasePlayerCharacter::OnAttack(const FInputActionValue& Value)
{
    if (HasAuthority())
    {
        PlayAttackMontage();
        UE_LOG(LogTemp, Log, TEXT("Has Authority"));   
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Has no Authority"));  
    }
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
    Attributes->ApplyDamage(Damage);
}

void ABasePlayerCharacter::OnPlayerDeath(AActor* OwningActor)
{
    UE_LOG(LogTemp, Warning, TEXT("Player died: %s"), *GetName());

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }

    if (USkeletalMeshComponent* SkelMesh = GetMesh())
    {
        if (UCapsuleComponent* Capsule = GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
        SkelMesh->SetAllBodiesSimulatePhysics(true);
        SkelMesh->SetSimulatePhysics(true);
        SkelMesh->WakeAllRigidBodies();
    }

    FTimerHandle RespawnTimer;
    GetWorldTimerManager().SetTimer(RespawnTimer, [this]()
    {
        UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
    }, 3.0f, false);
}

void ABasePlayerCharacter::GetHit_Implementation(AActor* InstigatorActor, float Damage, const FVector& ImpactPoint)
{
    if (!Attributes) return;

    UE_LOG(LogTemp, Log, TEXT("Player GetHit_Implementation: Damage=%f from %s"), Damage, *GetNameSafe(InstigatorActor));
    Attributes->ApplyDamage(Damage);
}
