// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ECS : ModuleRules
{
	public ECS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // UEBuildConfiguration.bForceEnableExceptions = true;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
        bEnableExceptions = true;

        PublicIncludePaths.AddRange(
	        new string[] {
		        Path.Combine(ModuleDirectory, "ThirdParty"),
				Path.Combine(ModuleDirectory, "ECS")
	        }
        );
	}
}
