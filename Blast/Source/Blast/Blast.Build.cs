using System;
using System.IO;
using EpicGames.Core;

namespace UnrealBuildTool.Rules
{
    public class Blast : ModuleRules
    {
        public static string GetBlastLibraryFolderName(ModuleRules Rules)
        {
            if (Rules.Target.Configuration == UnrealTargetConfiguration.Debug)
            {
                return "debug";
            }
            else
            {
                return "release";
            }
        }

        public static void SetupModuleBlastSupport(ModuleRules Rules, string[] BlastLibs)
        {
            string LibFolderName = GetBlastLibraryFolderName(Rules);

            Rules.PublicDefinitions.Add(string.Format("BLAST_LIB_CONFIG_STRING=\"{0}\"", LibFolderName));

            const bool bUsePhysX = false;
            Rules.PublicDefinitions.Add(string.Format("BLAST_USE_PHYSX={0}", bUsePhysX ? 1 : 0));

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
                DLLSuffix = ".dll";
                LibSuffix = ".lib";
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

            if (bUsePhysX)
            {
            }
            else
            {
                Rules.PrivateDependencyModuleNames.Add("Chaos");
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
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/extensions/serialization/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/extensions/shaders/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/extensions/stress/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/shared/NvFoundation/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/globals/")),
                    Path.GetFullPath(Path.Combine(ModuleDirectory, "Public/lowlevel/")),
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
                    "BlastLoader",
                    "PhysicsCore"
                }
            );

            if (Target.bBuildEditor)
            {
                PrivateDependencyModuleNames.AddRange(
                  new string[]
                  {
                        "RawMesh",
                        "UnrealEd",
                        "BlastLoaderEditor"
                  }
                );
            }

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Engine"
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
                    "Blast/Public/extensions/assetutils",
                    "Blast/Public/extensions/authoring",
                    "Blast/Public/extensions/authoringCommon",
                    "Blast/Public/extensions/serialization",
                    "Blast/Public/extensions/shaders",
                    "Blast/Public/extensions/stress",
                    "Blast/Public/shared/NvFoundation",
                    "Blast/Public/globals",
                    "Blast/Public/lowlevel"
                }
            );

            SetupModuleBlastSupport(this, BlastLibs);

            SetupModulePhysicsSupport(Target);
        }
    }
}
