#include "APlayerController.h"
#include "Blueprint/UserWidget.h"

AAPlayerController::AAPlayerController()
{
    bShowMouseCursor = false;
}

void AAPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (MainHUDClass)
    {
        MainHUDWidget = CreateWidget<UMainHUD>(this, MainHUDClass);
        if (MainHUDWidget)
        {
            MainHUDWidget->AddToViewport();
        }
    }
}
