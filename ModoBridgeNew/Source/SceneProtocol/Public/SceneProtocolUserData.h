// --------------------------------------------------------------------------
// A SceneProtocol-specific UnrealEngine StaticMeshActor
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "MetadataAsset.h"

#include "Engine/AssetUserData.h"

#include "SceneProtocolUserData.generated.h"

// Sadly the UnrealHeaderTool doesn't support namespaces
UCLASS()
class SCENEPROTOCOL_API USceneProtocolUserData : public UAssetUserData
{
  GENERATED_BODY()

public:
  USceneProtocolUserData(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITORONLY_DATA
  UPROPERTY(EditAnywhere, DuplicateTransient, Category = "Scene Protocol User Data", meta = (DisplayName = "ItemToken"))
  FName itemToken;
#endif
};