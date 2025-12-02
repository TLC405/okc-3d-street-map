// Copyright 2024 OKC Street Map Project. All Rights Reserved.

using UnrealBuildTool;

public class OKCStreetMap : ModuleRules
{
    public OKCStreetMap(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Slate",
            "SlateCore",
            "ChaosVehicles",
            "PhysicsCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "HTTP",
            "Json",
            "JsonUtilities"
        });

        // Mobile platform support
        if (Target.Platform == UnrealTargetPlatform.Android ||
            Target.Platform == UnrealTargetPlatform.IOS)
        {
            PrivateDependencyModuleNames.Add("ApplicationCore");
        }
    }
}
