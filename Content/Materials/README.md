# Content/Materials Directory

This directory contains material assets for the game.

## Recommended Materials

### M_Street
- Purpose: Ground/street material
- Features: Asphalt texture with optional lane markings
- Applied to: Ground plane in BuildingGenerator

### M_Building
- Purpose: Default building material
- Features: Gray concrete/glass appearance
- Applied to: Commercial buildings

### M_Landmark
- Purpose: Special building material
- Features: Reflective glass/steel for skyscrapers
- Applied to: Landmark buildings (Devon Tower, etc.)

### M_Car
- Purpose: Vehicle material
- Features: Metallic paint with clear coat
- Applied to: Player car mesh

## Creating Materials

1. Right-click in Content Browser
2. Select "Material"
3. Name the material (e.g., M_Street)
4. Double-click to open Material Editor
5. Create material graph and save

## Example Street Material Setup

```
TextureCoordinate → Multiply → TextureSample (Asphalt) → BaseColor
                                                      → Normal
                                                      → Roughness
```
