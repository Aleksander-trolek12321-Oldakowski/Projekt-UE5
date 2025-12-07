#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UMainHUD.h"
#include "APlayerController.generated.h"


UCLASS()
class PROJEKT_UE5_API AAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    AAPlayerController();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
    TSubclassOf<UMainHUD> MainHUDClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
    UMainHUD* MainHUDWidget;
	
};
