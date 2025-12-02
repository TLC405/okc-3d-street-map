// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#include "MapDataLoader.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Math/UnrealMathUtility.h"

UMapDataLoader::UMapDataLoader()
{
    // Downtown OKC coordinates
    CenterLatitude = 35.4676f;
    CenterLongitude = -97.5164f;
    MapRadiusMeters = 2000.0f;

    // Calculate meters per degree at this latitude
    MetersPerDegreeLatitude = 111320.0f;
    MetersPerDegreeLongitude = 111320.0f * FMath::Cos(FMath::DegreesToRadians(CenterLatitude));
}

bool UMapDataLoader::LoadMapDataFromFile(const FString& FilePath)
{
    FString JsonContent;
    if (!FFileHelper::LoadFileToString(JsonContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load map data file: %s"), *FilePath);
        return false;
    }

    return LoadMapDataFromJson(JsonContent);
}

bool UMapDataLoader::LoadMapDataFromJson(const FString& JsonString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse map JSON data"));
        return false;
    }

    ClearMapData();

    // Parse buildings
    if (JsonObject->HasField(TEXT("buildings")))
    {
        ParseBuildingsFromJson(JsonObject);
    }

    // Parse streets
    if (JsonObject->HasField(TEXT("streets")))
    {
        ParseStreetsFromJson(JsonObject);
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded %d buildings and %d streets"), Buildings.Num(), Streets.Num());
    return true;
}

void UMapDataLoader::ParseBuildingsFromJson(const TSharedPtr<FJsonObject>& JsonObject)
{
    const TArray<TSharedPtr<FJsonValue>>* BuildingsArray;
    if (JsonObject->TryGetArrayField(TEXT("buildings"), BuildingsArray))
    {
        for (const TSharedPtr<FJsonValue>& BuildingValue : *BuildingsArray)
        {
            const TSharedPtr<FJsonObject>* BuildingObject;
            if (BuildingValue->TryGetObject(BuildingObject))
            {
                FBuildingData Building;
                
                Building.BuildingId = (*BuildingObject)->GetStringField(TEXT("id"));
                Building.Name = (*BuildingObject)->GetStringField(TEXT("name"));
                Building.BuildingType = (*BuildingObject)->GetStringField(TEXT("type"));
                Building.Height = (*BuildingObject)->GetNumberField(TEXT("height"));
                Building.Levels = (*BuildingObject)->GetIntegerField(TEXT("levels"));

                // Parse footprint points
                const TArray<TSharedPtr<FJsonValue>>* PointsArray;
                if ((*BuildingObject)->TryGetArrayField(TEXT("footprint"), PointsArray))
                {
                    for (const TSharedPtr<FJsonValue>& PointValue : *PointsArray)
                    {
                        const TArray<TSharedPtr<FJsonValue>>* CoordArray;
                        if (PointValue->TryGetArray(CoordArray) && CoordArray->Num() >= 2)
                        {
                            float Lat = (*CoordArray)[0]->AsNumber();
                            float Lon = (*CoordArray)[1]->AsNumber();
                            Building.FootprintPoints.Add(LatLongToLocal(Lat, Lon));
                        }
                    }
                }

                // Calculate center position
                if (Building.FootprintPoints.Num() > 0)
                {
                    FVector2D Center = FVector2D::ZeroVector;
                    for (const FVector2D& Point : Building.FootprintPoints)
                    {
                        Center += Point;
                    }
                    Center /= Building.FootprintPoints.Num();
                    Building.CenterPosition = FVector(Center.X, Center.Y, Building.Height / 2.0f);
                }

                Buildings.Add(Building);
            }
        }
    }
}

void UMapDataLoader::ParseStreetsFromJson(const TSharedPtr<FJsonObject>& JsonObject)
{
    const TArray<TSharedPtr<FJsonValue>>* StreetsArray;
    if (JsonObject->TryGetArrayField(TEXT("streets"), StreetsArray))
    {
        for (const TSharedPtr<FJsonValue>& StreetValue : *StreetsArray)
        {
            const TSharedPtr<FJsonObject>* StreetObject;
            if (StreetValue->TryGetObject(StreetObject))
            {
                FStreetData Street;
                
                Street.StreetId = (*StreetObject)->GetStringField(TEXT("id"));
                Street.Name = (*StreetObject)->GetStringField(TEXT("name"));
                Street.StreetType = (*StreetObject)->GetStringField(TEXT("type"));
                Street.Width = (*StreetObject)->GetNumberField(TEXT("width"));

                // Parse street points
                const TArray<TSharedPtr<FJsonValue>>* PointsArray;
                if ((*StreetObject)->TryGetArrayField(TEXT("points"), PointsArray))
                {
                    for (const TSharedPtr<FJsonValue>& PointValue : *PointsArray)
                    {
                        const TArray<TSharedPtr<FJsonValue>>* CoordArray;
                        if (PointValue->TryGetArray(CoordArray) && CoordArray->Num() >= 2)
                        {
                            float Lat = (*CoordArray)[0]->AsNumber();
                            float Lon = (*CoordArray)[1]->AsNumber();
                            Street.Points.Add(LatLongToLocal(Lat, Lon));
                        }
                    }
                }

                Streets.Add(Street);
            }
        }
    }
}

void UMapDataLoader::GenerateSampleMapData()
{
    ClearMapData();
    
    // Generate a grid of sample buildings
    float GridSize = 100.0f * 100.0f; // 100 meters in cm
    float BuildingSize = 30.0f * 100.0f; // 30 meters
    int32 GridCount = 10;

    for (int32 X = -GridCount / 2; X < GridCount / 2; X++)
    {
        for (int32 Y = -GridCount / 2; Y < GridCount / 2; Y++)
        {
            // Skip center area for streets
            if (FMath::Abs(X) <= 1 && FMath::Abs(Y) <= 1)
            {
                continue;
            }

            FBuildingData Building;
            Building.BuildingId = FString::Printf(TEXT("building_%d_%d"), X, Y);
            Building.Name = FString::Printf(TEXT("Building %d-%d"), X, Y);
            Building.BuildingType = TEXT("commercial");
            Building.Height = FMath::RandRange(10.0f, 50.0f) * 100.0f; // Random height in cm
            Building.Levels = FMath::RandRange(1, 10);

            float CenterX = X * GridSize;
            float CenterY = Y * GridSize;
            float HalfSize = BuildingSize / 2.0f;

            // Create square footprint
            Building.FootprintPoints.Add(FVector2D(CenterX - HalfSize, CenterY - HalfSize));
            Building.FootprintPoints.Add(FVector2D(CenterX + HalfSize, CenterY - HalfSize));
            Building.FootprintPoints.Add(FVector2D(CenterX + HalfSize, CenterY + HalfSize));
            Building.FootprintPoints.Add(FVector2D(CenterX - HalfSize, CenterY + HalfSize));

            Building.CenterPosition = FVector(CenterX, CenterY, Building.Height / 2.0f);

            Buildings.Add(Building);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d sample buildings"), Buildings.Num());
}

void UMapDataLoader::GenerateOKCDowntownData()
{
    ClearMapData();

    // Generate OKC Downtown layout based on real street grid
    // Downtown OKC uses a grid pattern with major streets

    float BlockSize = 100.0f * 100.0f; // 100 meters per block in cm
    float StreetWidth = 15.0f * 100.0f; // 15 meter streets
    float AlleyWidth = 5.0f * 100.0f; // 5 meter alleys

    // Major landmarks approximate positions and sizes
    struct FLandmarkInfo
    {
        FString Name;
        FVector2D Position;
        FVector2D Size;
        float Height;
    };

    TArray<FLandmarkInfo> Landmarks = {
        // Devon Energy Center (tallest building)
        { TEXT("Devon Tower"), FVector2D(0, 0), FVector2D(60, 60), 275.0f },
        // BOK Tower
        { TEXT("BOK Tower"), FVector2D(-200, 150), FVector2D(45, 45), 130.0f },
        // First National Center
        { TEXT("First National Center"), FVector2D(150, -100), FVector2D(50, 50), 150.0f },
        // Leadership Square
        { TEXT("Leadership Square"), FVector2D(-150, -200), FVector2D(40, 40), 108.0f },
        // Chase Tower
        { TEXT("Chase Tower"), FVector2D(200, 100), FVector2D(35, 35), 152.0f },
        // City Hall
        { TEXT("City Hall"), FVector2D(-100, 300), FVector2D(80, 40), 30.0f },
        // Myriad Botanical Gardens area
        { TEXT("Myriad Gardens"), FVector2D(0, -300), FVector2D(200, 100), 5.0f },
        // Bricktown area buildings
        { TEXT("Bricktown Building 1"), FVector2D(400, 50), FVector2D(30, 20), 15.0f },
        { TEXT("Bricktown Building 2"), FVector2D(450, 100), FVector2D(25, 25), 12.0f },
        { TEXT("Bricktown Building 3"), FVector2D(400, 150), FVector2D(35, 20), 18.0f },
    };

    // Create landmark buildings
    for (int32 i = 0; i < Landmarks.Num(); i++)
    {
        const FLandmarkInfo& Info = Landmarks[i];
        FBuildingData Building;
        Building.BuildingId = FString::Printf(TEXT("landmark_%d"), i);
        Building.Name = Info.Name;
        Building.BuildingType = TEXT("landmark");
        Building.Height = Info.Height * 100.0f; // Convert to cm
        Building.Levels = FMath::CeilToInt(Info.Height / 4.0f); // ~4m per floor

        float HalfX = Info.Size.X * 50.0f; // Half size in cm
        float HalfY = Info.Size.Y * 50.0f;
        float CenterX = Info.Position.X * 100.0f;
        float CenterY = Info.Position.Y * 100.0f;

        Building.FootprintPoints.Add(FVector2D(CenterX - HalfX, CenterY - HalfY));
        Building.FootprintPoints.Add(FVector2D(CenterX + HalfX, CenterY - HalfY));
        Building.FootprintPoints.Add(FVector2D(CenterX + HalfX, CenterY + HalfY));
        Building.FootprintPoints.Add(FVector2D(CenterX - HalfX, CenterY + HalfY));

        Building.CenterPosition = FVector(CenterX, CenterY, Building.Height / 2.0f);

        Buildings.Add(Building);
    }

    // Generate grid of smaller commercial buildings
    for (int32 X = -5; X <= 5; X++)
    {
        for (int32 Y = -5; Y <= 5; Y++)
        {
            float CenterX = X * BlockSize;
            float CenterY = Y * BlockSize;

            // Skip if too close to a landmark
            bool TooCloseToLandmark = false;
            for (const FLandmarkInfo& Info : Landmarks)
            {
                float LandmarkX = Info.Position.X * 100.0f;
                float LandmarkY = Info.Position.Y * 100.0f;
                float Distance = FVector2D::Distance(FVector2D(CenterX, CenterY), FVector2D(LandmarkX, LandmarkY));
                float MinDistance = FMath::Max(Info.Size.X, Info.Size.Y) * 75.0f;
                if (Distance < MinDistance)
                {
                    TooCloseToLandmark = true;
                    break;
                }
            }

            if (TooCloseToLandmark)
            {
                continue;
            }

            // Create smaller commercial building
            FBuildingData Building;
            Building.BuildingId = FString::Printf(TEXT("commercial_%d_%d"), X, Y);
            Building.Name = FString::Printf(TEXT("Commercial Building %d-%d"), X, Y);
            Building.BuildingType = TEXT("commercial");
            Building.Height = FMath::RandRange(15.0f, 60.0f) * 100.0f;
            Building.Levels = FMath::CeilToInt(Building.Height / 400.0f);

            float BuildingWidth = FMath::RandRange(20.0f, 40.0f) * 100.0f;
            float BuildingDepth = FMath::RandRange(20.0f, 40.0f) * 100.0f;
            float HalfW = BuildingWidth / 2.0f;
            float HalfD = BuildingDepth / 2.0f;

            // Add some randomness to position within block
            CenterX += FMath::RandRange(-1000.0f, 1000.0f);
            CenterY += FMath::RandRange(-1000.0f, 1000.0f);

            Building.FootprintPoints.Add(FVector2D(CenterX - HalfW, CenterY - HalfD));
            Building.FootprintPoints.Add(FVector2D(CenterX + HalfW, CenterY - HalfD));
            Building.FootprintPoints.Add(FVector2D(CenterX + HalfW, CenterY + HalfD));
            Building.FootprintPoints.Add(FVector2D(CenterX - HalfW, CenterY + HalfD));

            Building.CenterPosition = FVector(CenterX, CenterY, Building.Height / 2.0f);

            Buildings.Add(Building);
        }
    }

    // Generate streets
    for (int32 i = -6; i <= 6; i++)
    {
        // North-South streets
        FStreetData NSStreet;
        NSStreet.StreetId = FString::Printf(TEXT("ns_street_%d"), i);
        NSStreet.Name = FString::Printf(TEXT("Street %d N/S"), i);
        NSStreet.StreetType = TEXT("primary");
        NSStreet.Width = StreetWidth;
        NSStreet.Points.Add(FVector2D(i * BlockSize, -6 * BlockSize));
        NSStreet.Points.Add(FVector2D(i * BlockSize, 6 * BlockSize));
        Streets.Add(NSStreet);

        // East-West streets
        FStreetData EWStreet;
        EWStreet.StreetId = FString::Printf(TEXT("ew_street_%d"), i);
        EWStreet.Name = FString::Printf(TEXT("Street %d E/W"), i);
        EWStreet.StreetType = TEXT("primary");
        EWStreet.Width = StreetWidth;
        EWStreet.Points.Add(FVector2D(-6 * BlockSize, i * BlockSize));
        EWStreet.Points.Add(FVector2D(6 * BlockSize, i * BlockSize));
        Streets.Add(EWStreet);
    }

    UE_LOG(LogTemp, Log, TEXT("Generated OKC Downtown: %d buildings, %d streets"), Buildings.Num(), Streets.Num());
}

FVector UMapDataLoader::LatLongToWorldPosition(float Latitude, float Longitude) const
{
    float DeltaLat = Latitude - CenterLatitude;
    float DeltaLon = Longitude - CenterLongitude;

    // Convert to meters then to Unreal units (cm)
    float X = DeltaLon * MetersPerDegreeLongitude * 100.0f;
    float Y = DeltaLat * MetersPerDegreeLatitude * 100.0f;

    return FVector(X, Y, 0.0f);
}

void UMapDataLoader::WorldPositionToLatLong(FVector WorldPos, float& OutLatitude, float& OutLongitude) const
{
    // Convert from Unreal units (cm) to degrees
    float DeltaLon = WorldPos.X / (MetersPerDegreeLongitude * 100.0f);
    float DeltaLat = WorldPos.Y / (MetersPerDegreeLatitude * 100.0f);

    OutLatitude = CenterLatitude + DeltaLat;
    OutLongitude = CenterLongitude + DeltaLon;
}

void UMapDataLoader::ClearMapData()
{
    Buildings.Empty();
    Streets.Empty();
}

FVector2D UMapDataLoader::LatLongToLocal(float Latitude, float Longitude) const
{
    FVector WorldPos = LatLongToWorldPosition(Latitude, Longitude);
    return FVector2D(WorldPos.X, WorldPos.Y);
}
