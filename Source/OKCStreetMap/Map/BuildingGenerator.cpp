// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#include "BuildingGenerator.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

// ABuildingActor Implementation

ABuildingActor::ABuildingActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(RootScene);

    // Create building mesh component
    BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
    BuildingMesh->SetupAttachment(RootComponent);
    BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BuildingMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    BuildingMesh->SetMobility(EComponentMobility::Static);

    CurrentLODLevel = 0;
}

void ABuildingActor::SetBuildingData(const FBuildingData& InData)
{
    BuildingData = InData;
    
    // Set actor location to building center
    SetActorLocation(FVector(InData.CenterPosition.X, InData.CenterPosition.Y, 0.0f));
}

void ABuildingActor::UpdateLOD(float DistanceToPlayer)
{
    // Simple LOD: adjust shadow and detail based on distance
    if (DistanceToPlayer > 50000.0f) // 500m
    {
        CurrentLODLevel = 2;
        BuildingMesh->SetCastShadow(false);
    }
    else if (DistanceToPlayer > 20000.0f) // 200m
    {
        CurrentLODLevel = 1;
        BuildingMesh->SetCastShadow(false);
    }
    else
    {
        CurrentLODLevel = 0;
        BuildingMesh->SetCastShadow(true);
    }
}

// ABuildingGenerator Implementation

ABuildingGenerator::ABuildingGenerator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(RootScene);

    // Create ground plane
    GroundPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundPlane"));
    GroundPlane->SetupAttachment(RootComponent);
    GroundPlane->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GroundPlane->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // Default settings
    bGenerateOnBeginPlay = true;
    bUseProceduralGeneration = true;
    bEnableCollision = true;
    bSimpleCollision = true;
    bEnableLOD = true;
    LODUpdateInterval = 0.5f;
    LODUpdateTimer = 0.0f;
    MapBoundaryRadius = 100000.0f; // 1km radius in cm

    // Setup default LOD levels
    FBuildingLODSettings LOD0;
    LOD0.MaxDistance = 20000.0f;
    LOD0.DetailLevel = 0;
    LOD0.bEnableShadows = true;
    LODLevels.Add(LOD0);

    FBuildingLODSettings LOD1;
    LOD1.MaxDistance = 50000.0f;
    LOD1.DetailLevel = 1;
    LOD1.bEnableShadows = false;
    LODLevels.Add(LOD1);

    FBuildingLODSettings LOD2;
    LOD2.MaxDistance = 100000.0f;
    LOD2.DetailLevel = 2;
    LOD2.bEnableShadows = false;
    LODLevels.Add(LOD2);

    // Try to find default cube mesh for buildings
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(
        TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        // Store reference for later use
    }
}

void ABuildingGenerator::BeginPlay()
{
    Super::BeginPlay();

    // Get player reference
    PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

    // Generate ground
    GenerateGround();

    if (bGenerateOnBeginPlay)
    {
        // Create map data loader and generate OKC data
        UMapDataLoader* MapData = NewObject<UMapDataLoader>(this);
        if (MapData)
        {
            MapData->GenerateOKCDowntownData();
            GenerateBuildingsFromData(MapData);
            GenerateStreetsFromData(MapData);
        }
    }
}

void ABuildingGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update LODs periodically
    if (bEnableLOD)
    {
        LODUpdateTimer += DeltaTime;
        if (LODUpdateTimer >= LODUpdateInterval)
        {
            LODUpdateTimer = 0.0f;
            UpdateAllBuildingLODs();
        }
    }
}

void ABuildingGenerator::GenerateBuildingsFromData(UMapDataLoader* MapData)
{
    if (!MapData)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildingGenerator: Invalid map data"));
        return;
    }

    // Clear existing buildings
    ClearAllBuildings();

    // Spawn buildings
    for (const FBuildingData& Building : MapData->Buildings)
    {
        SpawnBuilding(Building);
    }

    UE_LOG(LogTemp, Log, TEXT("BuildingGenerator: Generated %d buildings"), SpawnedBuildings.Num());
}

void ABuildingGenerator::GenerateStreetsFromData(UMapDataLoader* MapData)
{
    if (!MapData)
    {
        return;
    }

    // Streets are rendered as part of the ground texture in this implementation
    // For more complex street rendering, you would create street mesh strips here
    UE_LOG(LogTemp, Log, TEXT("BuildingGenerator: Street data loaded (%d streets)"), MapData->Streets.Num());
}

void ABuildingGenerator::GenerateGround()
{
    // Set ground plane scale to cover the map area
    if (GroundPlane)
    {
        // Load default plane mesh
        UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, 
            TEXT("/Engine/BasicShapes/Plane"));
        
        if (PlaneMesh)
        {
            GroundPlane->SetStaticMesh(PlaneMesh);
            
            // Scale to cover map area (plane is 100x100 by default)
            float Scale = MapBoundaryRadius / 50.0f;
            GroundPlane->SetWorldScale3D(FVector(Scale, Scale, 1.0f));
            GroundPlane->SetWorldLocation(FVector(0.0f, 0.0f, 0.0f));

            // Apply street material if available
            if (StreetMaterial)
            {
                GroundPlane->SetMaterial(0, StreetMaterial);
            }
        }
    }
}

void ABuildingGenerator::ClearAllBuildings()
{
    for (ABuildingActor* Building : SpawnedBuildings)
    {
        if (Building && IsValid(Building))
        {
            Building->Destroy();
        }
    }
    SpawnedBuildings.Empty();
}

void ABuildingGenerator::SpawnBuilding(const FBuildingData& Data)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Calculate building dimensions from footprint
    float MinX = TNumericLimits<float>::Max(), MaxX = TNumericLimits<float>::Lowest();
    float MinY = TNumericLimits<float>::Max(), MaxY = TNumericLimits<float>::Lowest();
    
    for (const FVector2D& Point : Data.FootprintPoints)
    {
        MinX = FMath::Min(MinX, Point.X);
        MaxX = FMath::Max(MaxX, Point.X);
        MinY = FMath::Min(MinY, Point.Y);
        MaxY = FMath::Max(MaxY, Point.Y);
    }

    float Width = MaxX - MinX;
    float Depth = MaxY - MinY;
    float Height = Data.Height;

    if (Width <= 0 || Depth <= 0 || Height <= 0)
    {
        return;
    }

    // Spawn building actor
    FVector SpawnLocation = FVector((MinX + MaxX) / 2.0f, (MinY + MaxY) / 2.0f, Height / 2.0f);
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ABuildingActor* NewBuilding = nullptr;

    if (BuildingActorClass)
    {
        NewBuilding = World->SpawnActor<ABuildingActor>(BuildingActorClass, SpawnLocation, SpawnRotation, SpawnParams);
    }
    else
    {
        NewBuilding = World->SpawnActor<ABuildingActor>(ABuildingActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
    }

    if (NewBuilding)
    {
        NewBuilding->SetBuildingData(Data);

        // Load cube mesh for building
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, 
            TEXT("/Engine/BasicShapes/Cube"));
        
        if (CubeMesh && NewBuilding->BuildingMesh)
        {
            NewBuilding->BuildingMesh->SetStaticMesh(CubeMesh);
            
            // Scale the cube to match building dimensions
            // Default cube is 100x100x100
            FVector Scale(Width / 100.0f, Depth / 100.0f, Height / 100.0f);
            NewBuilding->BuildingMesh->SetWorldScale3D(Scale);

            // Apply material
            UMaterialInterface* MaterialToUse = BuildingMaterial;
            if (Data.BuildingType == TEXT("landmark") && LandmarkMaterial)
            {
                MaterialToUse = LandmarkMaterial;
            }
            
            if (MaterialToUse)
            {
                NewBuilding->BuildingMesh->SetMaterial(0, MaterialToUse);
            }

            // Enable collision
            if (bEnableCollision)
            {
                NewBuilding->BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                NewBuilding->BuildingMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            }
        }

        SpawnedBuildings.Add(NewBuilding);
    }
}

void ABuildingGenerator::SpawnStreet(const FStreetData& Data)
{
    // Street rendering can be implemented here if needed
    // For now, streets are part of the ground texture
}

void ABuildingGenerator::UpdateAllBuildingLODs()
{
    if (!PlayerPawn)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (!PlayerPawn)
        {
            return;
        }
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (ABuildingActor* Building : SpawnedBuildings)
    {
        if (Building && IsValid(Building))
        {
            float Distance = FVector::Dist(PlayerLocation, Building->GetActorLocation());
            Building->UpdateLOD(Distance);
        }
    }
}

int32 ABuildingGenerator::GetLODLevelForDistance(float Distance) const
{
    for (int32 i = 0; i < LODLevels.Num(); i++)
    {
        if (Distance <= LODLevels[i].MaxDistance)
        {
            return LODLevels[i].DetailLevel;
        }
    }
    return LODLevels.Num() > 0 ? LODLevels.Last().DetailLevel : 0;
}

bool ABuildingGenerator::IsPositionInBounds(FVector Position) const
{
    float DistanceFromCenter = FVector::Dist2D(Position, FVector::ZeroVector);
    return DistanceFromCenter <= MapBoundaryRadius;
}

FVector ABuildingGenerator::ClampPositionToBounds(FVector Position) const
{
    float DistanceFromCenter = FVector::Dist2D(Position, FVector::ZeroVector);
    
    if (DistanceFromCenter > MapBoundaryRadius)
    {
        FVector Direction = Position.GetSafeNormal2D();
        return FVector(Direction.X * MapBoundaryRadius, Direction.Y * MapBoundaryRadius, Position.Z);
    }
    
    return Position;
}
