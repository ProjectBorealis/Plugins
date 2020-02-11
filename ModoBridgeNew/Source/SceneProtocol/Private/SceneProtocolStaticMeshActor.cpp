// --------------------------------------------------------------------------
// Implementation of the SceneProtocol StaticMeshActor.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#include "SceneProtocolStaticMeshActor.h"
#include "SceneProtocol.h"

ASceneProtocolStaticMeshActor::ASceneProtocolStaticMeshActor(const FObjectInitializer& ObjectInitializer)
{
  // If this actor has been reclaimed from the GC after a re-fetch, then its
  // component will likely also be reused. The component will still be marked
  // as pending kill, so we need to clear that flag otherwise the component
  // won't be registered.
  if (GetStaticMeshComponent()->IsPendingKill()) {
    GetStaticMeshComponent()->ClearPendingKill();
  }
}
