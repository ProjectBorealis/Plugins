// --------------------------------------------------------------------------
// Module interface for the core SceneProtocol module.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocolOptions.h"

#include "Modules/ModuleManager.h"

#include "Containers/Ticker.h"

namespace SceneTransmissionProtocol
{
  namespace Client
  {
    class Client;
  }
}

namespace SceneProtocol {
namespace NetworkBridge {
namespace Unreal {

/**
 * The SceneProtocol module is a self-contained UE Plugin module
 * that handles any interactions with the SceneProtocol platform. It doesn't directly
 * integrate with either the Editor or any game play components. This is
 * handled by additional modules such as the SceneProtocolEditor module.
 */

class SCENEPROTOCOL_API SceneProtocolModule : public IModuleInterface
{
public:

  void StartupModule();
  void ShutdownModule();

  bool Tick(float DeltaTime);

  bool IsFetching()
  {
      return _fetching;
  }

  bool IsClientStarted()
  {
      return _clientStarted;
  }

  /**
  * Singleton-like access to this module's interface.  This is just for convenience!
  * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
  *
  * @return Returns singleton instance, loading the module on demand if needed
  */
  static inline SceneProtocolModule& Get()
  {
    return FModuleManager::LoadModuleChecked< SceneProtocolModule >("SceneProtocol");
  }

  /**
  * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
  *
  * @return True if the module is loaded and ready to use
  */
  static inline bool IsAvailable()
  {
    return FModuleManager::Get().IsModuleLoaded("SceneProtocol");
  }

  enum FetchResults
  {
    Success = 0,
    Failed = 1,
    NoWorld = 2,
    UserCancel = 3,
    TransmissionFailure = 4,
    ClientNotRunning = 5,
    NotImplemented = 99
  };

  /**
   * Retrieves scene data from the platform according to the supplied options.
   *
   * @returns A status code describing the result of the fetch:
   *    - 0: Success
   *    - 1: Failed to connect
   *    - 2: No UWorld specified
   *    - 99: Feature not implemented
   */
  int FetchScene(const USceneProtocolOptions* options);

  /**
   * Cancels fetching scene data when one is in progress.
   *
   * @returns Nothing.
   */
  void CancelFetch();

  /**
   * Informs the module that the fetch has ended, via the onExit callback.
   */
  void FetchCompleted() {
      _fetching = false;
  }

  /*
   * Starts the client thread
  */
  int StartClient(const USceneProtocolOptions* options);

  /*
  * Stops the client thread
  */
  void StopClient();

  /*
  * "Push All" and "Push Selected" UI buttons are using this method to
  * extract item data from Unreal, pack them in client requests and
  * send data to the server as create operation requests
  *
  * "Push All" iterates over all actors in currently available scenegraph
  *
  * "Push Selected" iterates over selected actors in the scenegraph
  */
  int PushItems(bool selected);

  FTickerDelegate _tickDelegate;
  FDelegateHandle _tickDelegateHandle;
  float           _totalTime;

  SceneTransmissionProtocol::Client::Client* _client;
  bool _fetching;
  bool _clientStarted;
};

}
}
}



