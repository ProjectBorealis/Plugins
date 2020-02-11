// --------------------------------------------------------------------------
// Build rules for the SceneProtocol plugin module.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

using UnrealBuildTool;
using System.IO;


public class SceneProtocol: ModuleRules
{
    public SceneProtocol(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PrivateDefinitions.Add("__UNREAL__=1");
        PrivateDefinitions.Add("ZMQ_STATIC=1");

        PublicDependencyModuleNames.AddRange(new string[] {
            "Engine",
            "Core",
            "CoreUObject",
            "Slate",
            "SlateCore",
            "EditorStyle",
            "UnrealEd",
            "MainFrame",
            "AssetRegistry",
            "RawMesh",
            "AssetTools",
            "Settings",
            "RenderCore",
            "Projects"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Engine",
            "Core",
            "CoreUObject",
            "Slate",
            "SlateCore",
            "EditorStyle",
            "UnrealEd",
            "MainFrame",
            "AssetRegistry",
            "RawMesh",
            "AssetTools",
            "Settings",
            "Projects"
        });

        AddZeroMQ(Target);
        AddMsgPack(Target);
        AddBridgeProtocol(Target);
    }

    private string ModulePath {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../Source/ThirdParty/")); }
    }

    public bool AddZeroMQ(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64))
        {
            isLibrarySupported = true;

            // I think the files in P4 are fibbing about building with MT rather than MD, but
            // incredibly hard to tell with static libraries, and there are no relevant linker warnings that I can see.
            string lib = "libzmq.lib";
            string dir = "win-64-x86-release-14.0.24210-static-md";

            bool isDebug = Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT;
            if( isDebug ) {
                lib = "libzmq_d.lib";
                dir = "win-64-x86-debug-14.0.24210-static-md";
            }

            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "zeromq", dir, "lib", lib));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "zeromq", dir, "include"));
        }
        else if ((Target.Platform == UnrealTargetPlatform.Mac))
        {
            isLibrarySupported = true;

            string LibrariesPath = Path.Combine(ThirdPartyPath, "zeromq", "osx-64-x86-release-10-12", "lib");
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libzmq-static.a"));

            string IncludePath = Path.Combine(ThirdPartyPath, "zeromq", "osx-64-x86-release-10-12", "include");
            PublicIncludePaths.Add(IncludePath);
        }
        else if ((Target.Platform == UnrealTargetPlatform.Linux))
        {
            isLibrarySupported = true;

            string LibrariesPath = Path.Combine(ThirdPartyPath, "zeromq", "linux-64-x86-release-480-cxx11", "lib");
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libzmq-static.a"));

            string IncludePath = Path.Combine(ThirdPartyPath, "zeromq", "linux-64-x86-release-480-cxx11", "include");
            PublicIncludePaths.Add(IncludePath);
        }

        return isLibrarySupported;
    }

    public bool AddMsgPack(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64))
        {
            isLibrarySupported = true;

            string lib = "msgpackc.lib";
            string dir = "win-64-x86-release-14.0.24210-dynamic";

            bool isDebug = Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT;
            if (isDebug)
            {
                lib = "msgpackc.lib";
                dir = "win-64-x86-debug-14.0.24210-dynamic";
            }

            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "msgpack-c", dir, "lib", lib));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "msgpack-c", dir, "include"));
        }
        else if ((Target.Platform == UnrealTargetPlatform.Mac))
        {
            isLibrarySupported = true;

            string LibrariesPath = Path.Combine(ThirdPartyPath, "msgpack-c", "osx-64-x86-release-10-12", "lib");
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libmsgpackc.a"));

            string IncludePath = Path.Combine(ThirdPartyPath, "msgpack-c", "osx-64-x86-release-10-12", "include");
            PublicIncludePaths.Add(IncludePath);
        }
        else if ((Target.Platform == UnrealTargetPlatform.Linux))
        {
            isLibrarySupported = true;

            string LibrariesPath = Path.Combine(ThirdPartyPath, "msgpack-c", "linux-64-x86-release-410-gcc", "lib");
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libmsgpackc.a"));

            string IncludePath = Path.Combine(ThirdPartyPath, "msgpack-c", "linux-64-x86-release-410-gcc", "include");
            PublicIncludePaths.Add(IncludePath);
        }

        return isLibrarySupported;
    }

    public bool AddBridgeProtocol(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64))
        {
            isLibrarySupported = true;

            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "lib", "STPClient.lib"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "client", "include"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "shared", "include"));
        }
        else if ((Target.Platform == UnrealTargetPlatform.Mac))
        {
            isLibrarySupported = true;

            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "lib", "libSTPClient.a"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "client", "include"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "shared", "include"));
        }
        else if ((Target.Platform == UnrealTargetPlatform.Linux))
        {
            isLibrarySupported = true;

            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "lib", "libSTPClient.a"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "client", "include"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "BridgeProtocol", "shared", "include"));
        }

        return isLibrarySupported;
    }
}

