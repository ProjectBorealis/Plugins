// --------------------------------------------------------------------------
// Handler for SGP material definiton types.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "IAssetHandler.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "SceneProtocol.h"
#include "NodeHandler.h"
#include "HandlerUtils.h"

#include "ContentReader.h"
#include "CoreReaders/MaterialDefinitionReader.h"
#include "Names.h"

#include "Materials/MaterialExpressionConstant.h"
#include "AssetRegistryModule.h"
#include "Editor.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionTextureSample.h"

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

// The handler is not publically exposed and is only accessed via the AssetFactory
// construction mechanism and the IAssetHandler interface.
namespace {

  class MaterialDefinitionHandler : public SPNB::Unreal::IAssetHandler
  {
  public:
    virtual bool create(
      SPNB::BridgeProtocol::IPackage* package,
      const STP::Client::Response* response,
      std::string& uniqueName) override;

    virtual bool update(
      SPNB::BridgeProtocol::IPackage* package,
      const STP::Client::Response* response,
      std::string& uniqueName) override;

    static UMaterial* findMaterial(const STP::Client::Response* response);
    static FScalarMaterialInput* findScalarInput(UMaterial* material, const std::string& targetName);
    static FColorMaterialInput* findColorInput(UMaterial* material, const std::string& targetName);
    static FVectorMaterialInput* findVectorInput(UMaterial* material, const std::string& targetName);

    template<typename T>
    static void connectExpression(UMaterial* material, UMaterialExpression* expression, FMaterialInput<T>* input, int32 posY);
  };

  // Statically allocated handler.
  MaterialDefinitionHandler _theMaterialDefinitionHandler;


  /** Factory creation function.
  */
  SPNB::Unreal::IAssetHandler *createMaterialDefinitionHandler()
  {
    return &_theMaterialDefinitionHandler;
  }

  /** Factory destruction function */
  void destroyMaterialDefinitionHandler(SPNB::Unreal::IAssetHandler* handler)
  {
    (void)(handler); // statically allocated, don't destroy.
  }

  /** Registration of MaterialDefinitionHandler against a type name, with its factory
  *  creation/destruction functions.
  */
  NETBRIDGE_ADD_ASSET_HANDLER(MaterialDefinitionHandler, STP::Names::ResponseTypeMaterialDefinition, createMaterialDefinitionHandler, destroyMaterialDefinitionHandler);

  bool MaterialDefinitionHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response* response, std::string& uniqueName)
  {
    const USceneProtocolOptions* options = SPNB::Unreal::NodeHandlerManager::get()->getOptions();

    if (!options)
    {
      return false;
    }

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

    STP::Readers::MaterialDefinitionReader materialDefReader(response->getContentReader());

    // Don't try to handle materials we don't understand.
    STP::Readers::MaterialDefinitionReader::ShaderModel model = materialDefReader.getShaderModel();
    if (model == STP::Readers::MaterialDefinitionReader::ModelUnknown)
    {
      UE_LOG(LogSceneProtocol, Warning, TEXT("Unknown material definition shader model"));
      return false;
    }

    FString packageName = FString(package->name().c_str());
    FString packagePath = FPackageName::GetLongPackagePath(packageName);

    SPNB::Unreal::Package* upackage = static_cast<SPNB::Unreal::Package*>(package);

    if (StaticFindObject(UMaterial::StaticClass(), ANY_PACKAGE, *displayName) != nullptr)
    {
      displayName = MakeUniqueObjectName(ANY_PACKAGE, UMaterial::StaticClass(), *displayName).ToString();
    }

    UMaterial* material = NewObject<UMaterial>(upackage->package(), *displayName);

    if (!material)
    {
      return false;
    }

    material->SetShadingModel(EMaterialShadingModel::MSM_DefaultLit);
    
    // Add custom token data
    USceneProtocolUserData* userData = NewObject< USceneProtocolUserData >(material, NAME_None, RF_Public);
    userData->itemToken = *nodeName;
    material->AddAssetUserData(userData);

    // Just assume opaque blend mode if it's not known.
    STP::Readers::MaterialDefinitionReader::BlendMode mode = materialDefReader.getBlendMode();

    switch (mode)
    {
    case STP::Readers::MaterialDefinitionReader::Opaque:
      material->BlendMode = EBlendMode::BLEND_Opaque;
      break;

    case SceneTransmissionProtocol::Readers::MaterialDefinitionReader::Translucent:
      material->BlendMode = EBlendMode::BLEND_Translucent;
      break;

    case SceneTransmissionProtocol::Readers::MaterialDefinitionReader::ModeUnknown:
    default:
      UE_LOG(LogSceneProtocol, Warning, TEXT("Unknown blend mode on material definition %s"), *nodeName);
      material->BlendMode = EBlendMode::BLEND_Opaque;
      break;
    }

    int32 paramCount = materialDefReader.getParameterCount();
    int32 posY = 0;
    static const int32 kLargeNodeYOffset = 180;
    static const int32 kSmallNodeYOffset = 80;

    for (int32 i = 0; i < paramCount; ++i)
    {
      FName paramName = FName(*SPNB::Unreal::stringDataToFString(materialDefReader.getParameterName(i)));
      std::string target = SPNB::Unreal::stringDataToStdString(materialDefReader.getParameterTarget(i));
      STP::Readers::MaterialDefinitionReader::ParameterType type = materialDefReader.getParameterType(i);
      STP::String::RAII key = STP::Names::buildMaterialDefinitionParameterValueName(i);

      bool validData = false;
      bool validInput = false;

      switch (type) {
      case STP::Readers::MaterialDefinitionReader::Float:
        {
          bool isOK = false;
          float defaultValue = response->getContentReader().getFloat(SceneTransmissionProtocol::Readers::ContentReader::Type, *key, &isOK);
          if (isOK) {
            validData = true;
            FScalarMaterialInput* input = findScalarInput(material, target);

            if (input) {
              validInput = true;
              UMaterialExpressionScalarParameter* param = NewObject<UMaterialExpressionScalarParameter>(material);
              param->DefaultValue = defaultValue;
              param->SetParameterName(paramName);

              connectExpression(material, param, input, posY);
              posY += kSmallNodeYOffset;
            }
          }
        }
        break;

      case STP::Readers::MaterialDefinitionReader::Float2:
        {
          bool isOK = false;
          STP::Array::Handle handle = response->getContentReader().getArray(STP::Readers::ContentReader::Type, *key, &isOK);

          if (isOK && STP::Array::getArraySize(handle) == 2) {
            validData = true;

            FVectorMaterialInput* input = findVectorInput(material, target);

            if (input) {
              validInput = true;
              UMaterialExpressionVectorParameter* param = NewObject<UMaterialExpressionVectorParameter>(material);
              const float r = STP::Array::getFloatArrayItem(handle, 0, nullptr);
              const float g = STP::Array::getFloatArrayItem(handle, 1, nullptr);
              param->DefaultValue = FLinearColor(r, g, 0.f, 1.f);
              param->SetParameterName(paramName);
              connectExpression(material, param, input, posY);
              posY += kSmallNodeYOffset;
            }
          }
        }
        break;

      case STP::Readers::MaterialDefinitionReader::Float3:
        {
          bool isOK = false;
          STP::Array::Handle handle = response->getContentReader().getArray(STP::Readers::ContentReader::Type, *key, &isOK);

          if (isOK && STP::Array::getArraySize(handle) == 3) {
            validData = true;

            FVectorMaterialInput* input = findVectorInput(material, target);

            if (input) {
              validInput = true;
              UMaterialExpressionVectorParameter* param = NewObject<UMaterialExpressionVectorParameter>(material);
              const float r = STP::Array::getFloatArrayItem(handle, 0, nullptr);
              const float g = STP::Array::getFloatArrayItem(handle, 1, nullptr);
              const float b = STP::Array::getFloatArrayItem(handle, 2, nullptr);
              param->DefaultValue = FLinearColor(r, g, b, 1.f);
              param->SetParameterName(paramName);

              connectExpression(material, param, input, posY);
              posY += kLargeNodeYOffset;
            }
          }
        }
        break;

      case STP::Readers::MaterialDefinitionReader::Float4:
        {
          bool isOK = false;
          STP::Array::Handle handle = response->getContentReader().getArray(STP::Readers::ContentReader::Type, *key, &isOK);

          if (isOK && STP::Array::getArraySize(handle) == 4) {
            validData = true;

            FVectorMaterialInput* input = findVectorInput(material, target);

            if (input) {
              validInput = true;
              UMaterialExpressionVectorParameter* param = NewObject<UMaterialExpressionVectorParameter>(material);
              const float r = STP::Array::getFloatArrayItem(handle, 0, nullptr);
              const float g = STP::Array::getFloatArrayItem(handle, 1, nullptr);
              const float b = STP::Array::getFloatArrayItem(handle, 2, nullptr);
              const float a = STP::Array::getFloatArrayItem(handle, 3, nullptr);
              param->DefaultValue = FLinearColor(r, g, b, a);
              param->SetParameterName(paramName);

              connectExpression(material, param, input, posY);
              posY += kLargeNodeYOffset;
            }
          }
        }
        break;



      case STP::Readers::MaterialDefinitionReader::RGBA8:
        {
          bool isOK = false;
          STP::Array::Handle handle = response->getContentReader().getArray(STP::Readers::ContentReader::Type, *key, &isOK);

          if (isOK && STP::Array::getArraySize(handle) == 4) {
            validData = true;

            FColorMaterialInput* input = findColorInput(material, target);

            if (input) {
              validInput = true;
              UMaterialExpressionVectorParameter* param = NewObject<UMaterialExpressionVectorParameter>(material);
              const float r = STP::Array::getFloatArrayItem(handle, 0, nullptr);
              const float g = STP::Array::getFloatArrayItem(handle, 1, nullptr);
              const float b = STP::Array::getFloatArrayItem(handle, 2, nullptr);
              const float a = STP::Array::getFloatArrayItem(handle, 3, nullptr);
              param->DefaultValue = FLinearColor(r, g, b, a);
              param->SetParameterName(paramName);

              connectExpression(material, param, input, posY);
              posY += kLargeNodeYOffset;
            }
          }
        }
        break;

      case STP::Readers::MaterialDefinitionReader::Texture:
        {
          bool isOK = false;
          std::string defaultValue = SPNB::Unreal::stringDataToStdString(response->getContentReader().getString(STP::Readers::ContentReader::Type, *key, &isOK));
          if (isOK) {
            validData = true;

            std::string textureName;
            SPNB::Unreal::PackageMapper::instance().getPackageNameFromToken(defaultValue, textureName);
            UTexture2D* texture = SPNB::Unreal::PackageMapper::instance().getTexture(textureName);

            FColorMaterialInput* inputColor = nullptr;
            FVectorMaterialInput* inputVector = nullptr;
            FScalarMaterialInput* inputScalar = nullptr;
            
            inputColor = findColorInput(material, target);
            if (!inputColor)
            {
                inputVector = findVectorInput(material, target);
                if (!inputVector)
                    inputScalar = findScalarInput(material, target);
            }

            if (texture && (inputColor || inputVector || inputScalar)) {
              validInput = true;
              UMaterialExpressionTextureSample* param = NewObject<UMaterialExpressionTextureSample>(material);
              param->Texture = texture;
              param->SetParameterName(paramName);

              // Special case for normal maps
              if (target == STP::Names::MaterialDefinitionTargetNormal) {
                param->SamplerType = SAMPLERTYPE_Normal;

                // TODO: Move this part of the code to the Texture handler create and update methods
                // and add texture type flags for texture responses and requests
                texture->CompressionSettings = TC_Normalmap;
                texture->LODGroup = TEXTUREGROUP_WorldNormalMap;
                texture->SRGB = 0;
              }

              if (inputColor) {
                connectExpression (material, param, inputColor, posY);
              }
              else if (inputVector) {
                connectExpression (material, param, inputVector, posY);
              }
              else if (inputScalar) {
                connectExpression (material, param, inputScalar, posY);
              }

              posY += kLargeNodeYOffset;
            }
          }
        }
        break;
      }

      if (!validData) {
        UE_LOG(LogSceneProtocol,
          Warning,
          TEXT("Unable to get parameter value %s from %s"),
          ANSI_TO_TCHAR(*key), *nodeName);
      }
      else if (!validInput) {
        UE_LOG(LogSceneProtocol,
          Warning,
          TEXT("Unable to find input for target %s"),
          ANSI_TO_TCHAR(target.c_str()));
      }
    }

    UE_LOG(LogSceneProtocol, Log, TEXT("\tCreating material definition : %s"), ANSI_TO_TCHAR(uniqueName.c_str()));

    std::vector<UObject*> objs;
    objs.push_back(material);
    SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objs);

    material->SetFlags(RF_Public | RF_Standalone);
    material->MarkPackageDirty();
    material->PostEditChange();

    FAssetRegistryModule::AssetCreated(material);

    return true;

  }

  template<typename T>
  void MaterialDefinitionHandler::connectExpression(UMaterial* material, UMaterialExpression* expression, FMaterialInput<T>* input, int32 posY)
  {
    expression->MaterialExpressionEditorX = -200;
    expression->MaterialExpressionEditorY = posY;

    material->Expressions.Add(expression);

    TArray<FExpressionOutput> outputs;
    outputs = expression->GetOutputs();
    FExpressionOutput* output = outputs.GetData();

    input->Expression = expression;
    input->Mask = output->Mask;
    input->MaskR = output->MaskR;
    input->MaskG = output->MaskG;
    input->MaskB = output->MaskB;
    input->MaskA = output->MaskA;
  }

  // TODO: Not used currently
  bool MaterialDefinitionHandler::update(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response* response, std::string& uniqueName)
  {
    return true;
  }

  // TODO: Not used currently
  // should be used in the update method to find STP generated material
  UMaterial* MaterialDefinitionHandler::findMaterial(const STP::Client::Response* response)
  {
    const std::string token = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
    UObject* Asset = SPNB::Unreal::PackageMapper::instance().findObject(token);

    if (Asset && Asset->IsA(UMaterial::StaticClass()))
    {
      return Cast<UMaterial>(Asset);
    }

    return nullptr;
  }

  FScalarMaterialInput* MaterialDefinitionHandler::findScalarInput(UMaterial* material, const std::string& targetName)
  {
    if (targetName == STP::Names::MaterialDefinitionTargetMetallic) {
      return &material->Metallic;
    }
    else if (targetName == STP::Names::MaterialDefinitionTargetOpacity) {
      return &material->Opacity;
    }
    else if (targetName == STP::Names::MaterialDefinitionTargetRoughness) {
      return &material->Roughness;
    }
    else if (targetName == STP::Names::MaterialDefinitionTargetSpecular) {
      return &material->Specular;
    }

    return nullptr;
  }

  FColorMaterialInput* MaterialDefinitionHandler::findColorInput(UMaterial* material, const std::string& targetName)
  {
    if (targetName == STP::Names::MaterialDefinitionTargetColor) {
      return &material->BaseColor;
    }
    else if (targetName == STP::Names::MaterialDefinitionTargetEmissiveColor) {
      return &material->EmissiveColor;
    }
    else if (targetName == STP::Names::MaterialDefinitionTargetSubSurfaceColor) {
      return &material->SubsurfaceColor;
    }

    return nullptr;
  }

  FVectorMaterialInput* MaterialDefinitionHandler::findVectorInput(UMaterial* material, const std::string& targetName)
  {
    if (targetName == STP::Names::MaterialDefinitionTargetNormal) {
      return &material->Normal;
    }
    return nullptr;
  }
} // anonymous namespace
