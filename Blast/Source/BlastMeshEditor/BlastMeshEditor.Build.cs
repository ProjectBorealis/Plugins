// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class BlastMeshEditor : ModuleRules
    {
        public BlastMeshEditor(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivateIncludePaths.AddRange(
                new string[] {
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/assetutils/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/authoring/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/authoringCommon/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/serialization/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/shaders/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/stress/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/globals/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/lowlevel/include/")),
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[] {
                "Engine",
                "PhysX"
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[] {
                "Blast",
                "BlastEditor",
                "Core",
                "CoreUObject",
                "InputCore",
                "RenderCore",
                "PhysicsCore",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "UnrealEd",
                "Projects",
                "DesktopPlatform",
                "AdvancedPreviewScene",
                "AssetTools",
                "LevelEditor",
                "MeshMergeUtilities",
                "RawMesh",
                "MeshUtilitiesCommon",
                "StaticMeshDescription",
                "MeshDescriptionOperations",
                "RHI",
                }
            );

            DynamicallyLoadedModuleNames.Add("PropertyEditor");

            string[] BlastLibs =
            {
                 "NvBlastExtAuthoring",
            };

            Blast.SetupModuleBlastSupport(this, BlastLibs);

            AddEngineThirdPartyPrivateStaticDependencies(Target, "FBX");
        }
    }
}