using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class BlastEditor : ModuleRules
    {
        public BlastEditor(ReadOnlyTargetRules Target) : base(Target)
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
                    "Blast",
                    "UnrealEd"
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[] {
                    "Core",
                    "CoreUObject",
                    "AssetTools",
                    "Engine",
                    "RenderCore",
                    "Renderer",
                    "PropertyEditor",
                    "XmlParser",
                    "Slate",
                    "ContentBrowser",
                    "Projects",
                    "SlateCore",
                    "MainFrame",
                    "InputCore",
                    "EditorStyle",
                    "LevelEditor",
                    "JsonUtilities",
                    "PhysicsUtilities",
                    "Json",
                    "RHI",
                }
            );

            //Don't add to PrivateDependencyModuleNames since we only use IBlastMeshEditorModule and we can't have a circular depdency on Linux
            PrivateIncludePathModuleNames.AddRange(
                new string[] {
                    "BlastMeshEditor",
                }
            );


            string[] BlastLibs =
            {
                 "NvBlastExtAssetUtils",
                 "NvBlastExtAuthoring",
            };

            Blast.SetupModuleBlastSupport(this, BlastLibs);

            AddEngineThirdPartyPrivateStaticDependencies(Target, "FBX");

        }
    }
}