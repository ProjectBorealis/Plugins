// --------------------------------------------------------------------------
// Implementation of FetchScene commandlet.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "SceneProtocolFetchSceneCommandlet.h"
#include "SceneProtocolCommandlets.h"

#include "SceneProtocolModule.h"

#include "LevelEditor.h"
#include "AssetToolsModule.h"
#include "ISourceControlModule.h"
#include "FileHelpers.h"
#include "SourceControlHelpers.h"

#include "CoreGlobals.h"
#include "Editor.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "GameMapsSettings.h"

using namespace SceneProtocol::NetworkBridge;
using namespace SceneProtocol::NetworkBridge::Unreal;

USceneProtocolFetchSceneCommandlet::USceneProtocolFetchSceneCommandlet()
  : _persistOptions(true)
{
}


USceneProtocolFetchSceneCommandlet::USceneProtocolFetchSceneCommandlet(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer),
  _persistOptions(true)
{
}


int32 USceneProtocolFetchSceneCommandlet::Main(const FString& Params)
{
  ParseParams(Params);

  USceneProtocolOptions* SceneOptions \
    = GetMutableDefault<USceneProtocolOptions>();

  USceneProtocolOptions* WorkingOptions;

  if( _persistOptions ) {
    WorkingOptions = SceneOptions;
  } else {
    WorkingOptions = NewObject<USceneProtocolOptions>();
    WorkingOptions->UpdateFrom(SceneOptions);
  }

  WorkingOptions->ReadFromParamString(Params);

  auto world = LoadTargetMap();
  if (nullptr == world) {
      UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("An error occurred loading the target map"));
      return 2;
  }
  GEditor->GetEditorWorldContext().SetCurrentWorld(world);

  WorkingOptions->PrintToLog();
  UE_LOG(LogSceneProtocolCommandlets, Display,
      TEXT("SaveSettingsToProject: %s"), _persistOptions ? TEXT("true") : TEXT("false"));

  InitSourceControl();

  /**
   * From the 'import assets' commandlet from 4.14.3
   * https://github.com/EpicGames/UnrealEngine/blob/0f251f67cbcda3832e640de58582e0ee915969cd/Engine/Source/Editor/UnrealEd/Private/Commandlets/ImportAssetsCommandlet.cpp
   *
   * "Hack:  A huge amount of packages are marked dirty on startup.  This is
   * normally prevented in editor but commandlets have special powers.  We only
   * want to save assets that were created or modified at import time so clear
   * all existing ones now"
   */
  ClearDirtyPackages();

  SceneProtocolModule& Bridge = SceneProtocolModule::Get();

  int32 Status = Bridge.FetchScene(WorkingOptions);
  if (Status == Bridge.FetchResults::UserCancel ) {
    UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("User terminated SceneProtocol Import"));
    return Status;
  }
  if (Status != Bridge.FetchResults::Success) {
    UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("An error occurred importing from SceneProtocol %d"), Status);
    return Status;
  }

  UE_LOG(LogSceneProtocolCommandlets, Display, TEXT("Successfully importing from SceneProtocol"));

  if( !AttemptSave() ) {
    UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("A horrible error occurred Saving assets"));
    return 1;
  }

  if( _persistOptions ) {
    SceneOptions->SaveConfig();
  }

  return 0;
}


bool USceneProtocolFetchSceneCommandlet::ParseParams(const FString& Params)
{
  bool foundParams = false;

  TArray<FString> Tokens;
  TArray<FString> Switches;
  TMap<FString, FString> ParamVals;
  UCommandlet::ParseCommandLine(*Params, Tokens, Switches, ParamVals);

  const FString* PersistOptions = ParamVals.Find(FString(TEXT("SaveSettingsToProject")));
  if( PersistOptions ) {
    foundParams = true;
    const int v = FCString::Atoi(**PersistOptions);
    _persistOptions = v == 1;
  }

  const FString* TargetMap = ParamVals.Find(FString(TEXT("TargetMap")));
  if( TargetMap ) {
    foundParams = true;
    _targetMap = *TargetMap;
  } else {
    _targetMap = TEXT("");
  }

  return foundParams;
}


UWorld* USceneProtocolFetchSceneCommandlet::LoadTargetMap()
{
  /**
   * In the execution context for the commandlet, the default map is not
   * loaded. As such, we need to go try find the one we're supposed to be
   * importing the data into. This effectively places the requirement that
   * the Project either needs to have been open in an editor before, or have
   * a default map set.
   */

  FString TargetMap = _targetMap;

  // If the Commandlet wasn't supplied a map, then see if we have
  // the last level edited in the Project.
  if (TargetMap.IsEmpty()) {
    if (GetDefault<UEditorLoadingSavingSettings>()->LoadLevelAtStartup == ELoadLevelAtStartup::LastOpened) {
      GConfig->GetString(TEXT("EditorStartup"), TEXT("LastLevel"), TargetMap, GEditorPerProjectIni);
    }
  }

  // If we still don't have a map, then see if there is a default
  // map set in the Project Settings
  if (TargetMap.IsEmpty())
  {
    TargetMap = GetDefault<UGameMapsSettings>()->EditorStartupMap.GetLongPackageName();
  }

  if ( FPackageName::IsValidLongPackageName(TargetMap, true) )
  {
    FString PackagePath = FPackageName::LongPackageNameToFilename(TargetMap) + FPackageName::GetMapPackageExtension();

    UE_LOG(LogSceneProtocolCommandlets, Display, TEXT("Attempting to retrieve world from %s (%s)"), *TargetMap, *PackagePath);

    UPackage* pkg = LoadPackage(nullptr, *PackagePath, LOAD_EditorOnly);
    if( !pkg ) {
      UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("Unable to load package %s"), *PackagePath);
      return nullptr;
    }

    UWorld* pkgWorld = UWorld::FindWorldInPackage(pkg);
    if( !pkgWorld ) {
      UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("Unable to retrieve world from %s"), *PackagePath);
      return nullptr;
    }

    return pkgWorld;
  } else {
      UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("'%s' is not a valid Long Package Name"), *TargetMap);
  }

  return nullptr;
}


void USceneProtocolFetchSceneCommandlet::InitSourceControl()
{

  ISourceControlModule& SourceControlModule = ISourceControlModule::Get();
  _useSourceControl = SourceControlModule.IsEnabled();

  if( _useSourceControl ) {

    ISourceControlProvider& SourceControlProvider = SourceControlModule.GetProvider();
    SourceControlProvider.Init();

    _useSourceControl = SourceControlProvider.IsAvailable();
    if(!_useSourceControl) {
      UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("Not using source control as it is not available. Don't ask me why."))
    }
  }

  if( _useSourceControl ) {
    UE_LOG(LogSceneProtocolCommandlets, Display, TEXT("Source Control Enabled"));
  } else {
    UE_LOG(LogSceneProtocolCommandlets, Display, TEXT("Source Control Disabled"));
  }

}


void USceneProtocolFetchSceneCommandlet::ClearDirtyPackages()
{
  TArray<UPackage*> DirtyPackages;
  FEditorFileUtils::GetDirtyContentPackages(DirtyPackages);
  for(UPackage* Package : DirtyPackages)
  {
    Package->SetDirtyFlag(false);
  }
}


int32 USceneProtocolFetchSceneCommandlet::AttemptSave()
{
  /**
   * Nicked from the 'import assets' commandlet from 4.14.3
   * https://github.com/EpicGames/UnrealEngine/blob/0f251f67cbcda3832e640de58582e0ee915969cd/Engine/Source/Editor/UnrealEd/Private/Commandlets/ImportAssetsCommandlet.cpp
   *
   * @todo Do we need to clear the use of code from UE Source
   *
   * Saving is inherently complicated due to the integration with Source
   * Control, Couldn't find any ue provided helper functions to take care of
   * all of this, but this is how they're doing it in the 'Import Assets'
   * Commandlet. Given that that is pretty much synonymous with what were doing
   * over the network, seemed like it'd make the most sense to do the same
   * thing.
   */

  bool bSaveSucceeded = true;

  TArray<UPackage*> DirtyPackages;
  TArray<FSourceControlStateRef> PackageStates;

  FEditorFileUtils::GetDirtyContentPackages(DirtyPackages);
  FEditorFileUtils::GetDirtyWorldPackages(DirtyPackages);

  if(_useSourceControl) {
    ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();
    SourceControlProvider.GetState(DirtyPackages, PackageStates, EStateCacheUsage::ForceUpdate);
  }

  UE_LOG(LogSceneProtocolCommandlets, Display, TEXT("%d Packages to Save"), DirtyPackages.Num());
  for(int32 PackageIndex = 0; PackageIndex < DirtyPackages.Num(); ++PackageIndex)
  {
    UPackage* PackageToSave = DirtyPackages[PackageIndex];

    FString PackageFilename = SourceControlHelpers::PackageFilename(PackageToSave);

    bool bShouldAttemptToSave = false;
    bool bShouldAttemptToAdd = false;

    if(_useSourceControl) {

      FSourceControlStateRef PackageSCState = PackageStates[PackageIndex];

      bool bPackageCanBeCheckedOut = false;
      if(PackageSCState->IsCheckedOutOther())
      {
        // Cannot checkout, file is already checked out
        UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("%s is already checked out by someone else, can not submit!"), *PackageFilename);
        bSaveSucceeded = false;
      }
      else if(!PackageSCState->IsCurrent())
      {
        // Cannot checkout, file is not at head revision
        UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("%s is not at the head revision and cannot be checked out"), *PackageFilename);
        bSaveSucceeded = false;
      }
      else if(PackageSCState->CanCheckout())
      {
        const bool bWasCheckedOut = SourceControlHelpers::CheckOutFile(PackageFilename);
        bShouldAttemptToSave = bWasCheckedOut;
        if(!bWasCheckedOut)
        {
          UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("%s could not be checked out"), *PackageFilename);
          bSaveSucceeded = false;
        }
      }
      else
      {
        // package was not checked out by another user and is at the current head revision and could not be checked out
        // this means it should be added after save because it doesnt exist
        bShouldAttemptToSave = true;
        bShouldAttemptToAdd = true;
      }
    }
    else
    {

      bShouldAttemptToSave = true;

      bool bIsReadOnly = IFileManager::Get().IsReadOnly(*PackageFilename);
      if(bIsReadOnly)
      {
        bShouldAttemptToSave = FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*PackageFilename, false);
        if(!bShouldAttemptToSave)
        {
          UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("%s is read only and could not be made writable.  Will not save"), *PackageFilename);
          bSaveSucceeded = false;
        }
      }
    }

    if(bShouldAttemptToSave)
    {
      GEditor->SavePackage(PackageToSave, nullptr, RF_Standalone, *PackageFilename, (FOutputDevice*)GWarn);

      if(bShouldAttemptToAdd)
      {
        const bool bWasAdded = SourceControlHelpers::MarkFileForAdd(PackageFilename);
        if(!bWasAdded)
        {
          UE_LOG(LogSceneProtocolCommandlets, Error, TEXT("%s could not be added to source control"), *PackageFilename);
          bSaveSucceeded = false;
        }
      }
    }
  }

  return bSaveSucceeded;
}

