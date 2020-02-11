// --------------------------------------------------------------------------
// Handler for SGP material types.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "IAssetHandler.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "SceneProtocol.h"
#include "HandlerUtils.h"

#include "MetadataUtils.h"
#include "NodeHandler.h"
#include "ContentReader.h"
#include "CoreReaders/MaterialReader.h"
#include "Names.h"

#include "Editor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

namespace {
  class MaterialHandler : public SPNB::Unreal::IAssetHandler
  {
  public:
    MaterialHandler();

    virtual bool create(
        SPNB::BridgeProtocol::IPackage* package,
        const STP::Client::Response* response,
        std::string& uniqueName) override;

    virtual bool update(
        SPNB::BridgeProtocol::IPackage* package,
        const STP::Client::Response* response,
        std::string& uniqueName) override;

    /**
    * Look for the material instance in package.
    */
    static UMaterialInstanceConstant* findMaterialInstance(const STP::Client::Response* response);

    /**
    * Sets values of material parameters using the values from the MaterialReader.
    */
    static void setMaterialParams(UMaterialInstanceConstant* material, const STP::Readers::MaterialReader& materialReader);

  private:

    FAssetToolsModule& _assetToolsModule;
  };



  // Statically allocated handler.
  MaterialHandler _theMaterialHandler;


  /** Factory creation function.
  */
  SPNB::Unreal::IAssetHandler *createMaterialHandler()
  {
    return &_theMaterialHandler;
  }

  /** Factory destruction function */
  void destroyMaterialHandler(SPNB::Unreal::IAssetHandler* handler)
  {
    (void)(handler); // statically allocated, don't destroy.
  }

  /** Registration of DebugHandler against a type name, with its factory
  *  creation/destruction functions.
  */
  NETBRIDGE_ADD_ASSET_HANDLER(MaterialHandler, STP::Names::ResponseTypeMaterial, createMaterialHandler, destroyMaterialHandler);


  MaterialHandler::MaterialHandler()
    : _assetToolsModule(FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools"))
  {
  }

  bool MaterialHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
  {
    const USceneProtocolOptions* options = SPNB::Unreal::NodeHandlerManager::get()->getOptions();

    if (!options)
    {
      return false;
    }

    FString nodeName = SPNB::Unreal::stringDataToFString(response->getEntityToken());
    if (nodeName.Len() == 0)
    {
      return false;
    }

    FString displayName = SPNB::Unreal::stringDataToFString(response->getDisplayName());
    if (displayName.IsEmpty())
    {
        displayName = FPackageName::GetShortName(nodeName);
    }

    STP::Readers::MaterialReader materialReader(response->getContentReader());
    std::string shaderName = SPNB::Unreal::stringDataToStdString(materialReader.getTarget());

    if (shaderName.empty())
    {
      shaderName = "M_SceneProtocolDefault";
    }

    std::string shaderSourceName = std::string("/Game/") + shaderName;

    UE_LOG(LogTemp, Log, TEXT("\tCreating or find Shader : %s"), UTF8_TO_TCHAR(shaderSourceName.c_str()));
    UMaterial* pBase = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), 0, UTF8_TO_TCHAR(shaderSourceName.c_str())));

    UMaterialInstanceConstantFactoryNew* factory = NewObject<UMaterialInstanceConstantFactoryNew>();
    factory->InitialParent = pBase;

    FString packageName = FString(package->name().c_str());
    FString packagePath = FPackageName::GetLongPackagePath(packageName);

    SPNB::Unreal::Package* upackage = static_cast<SPNB::Unreal::Package*>(package);

    if (StaticFindObject(UMaterialInstanceConstant::StaticClass(), ANY_PACKAGE, *displayName) != nullptr)
    {
      displayName = MakeUniqueObjectName(ANY_PACKAGE, UMaterialInstanceConstant::StaticClass(), *displayName).ToString();
    }

    UMaterialInstanceConstant* constInstance = Cast<UMaterialInstanceConstant>(
      _assetToolsModule.Get().CreateAsset(
        displayName,
        packagePath,
        UMaterialInstanceConstant::StaticClass(),
        factory)
      );

    if (!constInstance)
    {
      return false;
    }

    USceneProtocolUserData* userData = NewObject< USceneProtocolUserData >(constInstance, NAME_None, RF_Public);
    userData->itemToken = FName(*nodeName);
    constInstance->AddAssetUserData(userData);

    UE_LOG(LogTemp, Log, TEXT("\tCreating material instance : %s"), ANSI_TO_TCHAR(uniqueName.c_str()));

    std::vector<UObject*> objs;
    objs.push_back(constInstance);
    SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objs);

    setMaterialParams(constInstance, materialReader);

    constInstance->SetFlags(RF_Public | RF_Standalone);
    constInstance->MarkPackageDirty();
    constInstance->PostEditChange();

    return true;
  }

  bool MaterialHandler::update(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response* response, std::string& uniqueName)
  {
    UMaterialInstanceConstant* material = findMaterialInstance(response);
    if (material) {
      STP::Readers::MaterialReader materialReader(response->getContentReader());
      setMaterialParams(material, materialReader);
      material->MarkPackageDirty();
      material->PostEditChange();
      return true;
    }

    return false;
  }

  UMaterialInstanceConstant* MaterialHandler::findMaterialInstance(const STP::Client::Response* response)
  {
    const std::string token = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
    UObject* Asset = SPNB::Unreal::PackageMapper::instance().findObject(token);

    if (Asset && Asset->IsA(UMaterialInstanceConstant::StaticClass())) {
      return Cast<UMaterialInstanceConstant>(Asset);
    }

    return nullptr;
  }

  void MaterialHandler::setMaterialParams(UMaterialInstanceConstant* material, const STP::Readers::MaterialReader& materialReader)
  {
    const auto inputCount = materialReader.getInputCount();
    for (auto i = 0; i < inputCount; ++i)
    {
      const auto name = SPNB::Unreal::stringDataToStdString(materialReader.getInputName(i));
      const auto type = materialReader.getInputType(i);
      const STP::String::RAII valueKey = SceneTransmissionProtocol::Names::buildMaterialInputValueName(i);

      // name is currently of the form 'material.SceneProtocolSurfaceParams.XYZ' but
      // Unreal is expecting 'XYZ'
      const auto paramName = name.substr(name.rfind('.') + 1);

      switch (type)
      {
      case STP::Types::Float:
        {
          bool isOK = false;
          float data = materialReader.getContentReader().getFloat(STP::Readers::ContentReader::Type, *valueKey, &isOK);
          if (!isOK)
          {
            UE_LOG(LogTemp, Warning, TEXT("\tFailed to get scalar float data for %s"), UTF8_TO_TCHAR(name.c_str()));
          }

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
          FMaterialParameterInfo info;
          info.Name = name.c_str();
          material->SetScalarParameterValueEditorOnly(info, data);
#else
          material->SetScalarParameterValueEditorOnly(name.c_str(), data);
#endif
        }
        break;

      case STP::Types::FloatArray:
        {
          bool isOK = false;
          STP::Array::Handle handle = materialReader.getContentReader().getArray(STP::Readers::ContentReader::Type, *valueKey, &isOK);
          if (!isOK)
          {
            UE_LOG(LogTemp, Warning, TEXT("\tFailed to get vector float data for %s"), UTF8_TO_TCHAR(name.c_str()));
          }

          float r = 1.f;
          float g = 1.f;
          float b = 1.f;
          float a = 1.f;

          if (STP::Array::getArraySize(handle) >= 3) {
            r = STP::Array::getFloatArrayItem(handle, 0, &isOK);
            g = STP::Array::getFloatArrayItem(handle, 1, &isOK);
            b = STP::Array::getFloatArrayItem(handle, 2, &isOK);

            if (STP::Array::getArraySize(handle) >= 4) {
              a = STP::Array::getFloatArrayItem(handle, 3, &isOK);
            }
          }

          FLinearColor color = FLinearColor(r, g, b, a);


#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
          FMaterialParameterInfo info;
         info.Name = paramName.c_str();
          material->SetVectorParameterValueEditorOnly(info, color);
#else
          material->SetVectorParameterValueEditorOnly(paramName.c_str(), color);
#endif
        }
        break;

      case STP::Types::Integer:
        {
          bool isOK = false;
          int32_t data = materialReader.getContentReader().getInteger(STP::Readers::ContentReader::Type, *valueKey, &isOK);
          if (!isOK)
          {
            UE_LOG(LogTemp, Warning, TEXT("\tFailed to get scalar integer data for %s"), UTF8_TO_TCHAR(name.c_str()));
          }

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
          FMaterialParameterInfo info;
          info.Name = name.c_str();
          material->SetScalarParameterValueEditorOnly(info, data);
#else
          material->SetScalarParameterValueEditorOnly(name.c_str(), data);
#endif
        }
        break;

      default:
        UE_LOG(LogSceneProtocol, Warning, TEXT("Failed to understand data %s of type %s"),
          *FString(name.c_str()),
          *FString(STP::Types::dataTypeToString(type)));
      }
    }
  }


} // anonymous namespace
