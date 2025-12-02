// Copyright 2024 OKC Street Map Project. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OKCStreetMapEditorTarget : TargetRules
{
    public OKCStreetMapEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.Add("OKCStreetMap");
    }
}
