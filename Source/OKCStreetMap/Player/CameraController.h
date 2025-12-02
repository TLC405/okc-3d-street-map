// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraController.generated.h"

class UCameraComponent;
class USpringArmComponent;
class APlayerCar;

/**
 * Camera controller component for managing vehicle camera behavior.
 * Handles camera input, smoothing, and mobile swipe controls.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OKCSTREETMAP_API UCameraController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCameraController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                               FActorComponentTickFunction* ThisTickFunction) override;

    // Camera Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraRotationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MinPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MaxPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float DefaultDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MaxDistance;

    // Mobile Touch Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobile")
    float TouchSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobile")
    float PinchZoomSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobile")
    bool bInvertYAxis;

    // Current Camera State
    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    float CurrentYaw;

    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    float CurrentPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    float CurrentDistance;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void RotateCamera(float DeltaYaw, float DeltaPitch);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ZoomCamera(float ZoomDelta);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ResetCamera();

    UFUNCTION(BlueprintCallable, Category = "Mobile")
    void HandleTouchDrag(FVector2D DragDelta);

    UFUNCTION(BlueprintCallable, Category = "Mobile")
    void HandlePinchZoom(float PinchDelta);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetSpringArmComponent(USpringArmComponent* InSpringArm);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void LookBehind(bool bEnable);

protected:
    // Internal functions
    void UpdateCameraPosition(float DeltaTime);
    void ApplyCameraLimits();

private:
    // Cached references
    UPROPERTY()
    USpringArmComponent* SpringArmRef;

    UPROPERTY()
    APlayerCar* OwnerVehicle;

    // Target values for smooth interpolation
    float TargetYaw;
    float TargetPitch;
    float TargetDistance;

    // Look behind state
    bool bLookingBehind;
    float PreLookBehindYaw;
};
