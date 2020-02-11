// --------------------------------------------------------------------------
// Implementation of CreateProject commandlet.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#include "SceneProtocolCreateProjectCommandlet.h"
#include "SceneProtocolCommandlets.h"

#include "GameProjectUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Editor.h"
#include "Settings/EditorLoadingSavingSettings.h"


USceneProtocolCreateProjectCommandlet::USceneProtocolCreateProjectCommandlet()
  : _projectPath(""), _sourceBuild(false)
{
}


USceneProtocolCreateProjectCommandlet::USceneProtocolCreateProjectCommandlet(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer),
  _projectPath(""), _sourceBuild(false)
{
}


int32 USceneProtocolCreateProjectCommandlet::Main(const FString& Params)
{
  ParseParams(Params);

  // Use the default template supplied with the plugin
  if( _templatePath.IsEmpty() ) {
    _templatePath = GetDefaultTempatePath();
  }

  if( !_templatePath.IsEmpty() ) {
    UE_LOG(LogSceneProtocolCommandlets, Display, TEXT("Using template %s"), *_templatePath);
  }

  // Setup creation parameters
  FText FailReason, FailLog;
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 24
  FProjectInformation ProjectInfo;
  ProjectInfo.ProjectFilename = _projectPath;
  ProjectInfo.bShouldGenerateCode = _sourceBuild;
  ProjectInfo.bCopyStarterContent = false;
#else
  FProjectInformation ProjectInfo(
      _projectPath,
      _sourceBuild, /* bInGenerateCode */
      false /* bInCopyStarterContent */
  );
#endif
  ProjectInfo.TemplateFile = _templatePath;

  TArray<FString> CreatedFiles;
  if( !GameProjectUtils::CreateProject(ProjectInfo, FailReason, FailLog, &CreatedFiles))
  {
    UE_LOG(LogSceneProtocolCommandlets, Error,
        TEXT("Failed to create project %s: %s"),
          *_projectPath, *FailReason.ToString());
    return 1;
  }

  if (_sourceBuild)
  {
    if( !GameProjectUtils::BuildCodeProject(_projectPath) )
    {
      UE_LOG(LogSceneProtocolCommandlets, Error,
          TEXT("Failed to compile project %s"));
      return 1;
    }
  }
  return 0;
}



bool USceneProtocolCreateProjectCommandlet::ParseParams(const FString& Params)
{
  bool foundParams = false;

  TArray<FString> Tokens;
  TArray<FString> Switches;
  TMap<FString, FString> ParamVals;
  UCommandlet::ParseCommandLine(*Params, Tokens, Switches, ParamVals);

  const FString* path = ParamVals.Find(FString(TEXT("Path")));
  if( path ) {
    foundParams = true;
    _projectPath = *path;
  }

  const FString* templatePath = ParamVals.Find(FString(TEXT("TemplatePath")));
  if( templatePath ) {
    foundParams = true;
    _templatePath = *templatePath;
  }

  const FString* sourceFlag = ParamVals.Find(FString(TEXT("SourceBuild")));
  if ( sourceFlag ) {
    foundParams = true;
    _sourceBuild = true;
  }

  return foundParams;
}

FString USceneProtocolCreateProjectCommandlet::GetDefaultTempatePath() const
{
    FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("SceneProtocol"))->GetBaseDir();

    return FPaths::Combine(*BaseDir,
        TEXT("Templates"),
        TEXT("SceneProtocolBasic"),
        TEXT("SceneProtocolBasic.uproject"));
}



