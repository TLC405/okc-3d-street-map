// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#include "CameraController.h"
#include "PlayerCar.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"

UCameraController::UCameraController()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Default camera settings
    CameraRotationSpeed = 100.0f;
    CameraSmoothingSpeed = 10.0f;
    MinPitch = -60.0f;
    MaxPitch = 10.0f;
    DefaultDistance = 600.0f;
    MinDistance = 200.0f;
    MaxDistance = 1500.0f;

    // Mobile settings
    TouchSensitivity = 0.5f;
    PinchZoomSpeed = 2.0f;
    bInvertYAxis = false;

    // Initialize state
    CurrentYaw = 0.0f;
    CurrentPitch = -20.0f;
    CurrentDistance = DefaultDistance;
    TargetYaw = 0.0f;
    TargetPitch = -20.0f;
    TargetDistance = DefaultDistance;

    bLookingBehind = false;
    PreLookBehindYaw = 0.0f;

    SpringArmRef = nullptr;
    OwnerVehicle = nullptr;
}

void UCameraController::BeginPlay()
{
    Super::BeginPlay();

    // Get owner vehicle reference
    OwnerVehicle = Cast<APlayerCar>(GetOwner());

    // Initialize spring arm reference if owner has one
    if (OwnerVehicle && OwnerVehicle->ThirdPersonSpringArm)
    {
        SpringArmRef = OwnerVehicle->ThirdPersonSpringArm;
        CurrentDistance = SpringArmRef->TargetArmLength;
        TargetDistance = CurrentDistance;
    }
}

void UCameraController::TickComponent(float DeltaTime, ELevelTick TickType, 
                                      FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateCameraPosition(DeltaTime);
}

void UCameraController::RotateCamera(float DeltaYaw, float DeltaPitch)
{
    if (bLookingBehind)
    {
        return; // Don't allow rotation while looking behind
    }

    TargetYaw += DeltaYaw * CameraRotationSpeed * GetWorld()->GetDeltaSeconds();
    TargetPitch += (bInvertYAxis ? DeltaPitch : -DeltaPitch) * CameraRotationSpeed * GetWorld()->GetDeltaSeconds();

    ApplyCameraLimits();
}

void UCameraController::ZoomCamera(float ZoomDelta)
{
    TargetDistance = FMath::Clamp(TargetDistance - ZoomDelta * 50.0f, MinDistance, MaxDistance);
}

void UCameraController::ResetCamera()
{
    TargetYaw = 0.0f;
    TargetPitch = -20.0f;
    TargetDistance = DefaultDistance;
    bLookingBehind = false;
}

void UCameraController::HandleTouchDrag(FVector2D DragDelta)
{
    if (bLookingBehind)
    {
        return;
    }

    float DeltaYaw = DragDelta.X * TouchSensitivity;
    float DeltaPitch = DragDelta.Y * TouchSensitivity;

    TargetYaw += DeltaYaw;
    TargetPitch += bInvertYAxis ? DeltaPitch : -DeltaPitch;

    ApplyCameraLimits();
}

void UCameraController::HandlePinchZoom(float PinchDelta)
{
    TargetDistance = FMath::Clamp(TargetDistance - PinchDelta * PinchZoomSpeed, MinDistance, MaxDistance);
}

void UCameraController::SetSpringArmComponent(USpringArmComponent* InSpringArm)
{
    SpringArmRef = InSpringArm;
    if (SpringArmRef)
    {
        CurrentDistance = SpringArmRef->TargetArmLength;
        TargetDistance = CurrentDistance;
    }
}

void UCameraController::LookBehind(bool bEnable)
{
    if (bEnable && !bLookingBehind)
    {
        // Store current yaw and look behind
        PreLookBehindYaw = TargetYaw;
        TargetYaw = TargetYaw + 180.0f;
        bLookingBehind = true;
    }
    else if (!bEnable && bLookingBehind)
    {
        // Restore previous yaw
        TargetYaw = PreLookBehindYaw;
        bLookingBehind = false;
    }
}

void UCameraController::UpdateCameraPosition(float DeltaTime)
{
    if (!SpringArmRef)
    {
        return;
    }

    // Smoothly interpolate current values to target values
    CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaTime, CameraSmoothingSpeed);
    CurrentPitch = FMath::FInterpTo(CurrentPitch, TargetPitch, DeltaTime, CameraSmoothingSpeed);
    CurrentDistance = FMath::FInterpTo(CurrentDistance, TargetDistance, DeltaTime, CameraSmoothingSpeed);

    // Apply to spring arm
    FRotator NewRotation = FRotator(CurrentPitch, CurrentYaw, 0.0f);
    SpringArmRef->SetRelativeRotation(NewRotation);
    SpringArmRef->TargetArmLength = CurrentDistance;
}

void UCameraController::ApplyCameraLimits()
{
    // Limit pitch
    TargetPitch = FMath::Clamp(TargetPitch, MinPitch, MaxPitch);

    // Normalize yaw
    while (TargetYaw > 180.0f) TargetYaw -= 360.0f;
    while (TargetYaw < -180.0f) TargetYaw += 360.0f;

    // Limit distance
    TargetDistance = FMath::Clamp(TargetDistance, MinDistance, MaxDistance);
}
