/* Copyright 2018 TurboCheke, Estudio Cheke  - All Rights Reserved */

namespace UnrealBuildTool.Rules
{
	public class MultiPackerEditor : ModuleRules
	{
		public MultiPackerEditor(ReadOnlyTargetRules Target) : base(Target)
        {
            bEnforceIWYU = true;
            bUseAVX = true;
            OptimizeCode = CodeOptimization.Always;
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            PrivateIncludePaths.Add("MultiPackerEditor/Private");

            PrivateIncludePathModuleNames.AddRange(
                new string[] {
                "Settings",
                "IntroTutorials",
                "AssetTools",
            }
            );

            PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core",
                    "Engine",
                    "CoreUObject",
                    "EditorStyle",
                    "UnrealEd",
                    "KismetWidgets",
                    "GraphEditor",
                    "Kismet",
                    "EditorWidgets",
                    "MultiPackerRuntime",
                }
				);

            PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "InputCore",
                    "AssetTools",
                    "Slate",
                    "SlateCore",
                    "PropertyEditor",
                    "ContentBrowser",
                    "Projects",
                    "RenderCore",
                    "MultiPackerRuntime",
                    "ApplicationCore",
                }
				);
        }
    }
}