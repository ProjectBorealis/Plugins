// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

namespace UnrealBuildTool.Rules
{
	public class DiscordRpc : ModuleRules
	{
		public DiscordRpc(ReadOnlyTargetRules Target) : base(Target)
		{
			PublicDefinitions.Add("DISCORD_DYNAMIC_LIB=1");

			PublicIncludePathModuleNames.AddRange(
				new string[] {
					"DiscordRpc"
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"DiscordRpc/Private"
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"DiscordRpcLibrary"
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"CoreUObject",
					"Engine",
					"Slate",
					"SlateCore",
					"Projects"
				}
			);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
			);

			string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "Source", "ThirdParty", "DiscordRpcLibrary"));
			PublicIncludePaths.Add(Path.Combine(BaseDirectory, "Include"));
		}
	}
}
