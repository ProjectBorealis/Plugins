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

#include "SteamAudioModule.h"

#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "IAudioParameterTransmitter.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/CoreDelegates.h"
#include "SteamAudioAudioEngineInterface.h"
#include "SteamAudioManager.h"
#include "SteamAudioOcclusion.h"
#include "SteamAudioReverb.h"
#include "SteamAudioSpatialization.h"
#include "SteamAudioUnrealAudioEngineInterface.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

DEFINE_LOG_CATEGORY(LogSteamAudio);

IMPLEMENT_MODULE(SteamAudio::FSteamAudioModule, SteamAudio);

namespace SteamAudio {

// ---------------------------------------------------------------------------------------------------------------------
// FSteamAudioModule
// ---------------------------------------------------------------------------------------------------------------------

static TSharedPtr<IAudioEngineState> GAudioEngineState = nullptr;

IAudioEngineState* FSteamAudioModule::GetAudioEngineState()
{
    return GAudioEngineState.Get();
}

void FSteamAudioModule::SetAudioEngineState(TSharedPtr<IAudioEngineState> State)
{
    GAudioEngineState = State;
}

void FSteamAudioModule::StartupModule()
{
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("SteamAudio");
    check(Plugin);

    // Construct the path to the DLL. This works for in-editor and standalone cases.
    FString BaseDir = Plugin->GetBaseDir();
#if PLATFORM_WINDOWS
#if PLATFORM_64BITS
    FString LibraryPath = BaseDir + TEXT("/Source/SteamAudioSDK/lib/windows-x64/phonon.dll");
#else
    FString LibraryPath = BaseDir + TEXT("/Source/SteamAudioSDK/lib/windows-x86/phonon.dll");
#endif
#elif PLATFORM_LINUX
    FString LibraryPath = BaseDir + TEXT("/Source/SteamAudioSDK/lib/linux-x64/libphonon.so");
#elif PLATFORM_MAC
    FString LibraryPath = BaseDir + TEXT("/Source/SteamAudioSDK/lib/osx/libphonon.dylib");
#elif PLATFORM_ANDROID
#if PLATFORM_ANDROID_ARM
    FString LibraryPath = TEXT("libphonon.so");
#elif PLATFORM_ANDROID_ARM64
    FString LibraryPath = TEXT("libphonon.so");
#elif PLATFORM_ANDROID_X86
    FString LibraryPath = TEXT("libphonon.so");
#elif PLATFORM_ANDROID_X64
    FString LibraryPath = TEXT("libphonon.so");
#endif
#endif

    // Make sure the DLL is loaded.
#if !PLATFORM_IOS
    Library = FPlatformProcess::GetDllHandle(*LibraryPath);
    check(Library);
#endif

    // Initialize plugin factories. This allows the plugins to be selected in the platform settings.
    SpatializationPluginFactory = MakeUnique<FSteamAudioSpatializationPluginFactory>();
    OcclusionPluginFactory = MakeUnique<FSteamAudioOcclusionPluginFactory>();
    ReverbPluginFactory = MakeUnique<FSteamAudioReverbPluginFactory>();
    IModularFeatures::Get().RegisterModularFeature(FSteamAudioSpatializationPluginFactory::GetModularFeatureName(), SpatializationPluginFactory.Get());
	IModularFeatures::Get().RegisterModularFeature(FSteamAudioOcclusionPluginFactory::GetModularFeatureName(), OcclusionPluginFactory.Get());
	IModularFeatures::Get().RegisterModularFeature(FSteamAudioReverbPluginFactory::GetModularFeatureName(), ReverbPluginFactory.Get());

    // Initialize the manager.
    Manager = MakeShared<FSteamAudioManager>();
    check(Manager);

	// IMPORTANT: When loading maps, Worlds can get registered to an audio device before they are initialized, which means World->AllowAudioPlayback()
	// always returns false because it was not set yet. Skip that check for now.
	FWorldDelegates::OnWorldCleanup.AddLambda([this](const UWorld* World, bool bSessionEnded, bool bCleanupResources)
	{
		if (/*World->AllowAudioPlayback() &&*/ (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game))
		{
			WorldsHoldingManager.Remove(World);
			if (WorldsHoldingManager.IsEmpty())
				Manager->ShutDownSteamAudio();
		}
	});

    FAudioDeviceWorldDelegates::OnWorldRegisteredToAudioDevice.AddLambda([this](const UWorld* World, Audio::DeviceID DeviceID)
    {
    	if (/*World->AllowAudioPlayback() &&*/ (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game))
	    {
	    	if (WorldsHoldingManager.IsEmpty())
	    		Manager->InitializeSteamAudio(EManagerInitReason::PLAYING);
	    	WorldsHoldingManager.Add(World);
	    }
    });
	FAudioDeviceWorldDelegates::OnWorldUnregisteredWithAudioDevice.AddLambda([this](const UWorld* World, Audio::DeviceID DeviceID)
	{
		if (/*World->AllowAudioPlayback() &&*/ (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game || World->WorldType == EWorldType::Editor))
		{
			WorldsHoldingManager.Remove(World);
			if (WorldsHoldingManager.IsEmpty())
				Manager->ShutDownSteamAudio();
		}
	});

    UE_LOG(LogSteamAudio, Log, TEXT("Initialized module SteamAudio."));
}

void FSteamAudioModule::ShutdownModule()
{
    // Unload the DLL.
    FPlatformProcess::FreeDllHandle(Library);

    UE_LOG(LogSteamAudio, Log, TEXT("Shut down module SteamAudio."));
}

IAudioPluginFactory* FSteamAudioModule::GetPluginFactory(EAudioPlugin PluginType)
{
    switch (PluginType)
    {
    case EAudioPlugin::SPATIALIZATION:
        return SpatializationPluginFactory.Get();

    case EAudioPlugin::OCCLUSION:
        return OcclusionPluginFactory.Get();

    case EAudioPlugin::REVERB:
        return ReverbPluginFactory.Get();

    default:
        return nullptr;
    }
}

void FSteamAudioModule::RegisterAudioDevice(FAudioDevice* AudioDevice)
{
    if (!AudioDevices.Contains(AudioDevice))
    {
        if (Manager)
        {
            Manager->RegisterAudioPluginListener(AudioDevice);
        }

        AudioDevices.Add(AudioDevice);
    }
}

void FSteamAudioModule::UnregisterAudioDevice(FAudioDevice* AudioDevice)
{
    AudioDevices.Remove(AudioDevice);
}

TSharedPtr<IAudioEngineState> FSteamAudioModule::CreateAudioEngineState()
{
    return MakeShared<FUnrealAudioEngineState>();
}

}
