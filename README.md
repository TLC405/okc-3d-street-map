# OKC 3D Street Map - Driving Game

A 3D street map driving game for Oklahoma City built with Unreal Engine 5. Drive around a realistic representation of downtown OKC with real building data, collision detection, and optimized performance for both desktop and mobile platforms.

![Unreal Engine 5.3+](https://img.shields.io/badge/Unreal%20Engine-5.3%2B-blue)
![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20Mac%20%7C%20iOS%20%7C%20Android-green)
![License](https://img.shields.io/badge/License-Apache%202.0-orange)

## Features

### Vehicle System
- **Realistic Physics**: Chaos Vehicle Physics for authentic driving feel
- **Multiple Control Schemes**: Arrow keys (desktop), WASD, gamepad, and virtual joystick (mobile)
- **Camera Modes**: Toggle between first-person and third-person views
- **Smooth Controls**: Progressive acceleration and deceleration

### Map & Environment
- **Real OKC Layout**: Downtown Oklahoma City centered at 35.4676° N, 97.5164° W
- **Procedural Buildings**: Generated from OpenStreetMap-style data
- **Famous Landmarks**: Devon Tower, BOK Tower, First National Center, and more
- **Street Grid**: Authentic OKC street layout
- **Map Boundaries**: Invisible walls keep players within the play area

### Mobile Optimization
- **Virtual Joystick**: Intuitive touch controls for throttle and steering
- **Swipe Camera**: Drag to look around
- **LOD System**: 3 detail levels for buildings based on distance
- **Performance Optimized**: Targets 30+ FPS on mobile devices
- **Orientation Support**: Works in both portrait and landscape

### User Interface
- **Speed Display**: Real-time speedometer (MPH or KM/H)
- **GPS Coordinates**: Current latitude/longitude position
- **Interactive Minimap**: Shows player position and orientation
- **Camera Mode Indicator**: Shows current view mode
- **Control Hints**: On-screen reminders for available controls

### Day/Night Cycle
- **Dynamic Lighting**: Sun position changes over time
- **Golden Hour**: Warm colors at sunrise and sunset
- **Night Mode**: Reduced ambient light with building lights

## Project Structure

```
OKCStreetMap/
├── Source/OKCStreetMap/
│   ├── OKCStreetMap.Build.cs    # Build configuration
│   ├── OKCStreetMap.h/cpp       # Module definition
│   ├── Player/
│   │   ├── PlayerCar.h/cpp      # Vehicle pawn with physics
│   │   └── CameraController.h/cpp # Camera management
│   ├── Map/
│   │   ├── MapDataLoader.h/cpp  # OSM data loading/generation
│   │   └── BuildingGenerator.h/cpp # Building spawning & LOD
│   ├── UI/
│   │   ├── GameHUD.h/cpp        # HUD rendering
│   │   └── MobileInputWidget.h/cpp # Touch controls
│   └── Game/
│       └── OKCGameMode.h/cpp    # Game state management
├── Content/
│   ├── Maps/                     # Level files
│   ├── Blueprints/              # Blueprint assets
│   └── Materials/               # Material assets
├── Config/
│   ├── DefaultEngine.ini        # Engine settings
│   ├── DefaultGame.ini          # Game settings
│   └── DefaultInput.ini         # Input mappings
└── OKCStreetMap.uproject        # Project file
```

## Controls

### Desktop
| Key | Action |
|-----|--------|
| ↑ / W | Accelerate |
| ↓ / S | Brake / Reverse |
| ← / A | Steer Left |
| → / D | Steer Right |
| C | Toggle Camera Mode |
| Tab | Toggle Minimap |
| Space | Look Behind |
| R | Respawn at Start |
| Mouse | Look Around (3rd person) |

### Mobile
- **Left side of screen**: Virtual joystick for driving
- **Right side of screen**: Swipe to look around
- **Camera button**: Toggle first/third person
- **Minimap button**: Show/hide minimap

### Gamepad
| Button | Action |
|--------|--------|
| Right Trigger | Accelerate |
| Left Trigger | Brake |
| Left Stick | Steer |
| Right Stick | Look Around |
| B / Circle | Toggle Camera |
| Y / Triangle | Toggle Minimap |

## Requirements

- **Engine**: Unreal Engine 5.3 or later
- **Platform SDK** (for mobile builds):
  - iOS: Xcode 14+ and iOS SDK 13+
  - Android: Android Studio, NDK r25+, SDK 24+

### Required Plugins (Enabled by Default)
- ChaosVehiclesPlugin
- EnhancedInput
- AndroidDeviceProfileSelector
- IOSDeviceProfileSelector

## Setup Instructions

### 1. Clone the Repository
```bash
git clone https://github.com/TLC405/okc-3d-street-map.git
cd okc-3d-street-map
```

### 2. Generate Project Files

**Windows (Visual Studio)**
```bash
# Right-click OKCStreetMap.uproject → Generate Visual Studio project files
# Or use UnrealBuildTool:
"C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealBuildTool.exe" -projectfiles -project="path\to\OKCStreetMap.uproject" -game -engine
```

**Mac (Xcode)**
```bash
# Right-click OKCStreetMap.uproject → Services → Generate Xcode Project
# Or use terminal:
/Users/Shared/Epic\ Games/UE_5.3/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh -project="path/to/OKCStreetMap.uproject"
```

### 3. Open in Unreal Editor
1. Launch Unreal Engine 5.3+
2. Open `OKCStreetMap.uproject`
3. Wait for shaders to compile (first time only)

### 4. Create the Main Level
1. In the Editor, go to **File → New Level → Empty Level**
2. Save as `Content/Maps/OKCCity_Main`
3. Add a **Player Start** actor at origin (0, 0, 100)
4. The building generator will automatically populate the map

### 5. Play in Editor
- Press **Play** or **Alt+P** to test
- Use arrow keys or WASD to drive

## Building for Platforms

### Windows
```
File → Package Project → Windows (64-bit)
```

### Mac
```
File → Package Project → Mac
```

### iOS
1. Configure iOS signing in Project Settings
2. ```File → Package Project → iOS```

### Android
1. Install Android SDK/NDK via Android Studio
2. Configure in Project Settings → Platforms → Android
3. ```File → Package Project → Android (ASTC)```

## Performance Targets

| Platform | Target FPS | Resolution |
|----------|------------|------------|
| Desktop | 60+ FPS | Native |
| Mobile High-end | 60 FPS | Native |
| Mobile Mid-range | 30 FPS | Native |
| Mobile Low-end | 30 FPS | 75% |

## Customization

### Adding Real OSM Data
1. Export data from [OpenStreetMap](https://www.openstreetmap.org/)
2. Convert to JSON format (see `MapDataLoader.h` for schema)
3. Place in `Content/Data/` folder
4. Update `MapDataPath` in Game Mode

### Building Data JSON Format
```json
{
  "buildings": [
    {
      "id": "building_001",
      "name": "Devon Tower",
      "type": "landmark",
      "height": 275,
      "levels": 50,
      "footprint": [
        [35.4676, -97.5164],
        [35.4676, -97.5154],
        [35.4686, -97.5154],
        [35.4686, -97.5164]
      ]
    }
  ],
  "streets": [
    {
      "id": "street_001",
      "name": "Main Street",
      "type": "primary",
      "width": 15,
      "points": [
        [35.4670, -97.5200],
        [35.4670, -97.5100]
      ]
    }
  ]
}
```

### Modifying Vehicle Physics
Edit `PlayerCar.cpp` or adjust properties in the Blueprint `BP_PlayerCar`:
- `MaxSpeed` - Top speed in MPH
- `AccelerationRate` - How fast the car speeds up
- `DecelerationRate` - How fast the car slows down
- `SteeringSpeed` - How responsive steering is

## Known Limitations

1. **Buildings**: Currently uses simple box meshes; can be enhanced with detailed models
2. **Streets**: Rendered as ground texture; could add lane markings
3. **Traffic**: No AI traffic or pedestrians (planned for future update)
4. **Weather**: No rain/weather effects (planned for future update)

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Epic Games** - Unreal Engine 5
- **OpenStreetMap Contributors** - Map data inspiration
- **Oklahoma City** - The beautiful city this game represents

## Support

For issues and feature requests, please use the [GitHub Issues](https://github.com/TLC405/okc-3d-street-map/issues) page.
