// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#include "MobileInputWidget.h"
#include "../Player/PlayerCar.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UMobileInputWidget::UMobileInputWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Default joystick settings
    JoystickRadius = 100.0f;
    DeadZone = 0.15f;
    bJoystickActive = false;

    // Default camera settings
    CameraSwipeSensitivity = 1.0f;
    bCameraControlActive = false;

    // Initialize input values
    JoystickInputX = 0.0f;
    JoystickInputY = 0.0f;
    CameraInputX = 0.0f;
    CameraInputY = 0.0f;

    // Touch tracking
    JoystickTouchIndex = -1;
    CameraTouchIndex = -1;
    JoystickCenter = FVector2D::ZeroVector;
    LastCameraTouchPosition = FVector2D::ZeroVector;

    // State
    bBraking = false;
    PlayerCar = nullptr;
}

void UMobileInputWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get player car reference
    APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    PlayerCar = Cast<APlayerCar>(Pawn);

    // Bind button callbacks
    if (CameraToggleButton)
    {
        CameraToggleButton->OnClicked.AddDynamic(this, &UMobileInputWidget::OnCameraToggleClicked);
    }

    if (MinimapToggleButton)
    {
        MinimapToggleButton->OnClicked.AddDynamic(this, &UMobileInputWidget::OnMinimapToggleClicked);
    }

    if (BrakeButton)
    {
        BrakeButton->OnPressed.AddDynamic(this, &UMobileInputWidget::OnBrakePressed);
        BrakeButton->OnReleased.AddDynamic(this, &UMobileInputWidget::OnBrakeReleased);
    }

    // Initialize joystick center position
    if (JoystickBackground)
    {
        FGeometry Geometry = JoystickBackground->GetCachedGeometry();
        JoystickCenter = Geometry.GetAbsolutePosition() + Geometry.GetAbsoluteSize() / 2.0f;
    }

    // Initially hide joystick thumb
    if (JoystickThumb)
    {
        JoystickThumb->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UMobileInputWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Apply joystick input to player car
    if (PlayerCar)
    {
        // Use Y for throttle (up = forward), X for steering
        PlayerCar->SetMobileInput(-JoystickInputY, JoystickInputX);
    }

    // Decay camera input
    CameraInputX = FMath::FInterpTo(CameraInputX, 0.0f, InDeltaTime, 5.0f);
    CameraInputY = FMath::FInterpTo(CameraInputY, 0.0f, InDeltaTime, 5.0f);
}

FReply UMobileInputWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
    int32 TouchIndex = InGestureEvent.GetPointerIndex();
    FVector2D TouchPosition = InGestureEvent.GetScreenSpacePosition();

    ActiveTouches.Add(TouchIndex, TouchPosition);

    // Check if touch is in joystick area
    if (JoystickTouchIndex == -1 && IsInJoystickArea(TouchPosition))
    {
        JoystickTouchIndex = TouchIndex;
        bJoystickActive = true;

        // Update joystick center to touch position for dynamic joystick
        if (JoystickBackground)
        {
            JoystickCenter = TouchPosition;
        }

        // Show joystick thumb
        if (JoystickThumb)
        {
            JoystickThumb->SetVisibility(ESlateVisibility::Visible);
        }

        UpdateJoystickPosition(TouchPosition);
        return FReply::Handled();
    }

    // Check if touch is in camera control area
    if (CameraTouchIndex == -1 && IsInCameraArea(TouchPosition))
    {
        CameraTouchIndex = TouchIndex;
        bCameraControlActive = true;
        LastCameraTouchPosition = TouchPosition;
        return FReply::Handled();
    }

    return FReply::Handled();
}

FReply UMobileInputWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
    int32 TouchIndex = InGestureEvent.GetPointerIndex();
    FVector2D TouchPosition = InGestureEvent.GetScreenSpacePosition();

    if (ActiveTouches.Contains(TouchIndex))
    {
        FVector2D PreviousPosition = ActiveTouches[TouchIndex];
        ActiveTouches[TouchIndex] = TouchPosition;

        // Handle joystick movement
        if (TouchIndex == JoystickTouchIndex)
        {
            UpdateJoystickPosition(TouchPosition);
            return FReply::Handled();
        }

        // Handle camera swipe
        if (TouchIndex == CameraTouchIndex)
        {
            FVector2D Delta = TouchPosition - LastCameraTouchPosition;
            UpdateCameraSwipe(Delta);
            LastCameraTouchPosition = TouchPosition;
            return FReply::Handled();
        }
    }

    return FReply::Handled();
}

FReply UMobileInputWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
    int32 TouchIndex = InGestureEvent.GetPointerIndex();

    ActiveTouches.Remove(TouchIndex);

    // Reset joystick if this was the joystick touch
    if (TouchIndex == JoystickTouchIndex)
    {
        JoystickTouchIndex = -1;
        bJoystickActive = false;
        ResetJoystick();
        return FReply::Handled();
    }

    // Reset camera control if this was the camera touch
    if (TouchIndex == CameraTouchIndex)
    {
        CameraTouchIndex = -1;
        bCameraControlActive = false;
        ResetCameraInput();
        return FReply::Handled();
    }

    return FReply::Handled();
}

void UMobileInputWidget::UpdateJoystickPosition(const FVector2D& TouchPosition)
{
    if (!JoystickThumb || !JoystickBackground)
    {
        return;
    }

    // Calculate offset from center
    FVector2D Offset = TouchPosition - JoystickCenter;
    float Distance = Offset.Size();

    // Clamp to joystick radius
    if (Distance > JoystickRadius)
    {
        Offset = Offset.GetSafeNormal() * JoystickRadius;
        Distance = JoystickRadius;
    }

    // Update thumb position
    FGeometry ThumbGeometry = JoystickThumb->GetCachedGeometry();
    FVector2D ThumbSize = ThumbGeometry.GetAbsoluteSize();
    
    // Position thumb at offset from center
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(JoystickThumb->Slot))
    {
        FVector2D LocalOffset = Offset / UWidgetLayoutLibrary::GetViewportScale(this);
        FVector2D CurrentPosition = CanvasSlot->GetPosition();
        CanvasSlot->SetPosition(CurrentPosition + LocalOffset);
    }

    // Calculate normalized input values
    if (Distance > JoystickRadius * DeadZone)
    {
        float NormalizedDistance = (Distance - JoystickRadius * DeadZone) / (JoystickRadius * (1.0f - DeadZone));
        NormalizedDistance = FMath::Clamp(NormalizedDistance, 0.0f, 1.0f);

        FVector2D Direction = Offset.GetSafeNormal();
        JoystickInputX = Direction.X * NormalizedDistance;
        JoystickInputY = Direction.Y * NormalizedDistance;
    }
    else
    {
        JoystickInputX = 0.0f;
        JoystickInputY = 0.0f;
    }

    // Broadcast joystick input
    OnJoystickInput.Broadcast(JoystickInputX, JoystickInputY);
}

void UMobileInputWidget::ResetJoystick()
{
    JoystickInputX = 0.0f;
    JoystickInputY = 0.0f;

    // Hide joystick thumb
    if (JoystickThumb)
    {
        JoystickThumb->SetVisibility(ESlateVisibility::Hidden);
    }

    // Reset thumb position
    if (JoystickThumb)
    {
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(JoystickThumb->Slot))
        {
            // Reset to center of joystick background
            // This would need proper position calculation based on your widget layout
        }
    }

    // Broadcast zero input
    OnJoystickInput.Broadcast(0.0f, 0.0f);
}

void UMobileInputWidget::UpdateCameraSwipe(const FVector2D& DeltaPosition)
{
    CameraInputX = DeltaPosition.X * CameraSwipeSensitivity;
    CameraInputY = DeltaPosition.Y * CameraSwipeSensitivity;
}

void UMobileInputWidget::ResetCameraInput()
{
    CameraInputX = 0.0f;
    CameraInputY = 0.0f;
}

bool UMobileInputWidget::IsInJoystickArea(const FVector2D& Position) const
{
    if (!JoystickBackground)
    {
        // Default: left third of screen is joystick area
        FVector2D ViewportSize;
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        return Position.X < ViewportSize.X * 0.33f;
    }

    FGeometry Geometry = JoystickBackground->GetCachedGeometry();
    FVector2D LocalPosition = Geometry.AbsoluteToLocal(Position);
    FVector2D Size = Geometry.GetLocalSize();

    // Check if within joystick background bounds (with some padding)
    float Padding = JoystickRadius * 0.5f;
    return LocalPosition.X >= -Padding && LocalPosition.X <= Size.X + Padding &&
           LocalPosition.Y >= -Padding && LocalPosition.Y <= Size.Y + Padding;
}

bool UMobileInputWidget::IsInCameraArea(const FVector2D& Position) const
{
    if (CameraControlArea)
    {
        FGeometry Geometry = CameraControlArea->GetCachedGeometry();
        FVector2D LocalPosition = Geometry.AbsoluteToLocal(Position);
        FVector2D Size = Geometry.GetLocalSize();

        return LocalPosition.X >= 0 && LocalPosition.X <= Size.X &&
               LocalPosition.Y >= 0 && LocalPosition.Y <= Size.Y;
    }

    // Default: right two-thirds of screen is camera area
    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);
    return Position.X > ViewportSize.X * 0.33f;
}

void UMobileInputWidget::GetJoystickInput(float& OutX, float& OutY) const
{
    OutX = JoystickInputX;
    OutY = JoystickInputY;
}

void UMobileInputWidget::GetCameraInput(float& OutX, float& OutY) const
{
    OutX = CameraInputX;
    OutY = CameraInputY;
}

void UMobileInputWidget::SetJoystickVisible(bool bVisible)
{
    if (JoystickBackground)
    {
        JoystickBackground->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UMobileInputWidget::SetButtonsVisible(bool bVisible)
{
    ESlateVisibility Visibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

    if (CameraToggleButton)
    {
        CameraToggleButton->SetVisibility(Visibility);
    }

    if (MinimapToggleButton)
    {
        MinimapToggleButton->SetVisibility(Visibility);
    }

    if (BrakeButton)
    {
        BrakeButton->SetVisibility(Visibility);
    }
}

void UMobileInputWidget::AdaptToOrientation(bool bIsLandscape)
{
    // Adjust widget layout based on orientation
    // This would reposition elements for portrait vs landscape mode
    
    if (bIsLandscape)
    {
        // Landscape: joystick on left, buttons on right
        JoystickRadius = 100.0f;
    }
    else
    {
        // Portrait: joystick smaller, buttons at bottom
        JoystickRadius = 80.0f;
    }
}

void UMobileInputWidget::OnCameraToggleClicked()
{
    OnCameraToggle.Broadcast();

    if (PlayerCar)
    {
        PlayerCar->ToggleCameraMode();
    }
}

void UMobileInputWidget::OnMinimapToggleClicked()
{
    OnMinimapToggle.Broadcast();
}

void UMobileInputWidget::OnBrakePressed()
{
    bBraking = true;
}

void UMobileInputWidget::OnBrakeReleased()
{
    bBraking = false;
}
