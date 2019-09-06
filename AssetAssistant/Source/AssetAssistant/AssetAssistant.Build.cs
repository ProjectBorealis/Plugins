// Copyright 2017 Tefel. All Rights Reserved.

using UnrealBuildTool;

public class AssetAssistant : ModuleRules
{
    public AssetAssistant(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "AssetTools",
				"Slate",
				"SlateCore",
                "Projects",
                "PropertyEditor",
                "UnrealEd",
                "UMG",
                "EditorStyle",
                "ApplicationCore"
            }
        );	
	}
}