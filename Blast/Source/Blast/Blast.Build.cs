using System;
using System.IO;
using Tools.DotNETCommon;

namespace UnrealBuildTool.Rules
{
    public class Blast : ModuleRules
    {
        public static string GetBlastLibraryFolderName(ModuleRules Rules)
        {
            switch (Rules.Target.Configuration)
            {
                case UnrealTargetConfiguration.Debug:
                    if (Rules.Target.bUseDebugPhysXLibraries)
                    {
                        return "debug";
                    }
                    else
                    {
                        return "checked";
                    }
                case UnrealTargetConfiguration.Shipping:
                    return "release";
                case UnrealTargetConfiguration.Test:
                    return "profile";
                case UnrealTargetConfiguration.Development:
                case UnrealTargetConfiguration.DebugGame:
                case UnrealTargetConfiguration.Unknown:
                default:
                    if (Rules.Target.bUseShippingPhysXLibraries)
                    {
                        return "release";
                    }
                    else if (Rules.Target.bUseCheckedPhysXLibraries)
                    {
                        return "checked";
                    }
                    else
                    {
                        return "profile";
                    }
            }
        }

        public static void SetupModuleBlastSupport(ModuleRules Rules, string[] BlastLibs)
        {
            string LibFolderName = GetBlastLibraryFolderName(Rules);

            Rules.PublicDefinitions.Add(string.Format("BLAST_LIB_CONFIG_STRING=\"{0}\"", LibFolderName));

            //Go up two from Source/Blast
            DirectoryReference ModuleRootFolder = (new DirectoryReference(Rules.ModuleDirectory)).ParentDirectory.ParentDirectory;
            DirectoryReference EngineDirectory = new DirectoryReference(Path.GetFullPath(Rules.Target.RelativeEnginePath));
            string BLASTLibDir = Path.Combine("$(EngineDir)", ModuleRootFolder.MakeRelativeTo(EngineDirectory), "Libraries", Rules.Target.Platform.ToString(), LibFolderName);

            string DLLSuffix = "";
            string DLLPrefix = "";
            string LibSuffix = "";

            // Libraries and DLLs for windows platform
            if (Rules.Target.Platform == UnrealTargetPlatform.Win64)
            {
                DLLSuffix = "_x64.dll";
                LibSuffix = "_x64.lib";
            }
            else if (Rules.Target.Platform == UnrealTargetPlatform.Win32)
            {
                DLLSuffix = "_x86.dll";
                LibSuffix = "_x86.lib";
            }
            else if (Rules.Target.Platform == UnrealTargetPlatform.Linux)
            {
                DLLPrefix = "lib";
                DLLSuffix = ".so";
                LibSuffix = ".so";
            }

            Rules.PublicDefinitions.Add(string.Format("BLAST_LIB_DLL_SUFFIX=\"{0}\"", DLLSuffix));
            Rules.PublicDefinitions.Add(string.Format("BLAST_LIB_DLL_PREFIX=\"{0}\"", DLLPrefix));

            foreach (string Lib in BlastLibs)
            {
                Rules.PublicAdditionalLibraries.Add(Path.Combine(BLASTLibDir, String.Format("{0}{1}{2}", DLLPrefix, Lib, LibSuffix)));
                var DllName = String.Format("{0}{1}{2}", DLLPrefix, Lib, DLLSuffix);
                Rules.PublicDelayLoadDLLs.Add(DllName);
                Rules.RuntimeDependencies.Add(Path.Combine(BLASTLibDir, DllName));
            }
            
            //It's useful to periodically turn this on since the order of appending files in unity build is random.
            //The use of types without the right header can creep in and cause random build failures

            //Rules.bFasterWithoutUnity = true;
        }

        public Blast(ReadOnlyTargetRules Target) : base(Target)
        {
            OptimizeCode = CodeOptimization.InNonDebugBuilds;

            PublicIncludePaths.AddRange(
                new string[] {
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/extensions/serialization/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/extensions/shaders/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/extensions/stress/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/globals/include/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/lowlevel/include/")),
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Projects",
                    "RenderCore",
                    "Renderer",
                    "RHI",
                    "BlastLoader"
                }
            );

            if (Target.bBuildEditor)
            {
                PrivateDependencyModuleNames.AddRange(
                  new string[]
                  {
                        "RawMesh",
                        "UnrealEd",
                        "BlastLoaderEditor",
                  }
                );
            }

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Engine",
                    "PhysX",
                }
            );

            string[] BlastLibs =
            {
                 "NvBlast",
                 "NvBlastGlobals",
                 "NvBlastExtSerialization",
                 "NvBlastExtShaders",
                 "NvBlastExtStress",
            };

            PrivateIncludePaths.AddRange(
                new string[]
                {
                    "Blast/Public/extensions/assetutils/include",
                    "Blast/Public/extensions/authoring/include",
                    "Blast/Public/extensions/authoringCommon/include",
                    "Blast/Public/extensions/serialization/include",
                    "Blast/Public/extensions/shaders/include",
                    "Blast/Public/extensions/stress/include",
                    "Blast/Public/globals/include",
                    "Blast/Public/lowlevel/include"
                }
            );

            SetupModuleBlastSupport(this, BlastLibs);

            SetupModulePhysicsSupport(Target);
        }
    }
}
