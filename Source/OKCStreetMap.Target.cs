// Copyright 2024 OKC Street Map Project. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OKCStreetMapTarget : TargetRules
{
    public OKCStreetMapTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.Add("OKCStreetMap");

        // Mobile optimization settings
        if (Target.Platform == UnrealTargetPlatform.Android ||
            Target.Platform == UnrealTargetPlatform.IOS)
        {
            bUsePCHFiles = true;
            bUseUnityBuild = true;
        }
    }
}
