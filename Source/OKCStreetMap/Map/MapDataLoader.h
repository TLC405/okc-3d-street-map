// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MapDataLoader.generated.h"

/**
 * Structure representing a building from OpenStreetMap data.
 */
USTRUCT(BlueprintType)
struct FBuildingData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Building")
    FString BuildingId;

    UPROPERTY(BlueprintReadWrite, Category = "Building")
    TArray<FVector2D> FootprintPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Building")
    float Height;

    UPROPERTY(BlueprintReadWrite, Category = "Building")
    int32 Levels;

    UPROPERTY(BlueprintReadWrite, Category = "Building")
    FString BuildingType;

    UPROPERTY(BlueprintReadWrite, Category = "Building")
    FString Name;

    UPROPERTY(BlueprintReadWrite, Category = "Building")
    FVector CenterPosition;

    FBuildingData()
        : Height(10.0f)
        , Levels(1)
    {
    }
};

/**
 * Structure representing a street segment.
 */
USTRUCT(BlueprintType)
struct FStreetData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Street")
    FString StreetId;

    UPROPERTY(BlueprintReadWrite, Category = "Street")
    FString Name;

    UPROPERTY(BlueprintReadWrite, Category = "Street")
    TArray<FVector2D> Points;

    UPROPERTY(BlueprintReadWrite, Category = "Street")
    float Width;

    UPROPERTY(BlueprintReadWrite, Category = "Street")
    FString StreetType;

    FStreetData()
        : Width(10.0f)
    {
    }
};

/**
 * Map data loader for loading OKC building and street data.
 * Can load from JSON files or procedurally generate sample data.
 */
UCLASS(Blueprintable, BlueprintType)
class OKCSTREETMAP_API UMapDataLoader : public UObject
{
    GENERATED_BODY()

public:
    UMapDataLoader();

    // Map bounds and center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    float CenterLatitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    float CenterLongitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    float MapRadiusMeters;

    // Loaded data
    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TArray<FBuildingData> Buildings;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TArray<FStreetData> Streets;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Map")
    bool LoadMapDataFromFile(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Map")
    bool LoadMapDataFromJson(const FString& JsonString);

    UFUNCTION(BlueprintCallable, Category = "Map")
    void GenerateSampleMapData();

    UFUNCTION(BlueprintCallable, Category = "Map")
    void GenerateOKCDowntownData();

    UFUNCTION(BlueprintCallable, Category = "Map")
    FVector LatLongToWorldPosition(float Latitude, float Longitude) const;

    UFUNCTION(BlueprintCallable, Category = "Map")
    void WorldPositionToLatLong(FVector WorldPos, float& OutLatitude, float& OutLongitude) const;

    UFUNCTION(BlueprintCallable, Category = "Map")
    void ClearMapData();

    UFUNCTION(BlueprintPure, Category = "Map")
    int32 GetBuildingCount() const { return Buildings.Num(); }

    UFUNCTION(BlueprintPure, Category = "Map")
    int32 GetStreetCount() const { return Streets.Num(); }

protected:
    // Helper functions
    void ParseBuildingsFromJson(const TSharedPtr<FJsonObject>& JsonObject);
    void ParseStreetsFromJson(const TSharedPtr<FJsonObject>& JsonObject);
    FVector2D LatLongToLocal(float Latitude, float Longitude) const;

private:
    // Conversion constants
    float MetersPerDegreeLatitude;
    float MetersPerDegreeLongitude;
};
