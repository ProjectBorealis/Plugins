// --------------------------------------------------------------------------
// Texture handler.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#pragma once

#include "IAssetHandler.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "SceneProtocol.h"
#include "MetadataUtils.h"
#include "HandlerUtils.h"

#include "NodeHandler.h"
#include "CoreReaders/TextureReader.h"
#include "Names.h"
#include "ArrayData.h"
#include "ContentReader.h"

#include "Engine/Texture2D.h"

#include <ImageUtils.h>
#include <AssetRegistryModule.h>

/** This creates texture assets from texture STP::Responses
*/

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

namespace {
  class TextureHandler : public SPNB::Unreal::IAssetHandler
  {
  public:
    virtual bool create(
        SPNB::BridgeProtocol::IPackage* package,
        const STP::Client::Response* response,
        std::string& uniqueName) override;
    
    virtual bool update(
        SceneProtocol::NetworkBridge::BridgeProtocol::IPackage* package,
        const SceneTransmissionProtocol::Client::Response *response,
        std::string& uniqueName) override;

    static void generateSrcDataRGBA(TArray<FColor>& srcData, const STP::Array::BinaryHandle& data, int32_t width, int32_t height);
    static void generateSrcDataRGB(TArray<FColor>& srcData, const STP::Array::BinaryHandle& data, int32_t width, int32_t height);
    static void generateSrcDataGray(TArray<FColor>& srcData, const STP::Array::BinaryHandle& data, int32_t width, int32_t height);
    static UTexture2D* findTexture(const STP::Client::Response *response);

  };

  // Statically allocated handler.
  TextureHandler _theTextureHandler;


  /**
  * Implementation of IAssetHandler::create that converts an STP TextureReader
  * into an Unreal texture.
  */
  bool TextureHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
  {
    FString nodeName = SPNB::Unreal::stringDataToFString(response->getEntityToken());
    if (nodeName.IsEmpty())
    {
      return false;
    }

    FString displayName = SPNB::Unreal::stringDataToFString(response->getDisplayName());
    if (displayName.IsEmpty())
    {
      displayName = FPackageName::GetShortName(nodeName);
    }

    STP::Readers::TextureReader textureReader(response->getContentReader());
    // Get the image details and create the FColor array.
    int32 width = textureReader.getWidth();
    int32 height = textureReader.getHeight();
    STP::Readers::TextureReader::Format fmt = textureReader.getFormat();
    STP::Array::BinaryHandle data = textureReader.getData();

    FCreateTexture2DParameters createParams;
    //@TODO: Add support for RGB as well as RGBA.
    createParams.bUseAlpha = true;

    TArray<FColor> srcData;
    switch (fmt) {
    case STP::Readers::TextureReader::Format::RawRGBA:
        generateSrcDataRGBA(srcData, data, width, height);
        break;

    case STP::Readers::TextureReader::RawRGB:
        generateSrcDataRGB(srcData, data, width, height);
        break;

    case STP::Readers::TextureReader::RawGray:
        generateSrcDataGray(srcData, data, width, height);
        break;
    }

    SPNB::Unreal::Package* upackage = static_cast<SPNB::Unreal::Package*>(package);

    if (StaticFindObject(UTexture2D::StaticClass(), ANY_PACKAGE, *displayName) != nullptr)
    {
      displayName = MakeUniqueObjectName(ANY_PACKAGE, UTexture2D::StaticClass(), *displayName).ToString();
    }

    UTexture2D* tex = FImageUtils::CreateTexture2D(
      width, height, srcData, 
      upackage->package(), displayName,
      RF_Standalone | RF_Public, createParams);

    // Add custom item data
    USceneProtocolUserData* userData = NewObject< USceneProtocolUserData >(tex, NAME_None, RF_Public);
    userData->itemToken = *nodeName;
    tex->AddAssetUserData(userData);

    if (!tex) {
      return false;
    }

    UE_LOG(LogSceneProtocol, Log, TEXT("\tCreating material instance : %s"), ANSI_TO_TCHAR(uniqueName.c_str()));

    std::vector<UObject*> objs;
    objs.push_back(tex);
    SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objs);

    tex->UpdateResource();
    tex->MarkPackageDirty();
    tex->PostEditChange();

    FAssetRegistryModule::AssetCreated(tex);

    return true;
  }

  bool TextureHandler::update(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
  {
    UTexture2D* tex = findTexture(response);
    if (tex) {
      STP::Readers::TextureReader textureReader(response->getContentReader());
      
      bool resize = false;
      if (tex->GetSizeX() != textureReader.getWidth() ||
        tex->GetSizeY() != textureReader.getHeight()) {
        resize = true;
      }

      TArray<FColor> data;
      STP::Array::BinaryHandle rawData = textureReader.getData();

      //ETextureSourceFormat fmt;
      switch (textureReader.getFormat()) {
      case STP::Readers::TextureReader::RawRGBA:
        generateSrcDataRGBA(data, rawData, textureReader.getWidth(), textureReader.getHeight());
        break;
        
      case STP::Readers::TextureReader::RawRGB:
        generateSrcDataRGB(data, rawData, textureReader.getWidth(), textureReader.getHeight());
        break;

      case STP::Readers::TextureReader::RawGray:
        generateSrcDataGray(data, rawData, textureReader.getWidth(), textureReader.getHeight());
        break;
      }

      int32_t bytesPerPixel = STP::Readers::TextureReader::formatBytesPerPixel(textureReader.getFormat());

      tex->Source.Init(textureReader.getWidth(), textureReader.getHeight(), 1, 1, TSF_BGRA8, (uint8*)data.GetData());

      tex->UpdateResource();
      tex->MarkPackageDirty();
      tex->PostEditChange();

      return true;
    }
    return false;
  }


  void TextureHandler::generateSrcDataRGBA(TArray<FColor>& srcData, const STP::Array::BinaryHandle& data, int32_t width, int32_t height)
  {
      size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
      check(byteCount == STP::Array::getBinarySize(data));

      uint8_t* rawData = STP::Array::getBinaryData(data);
      for (size_t i = 0; i < byteCount; i += 4) {
          uint8 r = rawData[i];
          uint8 g = rawData[i + 1];
          uint8 b = rawData[i + 2];
          uint8 a = rawData[i + 3];

          srcData.Add(FColor(r, g, b, a));
      }
  }

  void TextureHandler::generateSrcDataRGB(TArray<FColor>& srcData, const STP::Array::BinaryHandle& data, int32_t width, int32_t height)
  {
      size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height) * 3;
      check(byteCount == STP::Array::getBinarySize(data));

      uint8_t* rawData = STP::Array::getBinaryData(data);
      for (size_t i = 0; i < byteCount; i += 3) {
          uint8 r = rawData[i];
          uint8 g = rawData[i + 1];
          uint8 b = rawData[i + 2];

          srcData.Add(FColor(r, g, b));
      }
  }

  void TextureHandler::generateSrcDataGray(TArray<FColor>& srcData, const STP::Array::BinaryHandle& data, int32_t width, int32_t height)
  {
      size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height);
      check(byteCount == STP::Array::getBinarySize(data));

      uint8_t* rawData = STP::Array::getBinaryData(data);
      for (size_t i = 0; i < byteCount; ++i) {
          uint8 luminance = rawData[i];

          srcData.Add(FColor(luminance, luminance, luminance));
      }
  }

  UTexture2D* TextureHandler::findTexture(const STP::Client::Response *response)
  {
    const std::string token = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
    UObject* Asset = SPNB::Unreal::PackageMapper::instance().findObject(token);

    if (Asset && Asset->IsA(UTexture2D::StaticClass())) {
      return Cast<UTexture2D>(Asset);
    }

    return nullptr;
  }


  /** Factory creation function.
  */
  SPNB::Unreal::IAssetHandler *createTextureHandler()
  {
    return &_theTextureHandler;
  }

  /** Factory destruction function */
  void destroyTextureHandler(SPNB::Unreal::IAssetHandler* handler)
  {
    (void)(handler); // statically allocated, don't destroy.
  }

  /** Registration of TextureHandler against a type name, with its factory
  *  creation/destruction functions.
  */
  NETBRIDGE_ADD_ASSET_HANDLER(TextureHandler, STP::Names::ResponseTypeTexture, createTextureHandler, destroyTextureHandler);
} // anonymous namespace

