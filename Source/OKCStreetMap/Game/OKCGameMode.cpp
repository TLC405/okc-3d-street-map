// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#include "OKCGameMode.h"
#include "../Map/MapDataLoader.h"
#include "../Map/BuildingGenerator.h"
#include "../Player/PlayerCar.h"
#include "../UI/GameHUD.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"

AOKCGameMode::AOKCGameMode()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default pawn and HUD classes
    DefaultPawnClass = APlayerCar::StaticClass();
    HUDClass = AOKCGameHUD::StaticClass();

    // Default game settings
    bEnableDayNightCycle = true;
    DayNightCycleSpeed = 0.001f; // Very slow cycle
    CurrentTimeOfDay = 12.0f; // Noon

    // Default map settings
    bGenerateProcedural = true;

    // Default lighting colors
    DayAmbientColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    NightAmbientColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    DayDirectionalIntensity = 5.0f;
    NightDirectionalIntensity = 0.5f;

    // Default spawn location (downtown OKC)
    DefaultSpawnLocation = FVector(0.0f, 0.0f, 100.0f);
    DefaultSpawnRotation = FRotator::ZeroRotator;

    // Initialize pointers
    MapData = nullptr;
    BuildingGenerator = nullptr;
    PlayerCar = nullptr;
    SunLight = nullptr;
    SkyLight = nullptr;
}

void AOKCGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Initialize game systems
    InitializeMap();
    InitializeLighting();
    InitializePlayer();

    UE_LOG(LogTemp, Log, TEXT("OKC Game Mode initialized"));
}

void AOKCGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update day/night cycle
    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void AOKCGameMode::InitializeMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create map data loader
    MapData = NewObject<UMapDataLoader>(this);
    if (MapData)
    {
        // Try to load map data from file, or generate procedurally
        if (!MapDataPath.IsEmpty() && MapData->LoadMapDataFromFile(MapDataPath))
        {
            UE_LOG(LogTemp, Log, TEXT("Loaded map data from: %s"), *MapDataPath);
        }
        else if (bGenerateProcedural)
        {
            MapData->GenerateOKCDowntownData();
            UE_LOG(LogTemp, Log, TEXT("Generated procedural OKC downtown data"));
        }
    }

    // Spawn building generator
    if (BuildingGeneratorClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        BuildingGenerator = World->SpawnActor<ABuildingGenerator>(BuildingGeneratorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }
    else
    {
        // Spawn default building generator
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        BuildingGenerator = World->SpawnActor<ABuildingGenerator>(ABuildingGenerator::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }

    // Generate buildings from map data
    if (BuildingGenerator && MapData)
    {
        BuildingGenerator->bGenerateOnBeginPlay = false; // We'll generate manually
        BuildingGenerator->GenerateBuildingsFromData(MapData);
    }
}

void AOKCGameMode::InitializeLighting()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing directional light
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    else
    {
        // Spawn sun light
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SunLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), 
            FVector::ZeroVector, FRotator(-45.0f, 0.0f, 0.0f), SpawnParams);
    }

    // Find existing sky light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (SkyLights.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(SkyLights[0]);
    }

    // Apply initial lighting
    ApplyLightingForTimeOfDay();
}

void AOKCGameMode::InitializePlayer()
{
    // Get player car reference
    APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    PlayerCar = Cast<APlayerCar>(Pawn);

    if (PlayerCar)
    {
        UE_LOG(LogTemp, Log, TEXT("Player car initialized"));
    }
}

void AOKCGameMode::UpdateDayNightCycle(float DeltaTime)
{
    // Update time of day (0-24 hours)
    CurrentTimeOfDay += DeltaTime * DayNightCycleSpeed * 3600.0f; // Convert to hours
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    ApplyLightingForTimeOfDay();
}

void AOKCGameMode::ApplyLightingForTimeOfDay()
{
    // Calculate sun angle based on time
    float SunAngle = (CurrentTimeOfDay - 6.0f) / 12.0f * 180.0f - 90.0f; // 6 AM = sunrise, 18 = sunset
    SunAngle = FMath::Clamp(SunAngle, -90.0f, 90.0f);

    // Update sun rotation
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(SunAngle, 180.0f, 0.0f));

        // Update sun intensity based on time
        if (UDirectionalLightComponent* LightComponent = SunLight->GetComponent())
        {
            float IntensityFactor = FMath::Cos(FMath::DegreesToRadians(SunAngle));
            IntensityFactor = FMath::Clamp(IntensityFactor, 0.0f, 1.0f);
            
            float Intensity = FMath::Lerp(NightDirectionalIntensity, DayDirectionalIntensity, IntensityFactor);
            LightComponent->SetIntensity(Intensity);

            // Warm color at sunrise/sunset
            float HourFromNoon = FMath::Abs(CurrentTimeOfDay - 12.0f);
            FLinearColor SunColor = FLinearColor::White;
            if (HourFromNoon > 5.0f && HourFromNoon < 7.0f)
            {
                // Golden hour
                SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            }
            LightComponent->SetLightColor(SunColor);
        }
    }

    // Update sky light
    if (SkyLight)
    {
        if (USkyLightComponent* SkyComponent = SkyLight->GetComponent())
        {
            float NightFactor = IsNightTime() ? 1.0f : 0.0f;
            FLinearColor AmbientColor = FMath::Lerp(DayAmbientColor, NightAmbientColor, NightFactor);
            SkyComponent->SetLightColor(AmbientColor);
        }
    }
}

void AOKCGameMode::SetTimeOfDay(float Time)
{
    CurrentTimeOfDay = FMath::Fmod(Time, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }
    ApplyLightingForTimeOfDay();
}

bool AOKCGameMode::IsNightTime() const
{
    return CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 20.0f;
}

void AOKCGameMode::ToggleDayNight()
{
    if (IsNightTime())
    {
        SetTimeOfDay(12.0f); // Set to noon
    }
    else
    {
        SetTimeOfDay(0.0f); // Set to midnight
    }
}

void AOKCGameMode::ReloadMap()
{
    if (BuildingGenerator)
    {
        BuildingGenerator->ClearAllBuildings();
    }

    if (MapData)
    {
        MapData->ClearMapData();
        MapData->GenerateOKCDowntownData();
    }

    if (BuildingGenerator && MapData)
    {
        BuildingGenerator->GenerateBuildingsFromData(MapData);
    }

    UE_LOG(LogTemp, Log, TEXT("Map reloaded"));
}

void AOKCGameMode::RespawnPlayer()
{
    if (PlayerCar)
    {
        PlayerCar->SetActorLocation(DefaultSpawnLocation);
        PlayerCar->SetActorRotation(DefaultSpawnRotation);
        
        // Reset velocity
        if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(PlayerCar->GetRootComponent()))
        {
            RootPrimitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
            RootPrimitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        }

        UE_LOG(LogTemp, Log, TEXT("Player respawned"));
    }
}

bool AOKCGameMode::IsMobilePlatform() const
{
#if PLATFORM_IOS || PLATFORM_ANDROID
    return true;
#else
    return false;
#endif
}
