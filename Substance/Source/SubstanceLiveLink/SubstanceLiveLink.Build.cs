// Copyright 2017 Allegorithmic, Inc. All Rights Reserved.
using UnrealBuildTool;
using System.IO;
using System.Collections.Generic;

public class SubstanceLiveLink : ModuleRules
{
	public SubstanceLiveLink(ReadOnlyTargetRules Target) : base(Target)
	{
        //PCH file
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;

        //Include Paths
        PrivateIncludePaths.AddRange(new string[] {
			"SubstanceLiveLink/Private",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"AssetTools",
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"Projects",
			"SlateCore",
			"Slate",
			"Sockets",
			"UnrealEd"
		});
	}
}
