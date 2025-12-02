// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#include "GameHUD.h"
#include "../Player/PlayerCar.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

AOKCGameHUD::AOKCGameHUD()
{
    // Default colors
    TextColor = FLinearColor::White;
    BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.7f);
    AccentColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

    // Default display settings
    bShowSpeed = true;
    bShowCoordinates = true;
    bShowMinimap = true;
    bShowCameraMode = true;
    bUseMPH = true;

    // Minimap settings
    MinimapSize = 200.0f;
    MinimapScale = 0.01f;
    MinimapPosition = FVector2D(50.0f, 50.0f);

    // Initialize
    PlayerCar = nullptr;
    HUDWidgetInstance = nullptr;
    HUDFont = nullptr;
    ScreenWidth = 1920.0f;
    ScreenHeight = 1080.0f;
}

void AOKCGameHUD::BeginPlay()
{
    Super::BeginPlay();

    // Get player car reference
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    PlayerCar = Cast<APlayerCar>(PlayerPawn);

    // Load default font
    HUDFont = GEngine->GetMediumFont();

    // Create HUD widget if class is set
    if (HUDWidgetClass)
    {
        APlayerController* PC = Cast<APlayerController>(GetOwningPlayerController());
        if (PC)
        {
            HUDWidgetInstance = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
            if (HUDWidgetInstance)
            {
                HUDWidgetInstance->AddToViewport();
            }
        }
    }
}

void AOKCGameHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas)
    {
        return;
    }

    // Update screen dimensions
    ScreenWidth = Canvas->SizeX;
    ScreenHeight = Canvas->SizeY;

    // Update player car reference if needed
    if (!PlayerCar)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        PlayerCar = Cast<APlayerCar>(PlayerPawn);
    }

    // Draw HUD elements
    if (bShowSpeed)
    {
        DrawSpeedometer();
    }

    if (bShowCoordinates)
    {
        DrawCoordinates();
    }

    if (bShowMinimap)
    {
        DrawMinimap();
    }

    if (bShowCameraMode)
    {
        DrawCameraMode();
    }

    DrawControlHints();
}

void AOKCGameHUD::DrawSpeedometer()
{
    if (!PlayerCar || !HUDFont)
    {
        return;
    }

    float Speed = bUseMPH ? PlayerCar->GetSpeedMPH() : PlayerCar->GetSpeedKMH();
    FString SpeedText = FString::Printf(TEXT("%.0f %s"), Speed, bUseMPH ? TEXT("MPH") : TEXT("KM/H"));

    // Position in bottom right
    float X = ScreenWidth - 250.0f;
    float Y = ScreenHeight - 100.0f;

    // Draw background
    DrawBackground(X - 10.0f, Y - 10.0f, 230.0f, 80.0f, 0.7f);

    // Draw speed text with large font
    DrawTextWithShadow(SpeedText, X, Y, AccentColor, 2.0f);
}

void AOKCGameHUD::DrawCoordinates()
{
    if (!PlayerCar || !HUDFont)
    {
        return;
    }

    float Latitude, Longitude;
    PlayerCar->GetCurrentCoordinates(Latitude, Longitude);

    FString LatText = FString::Printf(TEXT("Lat: %.6f°"), Latitude);
    FString LonText = FString::Printf(TEXT("Lon: %.6f°"), Longitude);

    // Position in bottom left
    float X = 50.0f;
    float Y = ScreenHeight - 100.0f;

    // Draw background
    DrawBackground(X - 10.0f, Y - 10.0f, 250.0f, 80.0f, 0.7f);

    // Draw coordinate texts
    DrawTextWithShadow(LatText, X, Y, TextColor, 1.2f);
    DrawTextWithShadow(LonText, X, Y + 30.0f, TextColor, 1.2f);
}

void AOKCGameHUD::DrawMinimap()
{
    if (!Canvas)
    {
        return;
    }

    // Position in top right
    float X = ScreenWidth - MinimapSize - MinimapPosition.X;
    float Y = MinimapPosition.Y;

    // Draw minimap background (circle approximation using filled rect with rounding)
    FCanvasTileItem BackgroundTile(
        FVector2D(X, Y),
        FVector2D(MinimapSize, MinimapSize),
        FLinearColor(0.1f, 0.1f, 0.1f, 0.8f)
    );
    Canvas->DrawItem(BackgroundTile);

    // Draw minimap border
    FCanvasLineItem BorderLine;
    BorderLine.SetColor(AccentColor);
    BorderLine.LineThickness = 2.0f;

    // Draw rectangular border
    Canvas->DrawLine(X, Y, X + MinimapSize, Y, AccentColor, 2.0f);
    Canvas->DrawLine(X + MinimapSize, Y, X + MinimapSize, Y + MinimapSize, AccentColor, 2.0f);
    Canvas->DrawLine(X + MinimapSize, Y + MinimapSize, X, Y + MinimapSize, AccentColor, 2.0f);
    Canvas->DrawLine(X, Y + MinimapSize, X, Y, AccentColor, 2.0f);

    // Draw player position indicator (center of minimap)
    float CenterX = X + MinimapSize / 2.0f;
    float CenterY = Y + MinimapSize / 2.0f;

    // Draw player arrow
    float ArrowSize = 10.0f;
    float PlayerYaw = 0.0f;
    if (PlayerCar)
    {
        PlayerYaw = FMath::DegreesToRadians(PlayerCar->GetActorRotation().Yaw);
    }

    // Arrow points (pointing up by default, rotated by player yaw)
    FVector2D ArrowTip(CenterX + FMath::Sin(PlayerYaw) * ArrowSize * 2.0f,
                       CenterY - FMath::Cos(PlayerYaw) * ArrowSize * 2.0f);
    FVector2D ArrowLeft(CenterX + FMath::Sin(PlayerYaw + 2.4f) * ArrowSize,
                        CenterY - FMath::Cos(PlayerYaw + 2.4f) * ArrowSize);
    FVector2D ArrowRight(CenterX + FMath::Sin(PlayerYaw - 2.4f) * ArrowSize,
                         CenterY - FMath::Cos(PlayerYaw - 2.4f) * ArrowSize);

    // Draw player arrow
    Canvas->DrawLine(ArrowTip.X, ArrowTip.Y, ArrowLeft.X, ArrowLeft.Y, FLinearColor::Green, 3.0f);
    Canvas->DrawLine(ArrowTip.X, ArrowTip.Y, ArrowRight.X, ArrowRight.Y, FLinearColor::Green, 3.0f);
    Canvas->DrawLine(ArrowLeft.X, ArrowLeft.Y, ArrowRight.X, ArrowRight.Y, FLinearColor::Green, 3.0f);

    // Draw compass directions
    DrawTextWithShadow(TEXT("N"), CenterX - 5.0f, Y + 5.0f, TextColor, 0.8f);
    DrawTextWithShadow(TEXT("S"), CenterX - 5.0f, Y + MinimapSize - 20.0f, TextColor, 0.8f);
    DrawTextWithShadow(TEXT("E"), X + MinimapSize - 15.0f, CenterY - 8.0f, TextColor, 0.8f);
    DrawTextWithShadow(TEXT("W"), X + 5.0f, CenterY - 8.0f, TextColor, 0.8f);
}

void AOKCGameHUD::DrawCameraMode()
{
    if (!PlayerCar || !HUDFont)
    {
        return;
    }

    FString ModeText = PlayerCar->bIsFirstPersonView ? TEXT("First Person") : TEXT("Third Person");

    // Position in top left
    float X = 50.0f;
    float Y = 50.0f;

    // Draw background
    DrawBackground(X - 10.0f, Y - 10.0f, 180.0f, 40.0f, 0.7f);

    // Draw mode text
    DrawTextWithShadow(ModeText, X, Y, AccentColor, 1.0f);
}

void AOKCGameHUD::DrawControlHints()
{
    if (!HUDFont)
    {
        return;
    }

    // Position in bottom center
    float X = ScreenWidth / 2.0f - 150.0f;
    float Y = ScreenHeight - 50.0f;

    FString HintText = TEXT("Arrow Keys: Drive | C: Camera | Tab: Minimap");

    // Draw background
    DrawBackground(X - 10.0f, Y - 10.0f, 350.0f, 35.0f, 0.5f);

    // Draw hint text
    DrawTextWithShadow(HintText, X, Y, TextColor, 0.8f);
}

void AOKCGameHUD::ToggleMinimap()
{
    bShowMinimap = !bShowMinimap;
}

void AOKCGameHUD::ToggleSpeedUnit()
{
    bUseMPH = !bUseMPH;
}

void AOKCGameHUD::SetMinimapVisible(bool bVisible)
{
    bShowMinimap = bVisible;
}

FVector2D AOKCGameHUD::GetScreenCenter() const
{
    return FVector2D(ScreenWidth / 2.0f, ScreenHeight / 2.0f);
}

void AOKCGameHUD::DrawBackground(float X, float Y, float Width, float Height, float Opacity)
{
    if (!Canvas)
    {
        return;
    }

    FCanvasTileItem TileItem(
        FVector2D(X, Y),
        FVector2D(Width, Height),
        FLinearColor(BackgroundColor.R, BackgroundColor.G, BackgroundColor.B, Opacity)
    );
    Canvas->DrawItem(TileItem);
}

void AOKCGameHUD::DrawTextWithShadow(const FString& Text, float X, float Y, FLinearColor Color, float Scale)
{
    if (!Canvas || !HUDFont)
    {
        return;
    }

    // Draw shadow
    FCanvasTextItem ShadowTextItem(
        FVector2D(X + 2.0f, Y + 2.0f),
        FText::FromString(Text),
        HUDFont,
        FLinearColor::Black
    );
    ShadowTextItem.Scale = FVector2D(Scale, Scale);
    Canvas->DrawItem(ShadowTextItem);

    // Draw main text
    FCanvasTextItem TextItem(
        FVector2D(X, Y),
        FText::FromString(Text),
        HUDFont,
        Color
    );
    TextItem.Scale = FVector2D(Scale, Scale);
    Canvas->DrawItem(TextItem);
}
