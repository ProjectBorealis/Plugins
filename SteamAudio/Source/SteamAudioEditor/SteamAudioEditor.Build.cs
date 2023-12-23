//
// Copyright (C) Valve Corporation. All rights reserved.
//

using UnrealBuildTool;

public class SteamAudioEditor : ModuleRules
{
    public SteamAudioEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateIncludePaths.AddRange(new string[] {
            "SteamAudio/Private",
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "Projects",
            "PropertyEditor",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "EditorStyle",
            "SteamAudioSDK",
            "SteamAudio",
        });
    }
}
