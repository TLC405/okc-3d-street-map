# Content/Maps Directory

This directory contains the Unreal Engine level/map files.

## Required Files
- `OKCCity_Main.umap` - Main game level (create in Unreal Editor)

## Creating the Main Level

1. Open the project in Unreal Editor
2. Go to **File → New Level → Empty Level**
3. Add the following actors:
   - **Player Start** at (0, 0, 100)
   - **Directional Light** (Sun)
   - **Sky Light**
   - **Sky Atmosphere**
   - **Volumetric Cloud**
4. Save as `Content/Maps/OKCCity_Main`

The BuildingGenerator will automatically spawn buildings when the level loads.
