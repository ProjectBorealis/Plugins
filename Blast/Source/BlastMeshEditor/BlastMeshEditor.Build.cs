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
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/assetutils/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/authoring/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/authoringCommon/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/serialization/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/shaders/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/extensions/stress/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/shared/NvFoundation/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/globals/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "../Blast/Public/lowlevel/")),
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[] {
                "Engine"
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