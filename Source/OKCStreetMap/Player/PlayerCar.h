// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "InputActionValue.h"
#include "PlayerCar.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class AOKCGameMode;

/**
 * Player-controlled vehicle for OKC Street Map driving game.
 * Supports both desktop (arrow keys) and mobile (virtual joystick) input.
 */
UCLASS()
class OKCSTREETMAP_API APlayerCar : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    APlayerCar();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Camera Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* ThirdPersonSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* ThirdPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FirstPersonCamera;

    // Camera Mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bIsFirstPersonView;

    // Vehicle Stats
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    float CurrentSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    FVector CurrentLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    float CurrentLatitude;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
    float CurrentLongitude;

    // Input Actions - Enhanced Input System
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* VehicleMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ThrottleAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* BrakeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SteerAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CameraToggleAction;

    // Mobile Input Support
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobile")
    float MobileThrottleInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobile")
    float MobileSteeringInput;

    // Vehicle Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
    float AccelerationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
    float DecelerationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Settings")
    float SteeringSpeed;

    // Map Coordinates (Downtown OKC center)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    float MapCenterLatitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    float MapCenterLongitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    float MetersPerDegree;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ToggleCameraMode();

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    float GetSpeedMPH() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    float GetSpeedKMH() const;

    UFUNCTION(BlueprintCallable, Category = "Mobile")
    void SetMobileInput(float Throttle, float Steering);

    UFUNCTION(BlueprintCallable, Category = "Map")
    void GetCurrentCoordinates(float& Latitude, float& Longitude) const;

protected:
    // Input Handlers
    void HandleThrottle(const FInputActionValue& Value);
    void HandleBrake(const FInputActionValue& Value);
    void HandleSteering(const FInputActionValue& Value);
    void HandleCameraToggle(const FInputActionValue& Value);

    // Update Functions
    void UpdateVehicleStats();
    void UpdateCoordinates();
    void ApplyInput();

private:
    // Input values
    float ThrottleInput;
    float BrakeInput;
    float SteeringInput;

    // Reference to game mode
    UPROPERTY()
    AOKCGameMode* GameModeRef;
};
