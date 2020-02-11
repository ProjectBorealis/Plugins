// --------------------------------------------------------------------------
// Debug handler.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#pragma once

#include "IAssetHandler.h"
#include "AssetFactory.h"
#include "SceneProtocol.h"
#include "ContentReader.h"
#include "Handlers/HandlerUtils.h"

#include <sstream>

/** This is an example of an an IAssetHandler implementation.
*
* It dumps the contents of the Response's dynamic-data to the Unreal output
* window. 
*/

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

namespace {
  class DebugHandler : public SPNB::Unreal::IAssetHandler
  {
  public:
    virtual bool create(
        SPNB::BridgeProtocol::IPackage* package, 
        const STP::Client::Response* response,
        std::string& uniqueName) override;

    virtual bool update(
        SPNB::BridgeProtocol::IPackage* package,
        const STP::Client::Response *response,
        std::string& uniqueName) override;
  };


  // Statically allocated handler.
  DebugHandler _theDebugHandler;

  /**
  * Implementation of IAssetHandler::create that just dumps out the
  * contents of the
  */
  bool DebugHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
  {
    FString nodeName = SPNB::Unreal::stringDataToFString(response->getEntityToken());
    size_t keyCount = response->getContentReader().getKeyCount(STP::Readers::ContentReader::Dynamic);
    UE_LOG(LogSceneProtocol, Display, TEXT("Entity token: %s"), *nodeName);
    UE_LOG(LogSceneProtocol, Display, TEXT("There are %d attributes"), (int)keyCount);

    for (size_t c = 0; c < keyCount; ++c)
    {
      FString key = SPNB::Unreal::stringDataToFString(response->getContentReader().getKey(STP::Readers::ContentReader::Dynamic, c));
      FString typeStr = STP::Types::dataTypeToString(response->getContentReader().getDataType(STP::Readers::ContentReader::Dynamic, c));

      switch (response->getContentReader().getDataType(STP::Readers::ContentReader::Dynamic, c))
      {
      case STP::Types::String:
        {
          FString str = SPNB::Unreal::stringDataToFString(response->getContentReader().getString(STP::Readers::ContentReader::Dynamic, TCHAR_TO_ANSI(*key), NULL));
          UE_LOG(LogSceneProtocol,
            Display,
            TEXT("%s [%s]: %s"),
            *key,
            *typeStr,
            *str);

        }
        break;

      case STP::Types::Integer:
        {
          int32_t data = response->getContentReader().getInteger(STP::Readers::ContentReader::Dynamic, TCHAR_TO_ANSI(*key), NULL);

          UE_LOG(LogSceneProtocol,
            Display,
            TEXT("%s [%s]: %d"),
            *key,
            *typeStr,
            data);
        }
        break;

      case STP::Types::Float:
        {
          float data = response->getContentReader().getFloat(STP::Readers::ContentReader::Dynamic, TCHAR_TO_ANSI(*key), NULL);

          UE_LOG(LogSceneProtocol,
            Display,
            TEXT("%s [%s]: %f"),
            *key,
            *typeStr,
            data);
        }
        break;

      case STP::Types::Double:
        {
          double data = response->getContentReader().getDouble(STP::Readers::ContentReader::Dynamic, TCHAR_TO_ANSI(*key), NULL);

          UE_LOG(LogSceneProtocol,
            Display,
            TEXT("%s [%s]: %f"),
            *key,
            *typeStr,
            data);
        }
        break;

      case STP::Types::StringArray:
        {
          STP::Array::Handle handle = response->getContentReader().getArray(STP::Readers::ContentReader::Dynamic, TCHAR_TO_ANSI(*key), NULL);

          std::stringstream strs;
          for (size_t i = 0; i < STP::Array::getArraySize(handle); ++i) {
            std::string s = SPNB::Unreal::stringDataToStdString(STP::Array::getStringArrayItem(handle, i, NULL));
            strs << s << ", ";
          }

          UE_LOG(LogSceneProtocol,
            Display,
            TEXT("%s [%s]: %s"),
            *key,
            *typeStr,
            UTF8_TO_TCHAR(strs.str().c_str()));
        }
        break;

      case STP::Types::IntegerArray:
        {
          STP::Array::Handle handle = response->getContentReader().getArray(STP::Readers::ContentReader::Dynamic, TCHAR_TO_UTF8(*key), NULL);
          
          std::stringstream strs;
          for (size_t i = 0; i < STP::Array::getArraySize(handle); ++i) {
            strs << STP::Array::getIntegerArrayItem(handle, i, NULL) << ", ";
          }

          UE_LOG(LogSceneProtocol,
            Display,
            TEXT("%s [%s]: %s"),
            *key,
            *typeStr,
            UTF8_TO_TCHAR(strs.str().c_str()));
        }
        break;

      case STP::Types::FloatArray:
        {
          STP::Array::Handle handle = response->getContentReader().getArray(STP::Readers::ContentReader::Dynamic, TCHAR_TO_UTF8(*key), NULL);

          std::stringstream strs;
          for (size_t i = 0; i < STP::Array::getArraySize(handle); ++i) {
            strs << STP::Array::getFloatArrayItem(handle, i, NULL) << ", ";
          }

          UE_LOG(LogSceneProtocol,
            Display,
            TEXT("%s [%s]: %s"),
            *key,
            *typeStr,
            UTF8_TO_TCHAR(strs.str().c_str()));
        }
        break;

      case STP::Types::DoubleArray:
        {
          STP::Array::Handle handle = response->getContentReader().getArray(STP::Readers::ContentReader::Dynamic, TCHAR_TO_UTF8(*key), NULL);

          std::stringstream strs;
          for (size_t i = 0; i < STP::Array::getArraySize(handle); ++i) {
            strs << STP::Array::getDoubleArrayItem(handle, i, NULL) << ", ";
          }

          UE_LOG(LogSceneProtocol,
            Display,
            TEXT("%s [%s]: %s"),
            *key,
            *typeStr,
            UTF8_TO_TCHAR(strs.str().c_str()));
        }
        break;
      }
    }
    return true;
  }

  bool DebugHandler::update(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
  {
    return true;
  }

  /** Factory creation function.
  */
  SPNB::Unreal::IAssetHandler *createDebugHandler()
  {
    return &_theDebugHandler;
  }

  /** Factory destruction function */
  void destroyDebugHandler(SPNB::Unreal::IAssetHandler* handler)
  {
    (void)(handler); // statically allocated, don't destroy.
  }

  /** Registration of DebugHandler against a type name, with its factory
  *  creation/destruction functions.
  */
  NETBRIDGE_ADD_ASSET_HANDLER(DebugHandler, "debug", createDebugHandler, destroyDebugHandler);
} // anonymous namespace

