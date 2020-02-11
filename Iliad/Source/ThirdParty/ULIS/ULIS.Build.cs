// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

using System;
using System.IO;
using UnrealBuildTool;

public class ULIS : ModuleRules
{
    public ULIS( ReadOnlyTargetRules Target ) : base( Target )
    {
        Type = ModuleType.External;

        if( Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Win32 )
        {
            // string PlatformDir                      = Target.Platform.ToString();
            string IncludeBase                      = Path.Combine( ModuleDirectory, "Redist", "Include" );
            string ULIS_IncludePath                 = Path.Combine( IncludeBase, "ULIS" );
            string LittleCMS_IncludePath            = Path.Combine( IncludeBase, "Little-CMS" );
            string glm_IncludePath                  = Path.Combine( IncludeBase, "glm" );
            string coal_IncludePath                 = Path.Combine( IncludeBase, "coal" );
            string boot_preprocessor_IncludePath    = Path.Combine( IncludeBase, "boost_preprocessor" );
            PublicSystemIncludePaths.AddRange(
                new string[] {
                      ULIS_IncludePath
                    , LittleCMS_IncludePath
                    , glm_IncludePath
                    , coal_IncludePath
                    , boot_preprocessor_IncludePath
                }
            );

            string LibBase          = Path.Combine( ModuleDirectory, "Redist", "Lib", "Win", "MSVC", "x64" );
            string LibBaseRelease   = Path.Combine( LibBase, "Release" );
            string LibBaseDebug     = Path.Combine( LibBase, "Debug" );

            string LibBaseFull          = LibBaseRelease;
            string ULIS_LibName         = "ULIS1.0";
            string LittleCMS_LibName    = "lcms2";

            if( Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT )
            {
                LibBaseFull         = LibBaseDebug;
                ULIS_LibName        += "d";
                LittleCMS_LibName   += "d";
            }
            
            string ULIS_LibNameExt = ULIS_LibName + ".lib";
            string LittleCMS_LibNameExt = LittleCMS_LibName + ".lib";

            PublicAdditionalLibraries.AddRange(
                new string[] {
                      Path.Combine( LibBaseFull, ULIS_LibNameExt )
                    , Path.Combine( LibBaseFull, LittleCMS_LibNameExt )
                }
            );

            PublicDefinitions.Add( "WITH_ULIS=1" );
        }
        else if( Target.Platform == UnrealTargetPlatform.Mac )
        {
            // string PlatformDir                      = Target.Platform.ToString();
            string IncludeBase                      = Path.Combine( ModuleDirectory, "Redist", "Include" );
            string ULIS_IncludePath                 = Path.Combine( IncludeBase, "ULIS" );
            string LittleCMS_IncludePath            = Path.Combine( IncludeBase, "Little-CMS" );
            string glm_IncludePath                  = Path.Combine( IncludeBase, "glm" );
            string coal_IncludePath                 = Path.Combine( IncludeBase, "coal" );
            string boot_preprocessor_IncludePath    = Path.Combine( IncludeBase, "boost_preprocessor" );
            PublicSystemIncludePaths.AddRange(
                new string[] {
                      ULIS_IncludePath
                    , LittleCMS_IncludePath
                    , glm_IncludePath
                    , coal_IncludePath
                    , boot_preprocessor_IncludePath
                }
            );

            string LibBase          = Path.Combine( ModuleDirectory, "Redist", "Lib", "Mac", "Xcode", "x64" );
            string LibBaseRelease   = Path.Combine( LibBase, "Release" );
            string LibBaseDebug     = Path.Combine( LibBase, "Debug" );

            string LibBaseFull          = LibBaseRelease;
            string ULIS_LibName         = "libULIS1.0";
            string LittleCMS_LibName    = "liblcms2";
            
            if( Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT ) 
            {
                LibBaseFull         = LibBaseDebug;
                ULIS_LibName        += "d";
                LittleCMS_LibName   += "d";
            }

            string ULIS_LibNameExt = ULIS_LibName + ".a";
            string LittleCMS_LibNameExt = LittleCMS_LibName + ".a";

            PublicAdditionalLibraries.AddRange(
                new string[] {
                      Path.Combine( LibBaseFull, ULIS_LibNameExt )
                    , Path.Combine( LibBaseFull, LittleCMS_LibNameExt )
                }
            );

            PublicDefinitions.Add( "WITH_ULIS=1" );
        }
        else // unsupported platform
        {
            PublicDefinitions.Add( "WITH_ULIS=0" );
        }
    }
}

