// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OKCGameMode.generated.h"

class UMapDataLoader;
class ABuildingGenerator;
class APlayerCar;

/**
 * Game mode for OKC Street Map driving game.
 * Manages game state, day/night cycle, and map generation.
 */
UCLASS()
class OKCSTREETMAP_API AOKCGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AOKCGameMode();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Game Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    bool bEnableDayNightCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    float DayNightCycleSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
    float CurrentTimeOfDay;

    // Map Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    TSubclassOf<ABuildingGenerator> BuildingGeneratorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FString MapDataPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    bool bGenerateProcedural;

    // Day/Night Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor DayAmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor NightAmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float DayDirectionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float NightDirectionalIntensity;

    // References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    UMapDataLoader* MapData;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ABuildingGenerator* BuildingGenerator;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    APlayerCar* PlayerCar;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Game")
    void SetTimeOfDay(float Time);

    UFUNCTION(BlueprintCallable, Category = "Game")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Game")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Game")
    void ToggleDayNight();

    UFUNCTION(BlueprintCallable, Category = "Map")
    void ReloadMap();

    UFUNCTION(BlueprintCallable, Category = "Player")
    void RespawnPlayer();

    UFUNCTION(BlueprintPure, Category = "Mobile")
    bool IsMobilePlatform() const;

protected:
    // Initialization
    void InitializeMap();
    void InitializePlayer();
    void InitializeLighting();

    // Day/Night cycle
    void UpdateDayNightCycle(float DeltaTime);
    void ApplyLightingForTimeOfDay();

private:
    // Lighting components
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLight;

    // Spawn location
    FVector DefaultSpawnLocation;
    FRotator DefaultSpawnRotation;
};
