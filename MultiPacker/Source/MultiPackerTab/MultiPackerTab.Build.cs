/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

namespace UnrealBuildTool.Rules
{
	public class MultiPackerTab : ModuleRules
	{
		public MultiPackerTab (ReadOnlyTargetRules Target) : base(Target)
        {
            bEnforceIWYU = true;
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateIncludePaths.Add("MultiPackerTab/Private");
            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "EditorWidgets",
                    "AssetTools",
                    "MultiPackerRuntime",
                }
                );

            PrivateDependencyModuleNames.AddRange(
				new string[]
                {
                    "Projects",
                    "InputCore",
                    "UnrealEd",
                    "LevelEditor",
                    "CoreUObject",
                    "Engine",
                    "Slate",
                    "SlateCore",
                    "EditorStyle",
                    "RenderCore",
                    "PropertyEditor",
                }
				);
          
		}

    }
}