// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ABaseCharacter.h"
#include "InputActionValue.h"
#include "ABasePlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
UCLASS()
class PROJEKT_UE5_API AABasePlayerCharacter : public AABaseCharacter
{
	GENERATED_BODY()

	public:
	AABasePlayerCharacter();

	protected:
		virtual void BeginPlay() override;
		virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

		// handlers
		void Move(const FInputActionValue& Value);
		void OnInteract(const FInputActionValue& Value);
		void OnAttack(const FInputActionValue& Value);

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* IA_Interact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* IA_Attack;
};
