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

#include "SteamAudioModule.h"
#include "Tickable.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Misc/QueuedThreadPool.h"
#include "SteamAudioSettings.h"

class USteamAudioDynamicObjectComponent;
class USteamAudioListenerComponent;
class USteamAudioSourceComponent;

namespace SteamAudio {

// ---------------------------------------------------------------------------------------------------------------------
// FSteamAudioPluginListener
// ---------------------------------------------------------------------------------------------------------------------

/**
 * Receives callbacks from Unreal's built-in audio engine.
 */
class FSteamAudioPluginListener : public IAudioPluginListener
{
public:
	FSteamAudioPluginListener();
	
    /**
     * Inherited from IAudioPluginListener
     */

    /** Called to specify the latest listener position and orientation. */
    virtual void OnListenerUpdated(FAudioDevice* AudioDevice, const int32 ViewportIndex, const FTransform& ListenerTransform, const float InDeltaSeconds) override;

    IPLCoordinateSpace3 GetListenerCoordinates() const { return ListenerCoordinates; }

private:
    /** The current listener position and orientation. */
    IPLCoordinateSpace3 ListenerCoordinates;
};


// ---------------------------------------------------------------------------------------------------------------------
// FSteamAudioManager
// ---------------------------------------------------------------------------------------------------------------------

class FSimulationThreadRunnable;

UENUM()
enum class EManagerInitReason : uint8
{
    NONE,
    EXPORTING_SCENE,
    GENERATING_PROBES,
    BAKING,
    PLAYING
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSteamAudioInitialized, EManagerInitReason)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSteamAudioShutDown, EManagerInitReason)

/**
 * Singleton class that contains global Steam Audio state.
 */
class STEAMAUDIO_API FSteamAudioManager : public FTickableGameObject
{
public:
	FOnSteamAudioInitialized OnInitialized;
	FOnSteamAudioShutDown OnShutDown;
	
    FSteamAudioManager();

    ~FSteamAudioManager();

    /**
     * Inherited from FTickableObjectBase
     */

    /** Called once every frame. */
    virtual void Tick(float DeltaTime) override;

    /** Returns the stat id to use for this object. */
    virtual TStatId GetStatId() const override;

    IPLContext GetContext() const { return Context; }
    IPLHRTF GetHRTF() const { return HRTF; }
    IPLScene GetScene() const { return Scene; }
    IPLSimulator GetSimulator() const { return Simulator; }
    IPLCoordinateSpace3 GetListenerCoordinates() const;
    const FSteamAudioSettings& GetSteamAudioSettings() const { return SteamAudioSettings; }
    bool IsInitialized() const { return bInitializationSucceded; }
    EManagerInitReason InitializedType() const { return bInitializationSucceded ? InitializationAttempted : EManagerInitReason::NONE; }

    /** Initializes the HRTF. */
    bool InitHRTF(IPLAudioSettings& AudioSettings);

    /** Initializes the global Steam Audio state. */
    bool InitializeSteamAudio(EManagerInitReason Reason);

    /** Shuts down the global Steam Audio state. */
    void ShutDownSteamAudio(bool bResetFlags = true);

    /** Initializes the audio plugin listener. */
    void RegisterAudioPluginListener(FAudioDevice* OwningDevice);

    /** Returns the Steam Audio simulation settings to use at runtime. */
    IPLSimulationSettings GetRealTimeSettings(IPLSimulationFlags Flags);

    /** Returns the Steam Audio simulation settings to use while baking. */
    IPLSimulationSettings GetBakingSettings(IPLSimulationFlags Flags);

    /** Creates an Instanced Mesh object for use by the given Steam Audio Dynamic Object component. If needed, loads
        the geometry and material data into a Scene object before instantiation. If another component has already
        loaded this data, we just reference it. */
    IPLInstancedMesh LoadDynamicObject(USteamAudioDynamicObjectComponent* DynamicObjectComponent);

    /** Releases the reference to the geometry and material data for the given Steam Audio Dynamic Mesh component.
        If the reference count reaches zero, the data is destroyed. */
    void UnloadDynamicObject(USteamAudioDynamicObjectComponent* DynamicObjectComponent);

    /** Registers a Steam Audio Source component for simulation. */
    void AddSource(USteamAudioSourceComponent* Source);

    /** Unregisters a Steam Audio Source component from simulation. */
    void RemoveSource(USteamAudioSourceComponent* Source);

    /** Retrieves a Steam Audio Source component on the given actor if it exists. */
	USteamAudioSourceComponent* GetSource(uint64_t AudioComponentID) const;

    /** Registers a Steam Audio Listener component for simulation. */
    void AddListener(USteamAudioListenerComponent* Listener);

    /** Unregisters a Steam Audio Listener component from simulation. */
    void RemoveListener(USteamAudioListenerComponent* Listener);

private:
    /** The scene type we were actually able to initialize. */
    IPLSceneType ActualSceneType;

    /** The reflection effect type we were actually able to initialize. */
    IPLReflectionEffectType ActualReflectionEffectType;

    /** The Steam Audio Context object. */
    IPLContext Context;

    /** The (default) HRTF. */
    IPLHRTF HRTF;

    /** The Embree device. */
    IPLEmbreeDevice EmbreeDevice;

    /** The OpenCL device. */
    IPLOpenCLDevice OpenCLDevice;

    /** The Radeon Rays device. */
    IPLRadeonRaysDevice RadeonRaysDevice;

    /** The TrueAudio Next device. */
    IPLTrueAudioNextDevice TrueAudioNextDevice;

    /** The global scene used for simulation. */
    IPLScene Scene;

    /** The Steam Audio Simulator object. */
    IPLSimulator Simulator;

    /** True if we've attempted to initialize Steam Audio. */
    EManagerInitReason InitializationAttempted;

    /** True if we successfully initialized Steam Audio. */
    bool bInitializationSucceded;

    /** A copy of the Steam Audio settings. */
    FSteamAudioSettings SteamAudioSettings;

    /** True if we've loaded the Steam Audio settings. */
    bool bSettingsLoaded;

    /** Scenes referenced by each dynamic object that's currently loaded. */
    TMap<FString, IPLScene> DynamicObjects;

    /** Reference counts for the scenes referenced by dynamic objects. */
    TMap<FString, int> DynamicObjectRefCounts;

    /** Steam Audio Source components that are currently registered for simulation. */
    TMap<uint32_t, USteamAudioSourceComponent*> Sources;

    /** Steam Audio Listener components that are currently registered for simulation. */
    TSet<USteamAudioListenerComponent*> Listeners;

    /** The audio plugin listener used to receive global data from the built-in audio engine. */
    TAudioPluginListenerPtr AudioPluginListener;

    /** Time elapsed since the last time the simulation thread was run. */
    float SimulationUpdateTimeElapsed;

    /** Thread pool containing the simulation thread. */
    FQueuedThreadPool* ThreadPool;

    /** If true, the simulation thread is idle. */
    std::atomic<bool> ThreadPoolIdle;

    /** Called by Steam Audio, writes Steam Audio log messages to the Unreal log. */
    static void IPLCALL LogCallback(IPLLogLevel Level, IPLstring Message);

    /** Called by Steam Audio, allocates memory using Unreal's allocator. */
    static void* IPLCALL AllocateCallback(IPLsize Size, IPLsize Alignment);

    /** Called by Steam Audio, frees memory allocated using Unreal's allocator. */
    static void IPLCALL FreeCallback(void* Ptr);
};

}
