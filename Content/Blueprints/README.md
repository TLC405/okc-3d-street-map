# Content/Blueprints Directory

This directory contains Blueprint assets derived from C++ classes.

## Recommended Blueprints to Create

### BP_PlayerCar
- Parent Class: `APlayerCar`
- Purpose: Player vehicle with configured physics and input
- Steps to create:
  1. Right-click → Blueprint Class
  2. Search for `PlayerCar` as parent
  3. Configure vehicle mesh, wheel setup, and input actions

### BP_Building
- Parent Class: `ABuildingActor`
- Purpose: Optional custom building with materials
- Default building uses engine cube mesh

### BP_GameMode
- Parent Class: `AOKCGameMode`
- Purpose: Game mode with configured references
- Set this as the default game mode in Project Settings

## Creating Blueprints from C++ Classes

1. Right-click in Content Browser
2. Select "Blueprint Class"
3. In the parent class picker, search for the C++ class name
4. Name and save the Blueprint
5. Configure properties as needed
