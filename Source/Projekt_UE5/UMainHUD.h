#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SharedTypes.h"
#include "UMainHUD.generated.h"

UCLASS()
class PROJEKT_UE5_API UMainHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category="HUD")
    void UpdateHealth(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent, Category="HUD")
    void UpdateStamina(float Current, float Max);

    UFUNCTION(BlueprintImplementableEvent, Category="HUD")
    void UpdatePawnState(const EPawnState NewState);
};
