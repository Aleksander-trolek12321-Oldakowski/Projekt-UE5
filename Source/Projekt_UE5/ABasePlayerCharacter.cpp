// Fill out your copyright notice in the Description page of Project Settings.


#include "ABasePlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"

AABasePlayerCharacter::AABasePlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}



void AABasePlayerCharacter::BeginPlay()
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
                    // priorytet 0 (możesz zmienić)
                    Subsystem->AddMappingContext(DefaultMappingContext, 0);
                }
            }
        }
    }
}

void AABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Move)     EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AABasePlayerCharacter::Move);
        if (IA_Interact) EIC->BindAction(IA_Interact, ETriggerEvent::Started,   this, &AABasePlayerCharacter::OnInteract);
        if (IA_Attack)   EIC->BindAction(IA_Attack, ETriggerEvent::Started,     this, &AABasePlayerCharacter::OnAttack);
    }
}

void AABasePlayerCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    AddMovementInput(GetActorForwardVector(), MovementVector.Y);
    AddMovementInput(GetActorRightVector(),   MovementVector.X);
}

void AABasePlayerCharacter::OnInteract(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Log, TEXT("Interact pressed"));
}

void AABasePlayerCharacter::OnAttack(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Log, TEXT("Attack pressed"));
}