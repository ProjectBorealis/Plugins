// --------------------------------------------------------------------------
// Build rules for the SceneProtocol plugin editor module.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
using UnrealBuildTool;
using System.IO;


public class SceneProtocolEditor : ModuleRules
{
    public SceneProtocolEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PrivateIncludePaths.AddRange(new string[] { "SceneProtocolEditor/Private" });

        PublicDependencyModuleNames.AddRange(new string[] {
            "Engine",
            "Core",
            "Slate",
            "SlateCore",
            "InputCore",
            "EditorStyle",
            "LevelEditor",
            "UnrealEd",
            "MainFrame",
            "Projects",
            "LevelEditor",
            "CoreUObject",
            "SceneProtocol"
        });
    }
}

