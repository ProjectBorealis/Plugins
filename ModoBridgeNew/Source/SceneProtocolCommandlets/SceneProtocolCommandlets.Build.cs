// --------------------------------------------------------------------------
// Build rules for the SceneProtocol plugin commandlets module.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
using UnrealBuildTool;
using System.IO;


public class SceneProtocolCommandlets : ModuleRules
{
    public SceneProtocolCommandlets(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PrivateIncludePaths.AddRange(new string[] { "SceneProtocolCommandlets/Private" });

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "Engine",
            "CoreUObject",
            "Settings",
            "GameProjectGeneration",
            "UnrealEd",
            "AssetRegistry",
            "AssetTools",
            "SourceControl",
            "Projects",
            "EngineSettings",
            "SceneProtocol"
        });
    }
}

