/* Copyright 2019 TurboCheke, Estudio Cheke  - All Rights Reserved */

namespace UnrealBuildTool.Rules
{
	public class MultiPackerRuntime : ModuleRules
	{
		public MultiPackerRuntime(ReadOnlyTargetRules Target) : base(Target)
        {
            bEnforceIWYU = true;
            bUseAVX = true;
            OptimizeCode = CodeOptimization.Always;
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            PrivateIncludePaths.Add("MultiPackerRuntime/Private");
         
            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
                    "Engine",
					"CoreUObject",
                    "UMG"
                }
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "Slate",
                    "SlateCore",
                    "UMG",
                    "RenderCore",
                }
				);

            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "EditorWidgets",
                    //"UnrealEd",
                    "LevelEditor"
                }
                );

                PublicDependencyModuleNames.AddRange(
                new string[]
                {
                   "DesktopPlatform",
                   "AssetTools",
                   "EditorStyle"
                }
                );
            }
		}

    }
}