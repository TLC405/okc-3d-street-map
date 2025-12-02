// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#include "PlayerCar.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Game/OKCGameMode.h"

APlayerCar::APlayerCar()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    bIsFirstPersonView = false;
    CurrentSpeed = 0.0f;
    MaxSpeed = 150.0f;          // MPH
    AccelerationRate = 50.0f;
    DecelerationRate = 30.0f;
    SteeringSpeed = 2.0f;
    
    // Downtown OKC coordinates
    MapCenterLatitude = 35.4676f;
    MapCenterLongitude = -97.5164f;
    MetersPerDegree = 111320.0f; // Approximate meters per degree at this latitude

    // Mobile input defaults
    MobileThrottleInput = 0.0f;
    MobileSteeringInput = 0.0f;

    // Input defaults
    ThrottleInput = 0.0f;
    BrakeInput = 0.0f;
    SteeringInput = 0.0f;

    // Create Third Person Spring Arm
    ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
    ThirdPersonSpringArm->SetupAttachment(RootComponent);
    ThirdPersonSpringArm->TargetArmLength = 600.0f;
    ThirdPersonSpringArm->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));
    ThirdPersonSpringArm->bUsePawnControlRotation = false;
    ThirdPersonSpringArm->bInheritPitch = false;
    ThirdPersonSpringArm->bInheritYaw = true;
    ThirdPersonSpringArm->bInheritRoll = false;
    ThirdPersonSpringArm->bDoCollisionTest = true;
    ThirdPersonSpringArm->bEnableCameraLag = true;
    ThirdPersonSpringArm->CameraLagSpeed = 10.0f;

    // Create Third Person Camera
    ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    ThirdPersonCamera->SetupAttachment(ThirdPersonSpringArm, USpringArmComponent::SocketName);
    ThirdPersonCamera->bUsePawnControlRotation = false;

    // Create First Person Camera (inside car hood area)
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(RootComponent);
    FirstPersonCamera->SetRelativeLocation(FVector(50.0f, 0.0f, 120.0f));
    FirstPersonCamera->SetRelativeRotation(FRotator(-5.0f, 0.0f, 0.0f));
    FirstPersonCamera->bUsePawnControlRotation = false;

    // Set default camera
    ThirdPersonCamera->SetActive(true);
    FirstPersonCamera->SetActive(false);
}

void APlayerCar::BeginPlay()
{
    Super::BeginPlay();

    // Setup Enhanced Input
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (VehicleMappingContext)
            {
                Subsystem->AddMappingContext(VehicleMappingContext, 0);
            }
        }
    }

    // Get reference to game mode
    GameModeRef = Cast<AOKCGameMode>(UGameplayStatics::GetGameMode(this));

    // Initialize coordinates
    UpdateCoordinates();
}

void APlayerCar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Setup Enhanced Input bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (ThrottleAction)
        {
            EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &APlayerCar::HandleThrottle);
            EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &APlayerCar::HandleThrottle);
        }

        if (BrakeAction)
        {
            EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &APlayerCar::HandleBrake);
            EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &APlayerCar::HandleBrake);
        }

        if (SteerAction)
        {
            EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Triggered, this, &APlayerCar::HandleSteering);
            EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Completed, this, &APlayerCar::HandleSteering);
        }

        if (CameraToggleAction)
        {
            EnhancedInputComponent->BindAction(CameraToggleAction, ETriggerEvent::Started, this, &APlayerCar::HandleCameraToggle);
        }
    }
}

void APlayerCar::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ApplyInput();
    UpdateVehicleStats();
    UpdateCoordinates();
}

void APlayerCar::HandleThrottle(const FInputActionValue& Value)
{
    ThrottleInput = Value.Get<float>();
}

void APlayerCar::HandleBrake(const FInputActionValue& Value)
{
    BrakeInput = Value.Get<float>();
}

void APlayerCar::HandleSteering(const FInputActionValue& Value)
{
    SteeringInput = Value.Get<float>();
}

void APlayerCar::HandleCameraToggle(const FInputActionValue& Value)
{
    ToggleCameraMode();
}

void APlayerCar::ApplyInput()
{
    // Get vehicle movement component
    UChaosWheeledVehicleMovementComponent* VehicleMovement = 
        Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
    
    if (VehicleMovement)
    {
        // Combine desktop and mobile input
        float FinalThrottle = FMath::Clamp(ThrottleInput + MobileThrottleInput, -1.0f, 1.0f);
        float FinalSteering = FMath::Clamp(SteeringInput + MobileSteeringInput, -1.0f, 1.0f);
        float FinalBrake = BrakeInput;

        // Handle reverse (negative throttle)
        if (FinalThrottle < 0.0f)
        {
            FinalBrake = FMath::Abs(FinalThrottle);
            FinalThrottle = 0.0f;
            
            // If nearly stopped, actually reverse
            if (CurrentSpeed < 5.0f)
            {
                FinalThrottle = FMath::Abs(MobileThrottleInput + ThrottleInput) * -1.0f;
                FinalBrake = 0.0f;
            }
        }

        // Apply inputs to vehicle
        VehicleMovement->SetThrottleInput(FinalThrottle);
        VehicleMovement->SetSteeringInput(FinalSteering);
        VehicleMovement->SetBrakeInput(FinalBrake);
    }
}

void APlayerCar::UpdateVehicleStats()
{
    // Calculate speed in MPH
    FVector Velocity = GetVelocity();
    float SpeedCmPerSec = Velocity.Size();
    CurrentSpeed = SpeedCmPerSec * 0.0223694f; // Convert cm/s to MPH

    // Update location
    CurrentLocation = GetActorLocation();
}

void APlayerCar::UpdateCoordinates()
{
    // Convert Unreal coordinates to lat/long
    // Unreal uses centimeters, and we need to convert to degrees
    FVector Location = GetActorLocation();
    
    // Convert X/Y to lat/long offset from center
    // X is East-West (Longitude), Y is North-South (Latitude) in UE4/5
    float LatOffset = (Location.Y / 100.0f) / MetersPerDegree; // Y to Latitude
    float LongOffset = (Location.X / 100.0f) / (MetersPerDegree * FMath::Cos(FMath::DegreesToRadians(MapCenterLatitude))); // X to Longitude
    
    CurrentLatitude = MapCenterLatitude + LatOffset;
    CurrentLongitude = MapCenterLongitude + LongOffset;
}

void APlayerCar::ToggleCameraMode()
{
    bIsFirstPersonView = !bIsFirstPersonView;

    if (bIsFirstPersonView)
    {
        ThirdPersonCamera->SetActive(false);
        FirstPersonCamera->SetActive(true);
    }
    else
    {
        FirstPersonCamera->SetActive(false);
        ThirdPersonCamera->SetActive(true);
    }
}

float APlayerCar::GetSpeedMPH() const
{
    return CurrentSpeed;
}

float APlayerCar::GetSpeedKMH() const
{
    return CurrentSpeed * 1.60934f;
}

void APlayerCar::SetMobileInput(float Throttle, float Steering)
{
    MobileThrottleInput = FMath::Clamp(Throttle, -1.0f, 1.0f);
    MobileSteeringInput = FMath::Clamp(Steering, -1.0f, 1.0f);
}

void APlayerCar::GetCurrentCoordinates(float& Latitude, float& Longitude) const
{
    Latitude = CurrentLatitude;
    Longitude = CurrentLongitude;
}
