//
// Copyright (C) Valve Corporation. All rights reserved.
//

#pragma once

#include "SteamAudioEditorModule.h"
#include "ComponentVisualizer.h"

class FPrimitiveDrawInterface;
class FSceneView;


namespace SteamAudio {

// ---------------------------------------------------------------------------------------------------------------------
// FSteamAudioBakedListenerComponentVisualizer
// ---------------------------------------------------------------------------------------------------------------------

class FSteamAudioBakedListenerComponentVisualizer : public FComponentVisualizer
{
public:
    virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};

}
