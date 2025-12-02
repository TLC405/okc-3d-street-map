// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapDataLoader.h"
#include "BuildingGenerator.generated.h"

class UStaticMeshComponent;
class UProceduralMeshComponent;
class UMaterialInterface;

/**
 * LOD level configuration for buildings
 */
USTRUCT(BlueprintType)
struct FBuildingLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 DetailLevel; // 0 = full, 1 = medium, 2 = low

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableShadows;

    FBuildingLODSettings()
        : MaxDistance(5000.0f)
        , DetailLevel(0)
        , bEnableShadows(true)
    {
    }
};

/**
 * Runtime building representation with LOD support
 */
UCLASS(BlueprintType)
class OKCSTREETMAP_API ABuildingActor : public AActor
{
    GENERATED_BODY()

public:
    ABuildingActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    UStaticMeshComponent* BuildingMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Building")
    FBuildingData BuildingData;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    int32 CurrentLODLevel;

    UFUNCTION(BlueprintCallable, Category = "Building")
    void SetBuildingData(const FBuildingData& InData);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateLOD(float DistanceToPlayer);
};

/**
 * Actor that generates and manages all buildings for the OKC map.
 * Handles procedural mesh generation, LOD management, and collision.
 */
UCLASS(Blueprintable)
class OKCSTREETMAP_API ABuildingGenerator : public AActor
{
    GENERATED_BODY()

public:
    ABuildingGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Generation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bUseProceduralGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TSubclassOf<ABuildingActor> BuildingActorClass;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* BuildingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* LandmarkMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StreetMaterial;

    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<FBuildingLODSettings> LODLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableLOD;

    // Collision Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bSimpleCollision;

    // Map Boundary
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
    float MapBoundaryRadius;

    // Ground plane
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground")
    UStaticMeshComponent* GroundPlane;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateBuildingsFromData(UMapDataLoader* MapData);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateStreetsFromData(UMapDataLoader* MapData);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateGround();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void ClearAllBuildings();

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateAllBuildingLODs();

    UFUNCTION(BlueprintPure, Category = "Generation")
    int32 GetBuildingCount() const { return SpawnedBuildings.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Boundary")
    bool IsPositionInBounds(FVector Position) const;

    UFUNCTION(BlueprintCallable, Category = "Boundary")
    FVector ClampPositionToBounds(FVector Position) const;

protected:
    // Internal functions
    void SpawnBuilding(const FBuildingData& Data);
    void SpawnStreet(const FStreetData& Data);
    UStaticMesh* CreateBuildingMesh(const FBuildingData& Data, int32 LODLevel);
    int32 GetLODLevelForDistance(float Distance) const;

private:
    // Spawned building references
    UPROPERTY()
    TArray<ABuildingActor*> SpawnedBuildings;

    // Street mesh references
    UPROPERTY()
    TArray<UStaticMeshComponent*> StreetMeshes;

    // LOD update timer
    float LODUpdateTimer;

    // Player reference for LOD calculations
    UPROPERTY()
    APawn* PlayerPawn;
};
