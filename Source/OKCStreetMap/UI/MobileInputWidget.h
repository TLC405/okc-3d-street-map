// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MobileInputWidget.generated.h"

class UImage;
class UBorder;
class UButton;
class UCanvasPanel;
class APlayerCar;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoystickInput, float, X, float, Y);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraToggle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinimapToggle);

/**
 * Mobile input widget providing virtual joystick and touch buttons.
 * Supports both portrait and landscape orientations.
 */
UCLASS()
class OKCSTREETMAP_API UMobileInputWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UMobileInputWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

public:
    // Joystick Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget), Category = "Joystick")
    UImage* JoystickBackground;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget), Category = "Joystick")
    UImage* JoystickThumb;

    // Camera Control Area
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget), Category = "Camera")
    UBorder* CameraControlArea;

    // Buttons
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget), Category = "Buttons")
    UButton* CameraToggleButton;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget), Category = "Buttons")
    UButton* MinimapToggleButton;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget), Category = "Buttons")
    UButton* BrakeButton;

    // Joystick Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joystick")
    float JoystickRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joystick")
    float DeadZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joystick")
    bool bJoystickActive;

    // Camera Swipe Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraSwipeSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bCameraControlActive;

    // Current Input Values
    UPROPERTY(BlueprintReadOnly, Category = "Input")
    float JoystickInputX;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    float JoystickInputY;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    float CameraInputX;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    float CameraInputY;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnJoystickInput OnJoystickInput;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCameraToggle OnCameraToggle;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMinimapToggle OnMinimapToggle;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Input")
    void GetJoystickInput(float& OutX, float& OutY) const;

    UFUNCTION(BlueprintCallable, Category = "Input")
    void GetCameraInput(float& OutX, float& OutY) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetJoystickVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetButtonsVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void AdaptToOrientation(bool bIsLandscape);

    UFUNCTION(BlueprintCallable, Category = "Input")
    bool IsBraking() const { return bBraking; }

protected:
    // Button callbacks
    UFUNCTION()
    void OnCameraToggleClicked();

    UFUNCTION()
    void OnMinimapToggleClicked();

    UFUNCTION()
    void OnBrakePressed();

    UFUNCTION()
    void OnBrakeReleased();

    // Joystick handling
    void UpdateJoystickPosition(const FVector2D& TouchPosition);
    void ResetJoystick();
    bool IsInJoystickArea(const FVector2D& Position) const;
    bool IsInCameraArea(const FVector2D& Position) const;

    // Camera swipe handling
    void UpdateCameraSwipe(const FVector2D& DeltaPosition);
    void ResetCameraInput();

private:
    // Touch tracking
    TMap<int32, FVector2D> ActiveTouches;
    int32 JoystickTouchIndex;
    int32 CameraTouchIndex;
    FVector2D JoystickCenter;
    FVector2D LastCameraTouchPosition;

    // State
    bool bBraking;

    // Cached reference
    UPROPERTY()
    APlayerCar* PlayerCar;
};
