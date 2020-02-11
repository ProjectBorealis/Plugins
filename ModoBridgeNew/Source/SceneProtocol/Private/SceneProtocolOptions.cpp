// --------------------------------------------------------------------------
// Definition of SceneProtocol module options.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "SceneProtocolOptions.h"

#include "Commandlets/Commandlet.h"

USceneProtocolOptions::USceneProtocolOptions( const FObjectInitializer& ObjectInitializer
  ) : Super(ObjectInitializer),
    Server("localhost"),
    Port(12000),
    Timeout(10000),
#ifdef STP_METADATA_SUPPORT
    ImportMetadata(true),
#endif
#ifdef STP_CUSTOM_ACTOR_CLASSES
    UseCustomActorClasses(true),
#endif
    ProcessEventLoop(false)
{

}

void USceneProtocolOptions::UpdateFrom(
    const USceneProtocolOptions* otherOptions, const bool configOnly )
{
  Server = otherOptions->Server;
  Port = otherOptions->Port;
  Timeout = otherOptions->Timeout;
#ifdef STP_METADATA_SUPPORT
  ImportMetadata = otherOptions->ImportMetadata;
#endif
#ifdef STP_CUSTOM_ACTOR_CLASSES
  UseCustomActorClasses = otherOptions->UseCustomActorClasses;
#endif

  if( !configOnly ) {
    ProcessEventLoop = otherOptions->ProcessEventLoop;
  }
}


bool USceneProtocolOptions::ReadFromParamString(const FString& Params)
{
  bool paramsFound = false;

  TArray<FString> Tokens;
  TArray<FString> Switches;
  TMap<FString, FString> ParamVals;
  UCommandlet::ParseCommandLine(*Params, Tokens, Switches, ParamVals);

  const FString* server = ParamVals.Find(FString(TEXT("Server")));
  if( server ) { paramsFound = true; Server = *server; }

  const FString* port = ParamVals.Find(FString(TEXT("Port")));
  if( port ) { paramsFound = true; Port = FCString::Atoi(**port); }

  const FString* timeout = ParamVals.Find(FString(TEXT("Timeout")));
  if (timeout) { paramsFound = true; Timeout = FCString::Atoi(**timeout); }

#ifdef STP_METADATA_SUPPORT
  const FString* importMetadata = ParamVals.Find(FString(TEXT("ImportMetadata")));
  if( importMetadata ) {
    paramsFound = true;
    const int v = FCString::Atoi(**importMetadata);
    ImportMetadata = v == 1;
  }
#endif

#ifdef STP_CUSTOM_ACTOR_CLASSES
  const FString* useCustomActorClasses = ParamVals.Find(FString(TEXT("UseCustomActorClasses")));
  if( useCustomActorClasses ) {
    paramsFound = true;
    const int v = FCString::Atoi(**useCustomActorClasses);
    UseCustomActorClasses = v == 1;
  }
#endif

  return paramsFound;

}

void USceneProtocolOptions::PrintToLog()
{
  UE_LOG(LogSceneProtocol, Display, TEXT("Server: %s"), *Server);
  UE_LOG(LogSceneProtocol, Display, TEXT("Port: %d"), Port);
  UE_LOG(LogSceneProtocol, Display, TEXT("Timeout: %d"), Timeout);

#ifdef STP_METADATA_SUPPORT
  UE_LOG(LogSceneProtocol, Display, TEXT("ImportMetadata: %s"), ImportMetadata ? TEXT("true") : TEXT("false"));
#endif
#ifdef STP_CUSTOM_ACTOR_CLASSES
  UE_LOG(LogSceneProtocol, Display, TEXT("UseCustomActorClasses: %s"), UseCustomActorClasses ? TEXT("true") : TEXT("false"));
#endif
}

