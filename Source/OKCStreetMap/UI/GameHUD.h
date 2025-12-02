// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

class UUserWidget;
class APlayerCar;

/**
 * HUD class for OKC Street Map game.
 * Displays speed, location, minimap, and camera mode.
 */
UCLASS()
class OKCSTREETMAP_API AOKCGameHUD : public AHUD
{
    GENERATED_BODY()

public:
    AOKCGameHUD();

protected:
    virtual void BeginPlay() override;

public:
    virtual void DrawHUD() override;

    // HUD Widget class
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    // Colors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FLinearColor TextColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FLinearColor BackgroundColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FLinearColor AccentColor;

    // Display Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bShowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bShowCoordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bShowMinimap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bShowCameraMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bUseMPH;

    // Minimap Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MinimapSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MinimapScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    FVector2D MinimapPosition;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ToggleMinimap();

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ToggleSpeedUnit();

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetMinimapVisible(bool bVisible);

protected:
    // Drawing functions
    void DrawSpeedometer();
    void DrawCoordinates();
    void DrawMinimap();
    void DrawCameraMode();
    void DrawControlHints();

    // Helper functions
    FVector2D GetScreenCenter() const;
    void DrawBackground(float X, float Y, float Width, float Height, float Opacity);
    void DrawTextWithShadow(const FString& Text, float X, float Y, FLinearColor Color, float Scale = 1.0f);

private:
    // Cached references
    UPROPERTY()
    APlayerCar* PlayerCar;

    UPROPERTY()
    UUserWidget* HUDWidgetInstance;

    // Screen dimensions
    float ScreenWidth;
    float ScreenHeight;

    // Cached fonts
    UFont* HUDFont;
};
