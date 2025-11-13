#include "ABasePlayerCharacter.h"
#include "UInteractionComponent.h"
#include "WeaponBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"

ABasePlayerCharacter::ABasePlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

    CurrentWeapon = nullptr;
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
    UE_LOG(LogTemp, Log, TEXT("Attack pressed"));
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
