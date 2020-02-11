// --------------------------------------------------------------------------
// Handler for SGP light types.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "SceneProtocol.h"
#include "SceneProtocolOptions.h"

#ifdef STP_CUSTOM_ACTOR_CLASSES
#include "SceneProtocolSpotLightActor.h"
#include "SceneProtocolDirectionalLightActor.h"
#include "SceneProtocolPointLightActor.h"
#endif

#include "IAssetHandler.h"
#include "AssetFactory.h"
#include "PackageMapper.h"
#include "NodeHandler.h"
#include "HandlerUtils.h"

#include "CoreReaders/LightReader.h"
#include "ContentReader.h"
#include "Names.h"
#include "Types.h"
#include "StringData.h"
#include "ArrayData.h"

#include "Editor.h"
#include "Math/Matrix.h"
#include "Math/TransformNonVectorized.h"
#include "AssetRegistryModule.h"

#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SpotLight.h"
#include "Components/SpotLightComponent.h"
#include "EngineUtils.h"

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

namespace {
  class LightHandler : public SPNB::Unreal::IAssetHandler
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

    template<typename T>
    static T* findActor(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response* response);

#ifdef STP_METADATA_SUPPORT
    static UMetadataAsset* createMetadata(UPackage* package, const STP::Client::Response *response, const FName& name);
#endif
    static bool createPointLight(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response);
    static bool createDirectionalLight(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response);
    static bool createSpotLight(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response);
    static void standardSetup(const STP::Client::Response& response, const AActor* actor, ULightComponent* lightComponent);
    template<typename T>
    static T* createActor(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response);
    template<typename T>
    static T* createCustomActor(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response);

  };

  // Statically allocated handler.
  LightHandler _theLightHandler;

  /** Factory creation function.
  */
  SPNB::Unreal::IAssetHandler *createLightHandler()
  {
    return &_theLightHandler;
  }

  /** Factory destruction function */
  void destroyLightHandler(SPNB::Unreal::IAssetHandler* handler)
  {
    (void)(handler); // statically allocated, don't destroy.
  }

  /** Registration of LightHandler against a type name, with its factory
  *  creation/destruction functions.
  */
  NETBRIDGE_ADD_ASSET_HANDLER(LightHandler, STP::Names::ResponseTypeLight, createLightHandler, destroyLightHandler);


  bool LightHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
  {

    STP::Readers::LightReader lightReader(response->getContentReader());
    auto lightType = lightReader.getType();

    bool success = false;
    switch (lightType)
    {
    case STP::Readers::LightReader::Type::Spot:
      success = createSpotLight(package, *response);
      break;

    case STP::Readers::LightReader::Type::Directional:
      success = createDirectionalLight(package, *response);
      break;

    case STP::Readers::LightReader::Type::Point:
      success = createPointLight(package, *response);
      break;
      
    default:
      UE_LOG(LogTemp, Warning, TEXT("\tUnknown light type"));
    }

    return success;
  }

  bool LightHandler::update(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
  {
    STP::Readers::LightReader lightReader(response->getContentReader());
    auto lightType = lightReader.getType();

    // Find the actor
    AActor* actor = nullptr;
    ULightComponent* lightComponent = nullptr;

    switch (lightType)
    {
    case STP::Readers::LightReader::Type::Spot:
    {
      USpotLightComponent* spotComponent = nullptr;
#ifdef STP_CUSTOM_ACTOR_CLASSES
      // Search for custom spot light actor
      ASceneProtocolSpotLightActor* spotActorCustom = findActor<ASceneProtocolSpotLightActor>(package, response);
      if (spotActorCustom)
      {
        spotComponent = spotActorCustom->spotComponent;
        actor = spotActorCustom;
      }
#endif
      if (!spotComponent)
      {
        ASpotLight* spotActor = findActor<ASpotLight>(package, response);

        if (spotActor)
        {
          spotComponent = spotActor->SpotLightComponent;
          actor = spotActor;
        }
      }

      if (spotComponent) {
        spotComponent->SetInnerConeAngle(FMath::RadiansToDegrees(lightReader.getInnerConeAngle()));
        spotComponent->SetOuterConeAngle(FMath::RadiansToDegrees(lightReader.getOuterConeAngle()));
        spotComponent->SetSourceRadius(lightReader.getSpotRadius());

        lightComponent = spotComponent;
      }
      else {
        UE_LOG(LogSceneProtocol, Warning,
          TEXT("Unable to find spot actor: %s. Has its type changed? This is not currently supported"),
          ANSI_TO_TCHAR(SPNB::Unreal::stringDataToStdString(response->getEntityToken()).c_str()));
        actor = nullptr;
      }
      break;
    }

    case STP::Readers::LightReader::Type::Directional:
    {
#ifdef STP_CUSTOM_ACTOR_CLASSES
      // Search for custom directional light actor
      ASceneProtocolDirectionalLightActor* directionalActorCustom = findActor<ASceneProtocolDirectionalLightActor>(package, response);
      if (directionalActorCustom)
      {
        lightComponent = directionalActorCustom->directionalComponent;
        actor = directionalActorCustom;
      }
#endif
      if (!lightComponent)
      {
        ADirectionalLight* directionalActor = findActor<ADirectionalLight>(package, response);

        if (directionalActor)
        {
          lightComponent = directionalActor->GetLightComponent();
          actor = directionalActor;
        }
      }

      if (!lightComponent) {
        UE_LOG(LogSceneProtocol, Warning,
          TEXT("Unable to find directional actor: %s. Has its type changed? This is not currently supported"),
          ANSI_TO_TCHAR(SPNB::Unreal::stringDataToStdString(response->getEntityToken()).c_str()));
        actor = nullptr;
      }
      break;
    }

    case STP::Readers::LightReader::Type::Point:
    {
      UPointLightComponent* pointComponent = nullptr;
#ifdef STP_CUSTOM_ACTOR_CLASSES
      // Search for custom point light actor
      ASceneProtocolPointLightActor* pointActorCustom = findActor<ASceneProtocolPointLightActor>(package, response);
      if (pointActorCustom)
      {
        pointComponent = pointActorCustom->pointComponent;
        actor = pointActorCustom;
      }
#endif
      if (!lightComponent)
      {
        APointLight* pointActor = findActor<APointLight>(package, response);

        if (pointActor)
        {
          pointComponent = pointActor->PointLightComponent;
          actor = pointActor;
        }
      }

      //Set point light specific values: radius
      if (pointComponent) {
        pointComponent->SetSourceRadius(lightReader.getPointRadius());
        lightComponent = pointComponent;
      }
      else {
        UE_LOG(LogSceneProtocol, Warning,
          TEXT("Unable to find point actor: %s. Has its type changed? This is not currently supported"),
          ANSI_TO_TCHAR(SPNB::Unreal::stringDataToStdString(response->getEntityToken()).c_str()));
        actor = nullptr;
      }

      break;
    }

    default:
      UE_LOG(LogTemp, Warning, TEXT("\tUnknown light type"));
    }

    if (actor) {
      standardSetup(*response, actor, lightComponent);
    }
    return true;
  }

  template<typename T>
  T* LightHandler::findActor(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response* response)
  {
    const std::string nodeName = SPNB::Unreal::stringDataToStdString(response->getEntityToken());
    if (nodeName.empty())
    {
      return nullptr;
    }

    T* actor = nullptr;

    for (TActorIterator<T> Itr(GEditor->GetEditorWorldContext().World()); Itr; ++Itr) {
      USceneProtocolUserData* userData = Cast<USceneProtocolUserData>(Itr->GetLightComponent()->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));
      if (userData && userData->itemToken == FName(*FString(nodeName.c_str()))) {
        actor = *Itr;
      }
    }

    return actor;
  }

#ifdef STP_METADATA_SUPPORT
  UMetadataAsset* LightHandler::createMetadata(UPackage* package, const STP::Client::Response *response, const FName& name)
  {
    UMetadataAsset* metadata = NewObject<UMetadataAsset>(package, name, RF_Standalone | RF_Public);
    SPNB::Unreal::AssetFactory::instance().FillMetadataAsset(metadata, response);
    FAssetRegistryModule::AssetCreated(metadata);
    metadata->MarkPackageDirty();
    return metadata;
  }
#endif

  bool LightHandler::createPointLight(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response)
  {
    const USceneProtocolOptions* options = SPNB::Unreal::NodeHandlerManager::get()->getOptions();
    if (!options)
    {
      return false;
    }

    const std::string nodeName = SPNB::Unreal::stringDataToStdString(response.getEntityToken());
    if (nodeName.empty())
    {
      return false;
    }

    UWorld* world = GEditor->GetEditorWorldContext().World();
    if (!world)
    {
      return false;
    }

    AActor * actor = nullptr;
    ULightComponent* lightComponent = nullptr;
    auto worldTransform = SPNB::Unreal::getWorldTransform(&response);

    UPointLightComponent * pointComponent;
    std::vector<UObject*> objs;

#ifdef STP_CUSTOM_ACTOR_CLASSES
    if (options->UseCustomActorClasses)
    {
      ASceneProtocolPointLightActor* sceneProtocolActor = createCustomActor<ASceneProtocolPointLightActor>(package, response);
      pointComponent = sceneProtocolActor->pointComponent;
      actor = sceneProtocolActor;
#ifdef STP_METADATA_SUPPORT
      objs.push_back(sceneProtocolActor->Metadata);
#endif
    }
    else
#endif
    {
      APointLight * pointActor = createActor<APointLight>(package, response);
      pointComponent = pointActor->PointLightComponent;
      actor = pointActor;
    }

    if (!actor || !pointComponent) {
      return false;
    }
    STP::Readers::LightReader lightReader(response.getContentReader());
    //Set point light specific values: radius
    pointComponent->SetSourceRadius(lightReader.getPointRadius());

    // Add custom user data
    USceneProtocolUserData* userData = NewObject< USceneProtocolUserData >(pointComponent, NAME_None, RF_Public);
    userData->itemToken = nodeName.c_str();
    pointComponent->AddAssetUserData(userData);
    
    standardSetup(response, actor, pointComponent);

    SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objs);

    return true;
  }

  bool LightHandler::createDirectionalLight(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response)
  {
    const USceneProtocolOptions* options = SPNB::Unreal::NodeHandlerManager::get()->getOptions();
    if (!options)
    {
      return false;
    }

    const std::string nodeName = SPNB::Unreal::stringDataToStdString(response.getEntityToken());
    if (nodeName.empty())
    {
      return false;
    }

    UWorld* world = GEditor->GetEditorWorldContext().World();
    if (!world)
    {
      return false;
    }


    AActor * actor = nullptr;
    ULightComponent* lightComponent = nullptr;
    auto worldTransform = SPNB::Unreal::getWorldTransform(&response);

    ULightComponent * directionalComponent;
    std::vector<UObject*> objs;

#ifdef STP_CUSTOM_ACTOR_CLASSES
    if (options->UseCustomActorClasses)
    {
      ASceneProtocolDirectionalLightActor* sceneProtocolActor = createCustomActor<ASceneProtocolDirectionalLightActor>(package, response);
      directionalComponent = sceneProtocolActor->directionalComponent;
      actor = sceneProtocolActor;
#ifdef STP_METADATA_SUPPORT
      objs.push_back(sceneProtocolActor->Metadata);
#endif
    }
    else
#endif
    {
      ADirectionalLight* directionalActor = createActor<ADirectionalLight>(package, response);
      directionalComponent = directionalActor->GetLightComponent();
      actor = directionalActor;
    }

    if (!actor || !directionalComponent) {
      return false;
    }

    // Add custom user data
    USceneProtocolUserData* userData = NewObject< USceneProtocolUserData >(directionalComponent, NAME_None, RF_Public);
    userData->itemToken = nodeName.c_str();
    directionalComponent->AddAssetUserData(userData);

    standardSetup(response, actor, directionalComponent);

    SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objs);

    return true;
  }

  bool LightHandler::createSpotLight(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response)
  {
    const USceneProtocolOptions* options = SPNB::Unreal::NodeHandlerManager::get()->getOptions();
    if (!options)
    {
      return false;
    }

    const std::string nodeName = SPNB::Unreal::stringDataToStdString(response.getEntityToken());
    if (nodeName.empty())
    {
      return false;
    }

    UWorld* world = GEditor->GetEditorWorldContext().World();
    if (!world)
    {
      return false;
    }


    AActor * actor = nullptr;
    ULightComponent* lightComponent = nullptr;
    auto worldTransform = SPNB::Unreal::getWorldTransform(&response);

    USpotLightComponent * spotComponent;
    std::vector<UObject*> objs;

#ifdef STP_CUSTOM_ACTOR_CLASSES
    if (options->UseCustomActorClasses)
    {
      ASceneProtocolSpotLightActor* sceneProtocolActor = createCustomActor<ASceneProtocolSpotLightActor>(package, response);
      spotComponent = sceneProtocolActor->spotComponent;
      actor = sceneProtocolActor;
#ifdef STP_METADATA_SUPPORT
      objs.push_back(sceneProtocolActor->Metadata);
#endif
    }
    else
#endif
    {
      ASpotLight * spotActor = createActor<ASpotLight>(package, response);
      spotComponent = spotActor->SpotLightComponent;
      actor = spotActor;
    }

    if (!actor || !spotComponent) {
      return false;
    }

    STP::Readers::LightReader lightReader(response.getContentReader());
    bool isOK = false;
    //Set the Spot Light specific values: lightHasInnerAndOuterCone = true; lightHasDirection = true;  lightHasRadius = true;
    spotComponent->SetInnerConeAngle(FMath::RadiansToDegrees(lightReader.getInnerConeAngle()));
    spotComponent->SetOuterConeAngle(FMath::RadiansToDegrees(lightReader.getOuterConeAngle()));
    spotComponent->SetSourceRadius(lightReader.getSpotRadius());

    // Add custom user data
    USceneProtocolUserData* userData = NewObject< USceneProtocolUserData >(spotComponent, NAME_None, RF_Public);
    userData->itemToken = nodeName.c_str();
    spotComponent->AddAssetUserData(userData);
    
    standardSetup(response, actor, spotComponent);

    SPNB::Unreal::PackageMapper::instance().addPackageObjects(package, objs);

    return true;
  }

  void LightHandler::standardSetup(const STP::Client::Response& response, const AActor* actor, ULightComponent* lightComponent)
  {
#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY
      SPNB::Unreal::attachParentActor(&response, const_cast<AActor*>(actor));
#endif

    // re-assign the transform to the Actor, otherwise it is concatenated against the value set in previous imports
    // see AActor::PostSpawnInitialize
    auto SceneRootComponent = actor->GetRootComponent();
    check(SceneRootComponent);

    auto worldTransform = SPNB::Unreal::getWorldTransform(&response);
    SceneRootComponent->SetWorldTransform(FTransform(FRotator(0.f, -90.f, 0.f)) * worldTransform);

    STP::Readers::LightReader lightReader(response.getContentReader());
    const auto color = lightReader.getColor();
    FLinearColor diffuseColor(color.r, color.g, color.b, 1.f);

    lightComponent->SetAffectDynamicIndirectLighting(false);
    lightComponent->SetAffectTranslucentLighting(false);
    lightComponent->SetCastShadows(true);
    lightComponent->SetIntensity(lightReader.getIntensity());
    lightComponent->SetLightColor(diffuseColor); // TODO: assuming ambient color is the same as diffuse
  }

  template<typename T>
  T* LightHandler::createActor(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response)
  {
    UWorld* world = GEditor->GetEditorWorldContext().World();

    FString nodeName = SPNB::Unreal::stringDataToFString(response.getEntityToken());
    auto worldTransform = SPNB::Unreal::getWorldTransform(&response);

    FActorSpawnParameters spawnParams;
    SPNB::Unreal::Package* upackage = static_cast<SPNB::Unreal::Package*>(package);

    T * actor = world->SpawnActor<T>(T::StaticClass(),
      worldTransform,
      spawnParams);

    FString displayName = SPNB::Unreal::stringDataToFString(response.getDisplayName());
    if (displayName.IsEmpty())
    {
      displayName = FPackageName::GetShortName(nodeName);
    }

    actor->SetActorLabel(*displayName);

    return actor;
  }

  template<typename T>
  T* LightHandler::createCustomActor(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response& response)
  {
    UWorld* world = GEditor->GetEditorWorldContext().World();
    const USceneProtocolOptions* options = SPNB::Unreal::NodeHandlerManager::get()->getOptions();

    T* actor = createActor<T>(package, response);

#ifdef STP_METADATA_SUPPORT
    if (options->ImportMetadata)
    {
      std::string nodeName = SPNB::Unreal::stringDataToStdString(response.getEntityToken());
      const FName metadataName = FName((nodeName + "_metadataAssets").c_str());
      actor->Metadata = createMetadata(static_cast<SPNB::Unreal::Package*>(package)->package(), &response, metadataName);
    }
#endif

    return actor;
  }
} // anon namespace
