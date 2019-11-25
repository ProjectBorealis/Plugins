// Copyright 2017 Allegorithmic, Inc. All Rights Reserved.
using UnrealBuildTool;
using System.IO;
using System.Collections.Generic;

public class SubstanceSource : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string PluginRootPath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../../")); }
	}

	private string SubstanceIncludePath
	{
		get { return Path.GetFullPath(Path.Combine(PluginRootPath, "Include")); }
	}

	public SubstanceSource(ReadOnlyTargetRules Target) : base(Target)
	{
		//PCH file
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		PrivatePCHHeaderFile = "Private/SubstanceSourcePrivatePCH.h";

        //Include Paths
        PrivateIncludePaths.AddRange(new string[] {
			"SubstanceSource/Private",
			SubstanceIncludePath
		});

        PublicDependencyModuleNames.AddRange(new string[] {
                "AssetTools",
                "Core",
                "CoreUObject",
                "DesktopPlatform",
                "EditorStyle",
                "EditorWidgets",
                "Engine",
                "ImageWrapper",
                "InputCore",
                "LevelEditor",
                "MainFrame",
                "SubstanceCore",
                "UnrealEd",
                "SequencerWidgets",
                "SubstanceEditor"
			});

        PrivateDependencyModuleNames.AddRange(new string[] {
                "AppFramework",
                "HTTP",
                "Json",
                "Projects",
                "Slate",
                "SlateCore",
				"WorkspaceMenuStructure",
				"SubstanceEditor",
                "PluginWarden"
            });

		// Add external libs
		List<string> StaticLibs = new List<string>();

		StaticLibs.Add("substance_source_framework");
		StaticLibs.Add("porterstemmer");

		string LibExtension = "";
		string LibPrefix = "";

		if (Target.Platform == UnrealTargetPlatform.Win32 ||
			Target.Platform == UnrealTargetPlatform.Win64)
		{
			LibExtension = ".lib";
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac ||
				 Target.Platform == UnrealTargetPlatform.Linux)
		{
			LibExtension = ".a";
			LibPrefix = "lib";
		}

		string BuildConfig = GetBuildConfig(Target);
		string PlatformConfig = Target.Platform.ToString();
		string SubstanceLibPath = ModuleDirectory + "/../../Libs/" + BuildConfig + "/" + PlatformConfig + "/";

		//add our static libs
		foreach (string staticlib in StaticLibs)
		{
			string libname = SubstanceLibPath + LibPrefix + staticlib + LibExtension;
			PublicAdditionalLibraries.Add(libname);
		}
	}

	public string GetBuildConfig(ReadOnlyTargetRules Target)
	{
		if (Target.Configuration == UnrealTargetConfiguration.Debug)
		{
			if (Target.Platform == UnrealTargetPlatform.Win32 ||
				Target.Platform == UnrealTargetPlatform.Win64)
			{
				if (Target.bDebugBuildsActuallyUseDebugCRT)
					return "Debug";
			}
			else
			{
				return "Debug";
			}
		}

		return "Release";
	}
}
