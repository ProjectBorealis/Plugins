/*
	Marmoset Toolbag Scene Importer

	Copyright (C) 2016, Marmoset LLC
*/
using UnrealBuildTool;

public class ToolbagImporter : ModuleRules
{
	public ToolbagImporter(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[] {
				"ToolbagImporter/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"ToolbagImporter/Private",
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
                "RenderCore",
                "Engine",
				"Slate", 
				"SlateCore",
                "UnrealEd",
                "MainFrame",
                "RawMesh",
                "EditorStyle",
                "InputCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
