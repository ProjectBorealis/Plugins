//
// Copyright 2017-2023 Valve Corporation.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include "SteamAudioAudioEngineInterface.h"

namespace SteamAudio {

// ---------------------------------------------------------------------------------------------------------------------
// FUnrealAudioEngineState
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Communicates between the game engine plugin and the audio engine plugin for Unreal's built-in audio engine.
 */
class FUnrealAudioEngineState : public IAudioEngineState
{
public:
    /**
     * Inherited from IAudioEngineState
     */

    /** Returns the listener transform used by the Unreal audio device. */
    virtual FTransform GetListenerTransform() override;

    /** Returns the audio settings from the current Unreal audio device. */
    virtual IPLAudioSettings GetAudioSettings() override;

    /** Creates an interface object for communicating with a spatializer effect instance in the audio engine plugin. */
    virtual TSharedPtr<IAudioEngineSource> CreateAudioEngineSource() override;
};


// ---------------------------------------------------------------------------------------------------------------------
// FUnrealAudioEngineSource
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Communicates between the game engine plugin and the spatializer effect for Unreal's built-in audio engine.
 */
class FUnrealAudioEngineSource : public IAudioEngineSource
{
public:
    /**
     * Inherited from IAudioEngineSource
     */

    /** Does nothing. */
    virtual void Initialize(AActor* Actor) override;

    /** Does nothing. */
    virtual void Destroy() override;

    /** Does nothing. */
    virtual void UpdateParameters(USteamAudioSourceComponent* Source) override;
};

}
