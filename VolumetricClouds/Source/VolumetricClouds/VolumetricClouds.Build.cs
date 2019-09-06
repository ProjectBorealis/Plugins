// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class VolumetricClouds : ModuleRules
{
	public VolumetricClouds(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
	
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "UnrealEd",
			}
		);
	}
}
