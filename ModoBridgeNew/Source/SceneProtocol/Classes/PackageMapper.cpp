// --------------------------------------------------------------------------
// Implementation of the UnrealEngine PackageMapper.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#include "PackageMapper.h"
#include "MetadataUtils.h"
#include "SceneProtocol.h"
#include "NodeHandler.h"

#include "Handlers/HandlerUtils.h"

#ifdef STP_CUSTOM_ACTOR_CLASSES
#include "SceneProtocolStaticMeshActor.h"
#include "SceneProtocolPointLightActor.h"
#include "SceneProtocolSpotLightActor.h"
#include "SceneProtocolDirectionalLightActor.h"
#endif

#include "ContentReader.h"
#include "CoreReaders/MeshReader.h"
#include "CoreReaders/MaterialReader.h"
#include "Names.h"

#include "Editor.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"

#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "PackageTools.h"
#include "Developer/AssetTools/Public/IAssetTools.h"

#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SpotLight.h"
#include "Components/SpotLightComponent.h"

#include "Engine/StaticMeshActor.h"

#include "Camera/CameraComponent.h"

#include <cassert>

// Currently, a Package is created for each of the types. This is not strictly
// necessary for Unreal for instances of types that don't also create an asset, 
// i.e. cameras and lights that don't have metadata.
//
// However, Packages are used for book-keeping while fetching, so they shouldn't
// be removed without first devising a new way to do this.
//

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;
namespace SPNBBP = SceneProtocol::NetworkBridge::BridgeProtocol;

namespace SceneProtocol {
  namespace NetworkBridge {
    namespace Unreal {
      // Generates a package name given a root location and a response.
      // It will return an empty FString on error.
      FString PackageMapper::generatePackageName(const FString& packageRoot, const std::string& nodeName)
      {
        const USceneProtocolOptions* options = NodeHandlerManager::get()->getOptions();
        if (!options)
        {
          return FString();
        }

        if (nodeName.empty())
        {
          return FString();
        }

        FString packageName = packageRoot;
        FString nodeNameString = FString (nodeName.c_str());
        packageName /= GetHashedInputPath(nodeNameString);

        return packageName;
      }

      PackageMapper* PackageMapper::_instance = nullptr;

      PackageMapper& PackageMapper::instance()
      {
        if (!_instance)
        {
          _instance = new PackageMapper();
        }
        return *_instance;
      }

      bool PackageMapper::tokenExists(const std::string& token)
      {
        auto tokenPair = _tokenToPackageNameMap.find(token);
        if (tokenPair != _tokenToPackageNameMap.end())
        {
          return true;
        }
        //else
        return false;
      }

      // Removes a package the PackageManagers's internal caches and deletes the underlying package.
      void PackageMapper::removePackageByName(FName& token)
      {
        std::string packageName;
        auto pair = _tokenToPackageNameMap.find(TCHAR_TO_UTF8(*token.ToString()));
        if (pair != _tokenToPackageNameMap.end())
        {
          packageName = TCHAR_TO_UTF8(*pair->second.ToString());
          _tokenToPackageNameMap.erase(pair);
        }
        else
        {
          return;
        }

        auto it = _packageNameToPackageMap.find(*FString(packageName.c_str()));
        if (it != _packageNameToPackageMap.end()) {
          SPNBBP::IPackage* package = it->second;

          // Erase the package
          // TODO: Revisit this. Actor only packages get deleted by forced garbage colection.
          package->close ();
          delete package;

          // Force garbage collection
          GEngine->ForceGarbageCollection(true);

          _packageToObjectsMap.erase(it->second);
          _packageNameToPackageMap.erase(it);
        }
      }

      void PackageMapper::getObjectsByClassNames(const TArray<FName>& classNames, TArray<UObject*>& inMemoryObjects)
      {
        for (FName className : classNames)
        {
          UClass* Class = FindObjectFast<UClass> (nullptr, className, false, true, RF_NoFlags);
          if (Class != nullptr)
          {
            GetObjectsOfClass (Class, inMemoryObjects, false, RF_NoFlags);
          }
        }
      }

      bool PackageMapper::registerUnownedPackage(const std::string& token, FName packageName, UPackage* package, std::vector<UObject*>& objects)
      {
        if (packageName.IsNone() || token.empty())
          return false;

        if (tokenExists(token))
        {
          return false;
        }

        auto packagePair = _packageNameToPackageMap.find(packageName);
        if (packagePair == _packageNameToPackageMap.end())
        {
          _tokenToPackageNameMap.insert(TokenToPackageNameMapVal(token, packageName));

          // Add the package and token name
          UnownedPackage* unownedPackage = new UnownedPackage(package);
          unownedPackage->name(TCHAR_TO_UTF8(*packageName.ToString()));
          _packageNameToPackageMap.insert(std::pair<FName, UnownedPackage*>(packageName, unownedPackage));

          // Add the object array
          _packageToObjectsMap.insert(std::pair<SPNB::BridgeProtocol::IPackage*, std::vector<UObject*> >(unownedPackage, objects));

          return true;
        }
        // else
        return false;
      }

      bool PackageMapper::registerOwnedPackage(const std::string& token, SceneProtocol::NetworkBridge::BridgeProtocol::IPackage* package)
      {
        if (tokenExists(token))
        {
          return false;
        }

        FName packageName = ANSI_TO_TCHAR(package->name().c_str());
        auto packagePair = _packageNameToPackageMap.find(packageName);
        if (packagePair != _packageNameToPackageMap.end())
        {
          return false;
        }

        _tokenToPackageNameMap.insert(std::pair<std::string, FName>(token, packageName));
        _packageNameToPackageMap.insert(std::pair<FName, SceneProtocol::NetworkBridge::BridgeProtocol::IPackage*>(packageName, package));

        return true;
      }

      template < typename T >
      void PackageMapper::generateMapsHelper(const TArray<FName, FDefaultAllocator>& classNames, bool (PackageMapper::*RegFunc)(T*))
      {
          TArray<UObject*> inMemoryObjects;
          getObjectsByClassNames(classNames, inMemoryObjects);

          for (UObject* Object : inMemoryObjects)
          {
              T* castObj = Cast<T>(Object);
              (this->*RegFunc)(castObj);
          }
      }

      // Iterates through available items and regenerates package maps
      //
      // This method skips over items that don't have custom user data.
      // Items without custom user data haven't been recieved/sent by STP 
      // or user didn't add STP custom user data.
      void PackageMapper::generatePackageMaps()
      {
        //
        // Iterate over static mesh actors
        //
        TArray<FName> classNames;
#ifdef STP_CUSTOM_ACTOR_CLASSES
        classNames.Push(ASceneProtocolStaticMeshActor::StaticClass()->GetFName());
#endif
        classNames.Push(AStaticMeshActor::StaticClass()->GetFName());

        generateMapsHelper< AStaticMeshActor >(classNames, &PackageMapper::registerMeshInstance);

        //
        // Iterate over static meshes
        //
        classNames.Empty();
        classNames.Push(UStaticMesh::StaticClass()->GetFName());

        generateMapsHelper< UStaticMesh >(classNames, &PackageMapper::registerMesh);

        //
        // Iterate over lights
        //
        classNames.Empty();
#ifdef STP_CUSTOM_ACTOR_CLASSES
        classNames.Push(ASceneProtocolSpotLightActor::StaticClass()->GetFName());
        classNames.Push(ASceneProtocolDirectionalLightActor::StaticClass()->GetFName());
        classNames.Push(ASceneProtocolPointLightActor::StaticClass()->GetFName());
#endif
        classNames.Push(ASpotLight::StaticClass()->GetFName());
        classNames.Push(ADirectionalLight::StaticClass()->GetFName());
        classNames.Push(APointLight::StaticClass()->GetFName());

        generateMapsHelper< UObject >(classNames, &PackageMapper::registerLight);

        //
        // Iterate over material definitions
        //
        classNames.Empty();
        classNames.Push(UMaterial::StaticClass()->GetFName());

        generateMapsHelper< UMaterial >(classNames, &PackageMapper::registerMaterialDefinition);

        //
        // Iterate over material instances
        //
        classNames.Empty();
        classNames.Push(UMaterialInstanceConstant::StaticClass()->GetFName());

        generateMapsHelper< UMaterialInstanceConstant >(classNames, &PackageMapper::registerMaterial);

        //
        // Iterate over textures
        //
        classNames.Empty();
        classNames.Push(UTexture2D::StaticClass()->GetFName());

        generateMapsHelper< UTexture2D >(classNames, &PackageMapper::registerTexture);

        //
        // Iterate over cameras
        //
        classNames.Empty();
        classNames.Push(ACameraActor::StaticClass()->GetFName());

        generateMapsHelper< ACameraActor >(classNames, &PackageMapper::registerCamera);
      }

      // Clears package maps
      // Don't delete anything here, called on world unload or STP client stop
      void PackageMapper::clearMaps()
      {
        _tokenToPackageNameMap.clear ();
        _packageNameToPackageMap.clear ();
        _packageToObjectsMap.clear();
      }

      void PackageMapper::onActorDeletion(AActor* actor) {
        USceneProtocolUserData* userData = nullptr;

        if (
#ifdef STP_CUSTOM_ACTOR_CLASSES
          actor->IsA(ASceneProtocolStaticMeshActor::StaticClass()) || 
#endif
          actor->IsA(AStaticMeshActor::StaticClass())) 
        {
          AStaticMeshActor* smActor = Cast<AStaticMeshActor>(actor);
          UStaticMeshComponent* meshComponent = smActor->GetStaticMeshComponent();
          if (!meshComponent)
            return;

          userData = (USceneProtocolUserData*)meshComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
#ifdef STP_CUSTOM_ACTOR_CLASSES
        else if (actor->IsA(ASceneProtocolPointLightActor::StaticClass())) {
          ASceneProtocolPointLightActor* plActor = Cast<ASceneProtocolPointLightActor>(actor);
          userData = (USceneProtocolUserData*)plActor->pointComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
        else if (actor->IsA(ASceneProtocolDirectionalLightActor::StaticClass())) {
          ASceneProtocolDirectionalLightActor* dlActor = Cast<ASceneProtocolDirectionalLightActor>(actor);
          userData = (USceneProtocolUserData*)dlActor->directionalComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
        else if (actor->IsA(ASceneProtocolSpotLightActor::StaticClass())) {
          ASceneProtocolSpotLightActor* slActor = Cast<ASceneProtocolSpotLightActor>(actor);
          userData = (USceneProtocolUserData*)slActor->spotComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
#endif
        else if (actor->IsA(APointLight::StaticClass())) {
          APointLight* plActor = Cast<APointLight>(actor);
          userData = (USceneProtocolUserData*)plActor->PointLightComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
        else if (actor->IsA(ADirectionalLight::StaticClass())) {
          ADirectionalLight* dlActor = Cast<ADirectionalLight>(actor);
          userData = (USceneProtocolUserData*)dlActor->GetLightComponent()->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
        else if (actor->IsA(ASpotLight::StaticClass())) {
          ASpotLight* slActor = Cast<ASpotLight>(actor);
          userData = (USceneProtocolUserData*)slActor->SpotLightComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
        else if (actor->IsA(ACameraActor::StaticClass())) {
          ACameraActor* slActor = Cast<ACameraActor>(actor);
          userData = (USceneProtocolUserData*)slActor->GetCameraComponent()->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }

        if (userData && !userData->itemToken.IsNone()) {
          removePackageByName(userData->itemToken);
        }
      }

      void PackageMapper::onAssetsPreDeleted(const TArray<UObject*>& objects) {
        // Objects pending deletion has been changed so we need to clear the list
        _assetTokensPendingDeletion.clear();

        for (UObject* object : objects)
        {
          USceneProtocolUserData* userData = nullptr;

          if (object->IsA(UStaticMesh::StaticClass()))
          {
            UStaticMesh* mesh = Cast<UStaticMesh>(object);
            userData = Cast<USceneProtocolUserData>(mesh->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));
          }
          else if (object->IsA(UMaterial::StaticClass()))
          {
            UMaterial* material = Cast<UMaterial>(object);
            userData = Cast<USceneProtocolUserData>(material->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));
          }
          else if (object->IsA(UTexture2D::StaticClass()))
          {
            UTexture2D* texture = Cast<UTexture2D>(object);
            userData = Cast<USceneProtocolUserData>(texture->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));
          }
          else if (object->IsA(UMaterialInstanceConstant::StaticClass()))
          {
            UMaterialInstanceConstant* matInst = Cast<UMaterialInstanceConstant>(object);
            userData = Cast<USceneProtocolUserData>(matInst->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));
          }

          if (userData && !userData->itemToken.IsNone())
          {
            _assetTokensPendingDeletion.push_front(userData->itemToken);
          }

        }
      }

      void PackageMapper::onAssetsDeleted(const TArray<UClass*>& deletedAssetClasses) {
        for(FName tokenName : _assetTokensPendingDeletion)
        {
          removePackageByName(tokenName);
        }

        _assetTokensPendingDeletion.clear();
      }

      void PackageMapper::onInMemoryAssetsDeleted(UObject *Object) {
        onAssetsDeleted(TArray<UClass*>());
      }

      void PackageMapper::onWorldAdded(UWorld* world) {
        clearMaps();
        generatePackageMaps();
      }

      void PackageMapper::onLevelRemoved(ULevel* level, UWorld* world) {
        // empty
      }

      void PackageMapper::registerEventHandlers()
      {
        GEditor->OnLevelActorDeleted().AddRaw(this, &PackageMapper::onActorDeletion);
        GEditor->OnWorldAdded().AddRaw(this, &PackageMapper::onWorldAdded);
        //FWorldDelegates::LevelRemovedFromWorld.AddRaw(this, &PackageMapper::onLevelRemoved);
        FEditorDelegates::OnAssetsPreDelete.AddRaw(this, &PackageMapper::onAssetsPreDeleted);
        FEditorDelegates::OnAssetsDeleted.AddRaw(this, &PackageMapper::onAssetsDeleted);

        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        AssetRegistryModule.Get().OnInMemoryAssetDeleted().AddRaw(this, &PackageMapper::onInMemoryAssetsDeleted);
      }

      bool PackageMapper::checkUserDataValidity(USceneProtocolUserData* userData)
      {
          if (!userData)
          {
              return false;
          }

          if (userData->itemToken.IsNone())
          {
              return false;
          }

          if (tokenExists(TCHAR_TO_UTF8(*(userData->itemToken.ToString()))))
          {
              // If we have a duplicate token clear the storage for this item
              userData->itemToken = FName();
              return false;
          }

          return true;
      }

      void PackageMapper::registerObjectVect(std::vector<UObject*>& objects, USceneProtocolUserData* userData, FString packageRoot, UPackage* outermostPackage)
      {
          FString packageName = generatePackageName(FString(packageRoot), TCHAR_TO_UTF8(*(userData->itemToken.ToString())));
          registerUnownedPackage(TCHAR_TO_UTF8(*(userData->itemToken.ToString())), FName(*packageName), outermostPackage, objects);
      }

      bool PackageMapper::registerMeshInstance(AStaticMeshActor* staticMeshActor) {
        // Check if we have the custom user data applied to the mesh component
        UStaticMeshComponent* meshComponent = staticMeshActor->GetStaticMeshComponent();
        if (!meshComponent)
          return false;

        USceneProtocolUserData* actorUserData = (USceneProtocolUserData*)meshComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());

        if (!checkUserDataValidity(actorUserData))
        {
            return false;
        }

        // Add the object and the static mesh object
        std::vector<UObject*> objects;
        objects.push_back(staticMeshActor);
        registerObjectVect(objects, actorUserData, FString(MESH_INST_PACKAGE_ROOT), staticMeshActor->GetOutermost());

        return true;
      }

      bool PackageMapper::registerMesh(UStaticMesh* staticMesh) {
        USceneProtocolUserData* meshUserData = Cast<USceneProtocolUserData>(staticMesh->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass()));

        if (!checkUserDataValidity(meshUserData))
        {
            return false;
        }

        std::vector<UObject*> objects;
        objects.push_back(staticMesh);
        registerObjectVect(objects, meshUserData, FString(MESH_PACKAGE_ROOT), staticMesh->GetOutermost());

        return true;
      }

      bool PackageMapper::registerLight(UObject* lightObject) {
        USceneProtocolUserData* userData = nullptr;
#ifdef STP_CUSTOM_ACTOR_CLASSES
        if (lightObject->IsA(ASceneProtocolSpotLightActor::StaticClass()))
        {
          ASceneProtocolSpotLightActor* spotLightActor = (ASceneProtocolSpotLightActor*)lightObject;
          userData = (USceneProtocolUserData*)spotLightActor->spotComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
        else if (lightObject->IsA(ASceneProtocolDirectionalLightActor::StaticClass()))
        {
          ASceneProtocolDirectionalLightActor* directionalLightActor = (ASceneProtocolDirectionalLightActor*)lightObject;
          userData = (USceneProtocolUserData*)directionalLightActor->directionalComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
        else if (lightObject->IsA(ASceneProtocolPointLightActor::StaticClass()))
        {
          ASceneProtocolPointLightActor* pointLightActor = (ASceneProtocolPointLightActor*)lightObject;
          userData = (USceneProtocolUserData*)pointLightActor->pointComponent->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }
        else 
#endif  
        if (lightObject->IsA(ASpotLight::StaticClass()) || lightObject->IsA(ADirectionalLight::StaticClass()) || lightObject->IsA(APointLight::StaticClass()))
        {
          ALight* lightActor = (ALight*)lightObject;
          userData = (USceneProtocolUserData*)lightActor->GetLightComponent()->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());
        }

        if (!checkUserDataValidity(userData))
        {
            return false;
        }

        // Add the object
        std::vector<UObject*> objects;
        objects.push_back(lightObject);
        registerObjectVect(objects, userData, FString(LIGHTS_PACKAGE_ROOT), lightObject->GetOutermost());

        return true;
      }

      bool PackageMapper::registerMaterialDefinition(UMaterial* material) {
        USceneProtocolUserData* userData = (USceneProtocolUserData*)material->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());

        if (!checkUserDataValidity(userData))
        {
            return false;
        }

        // Add the object
        std::vector<UObject*> objects;
        objects.push_back(material);
        registerObjectVect(objects, userData, FString(MATERIAL_DEF_PACKAGE_ROOT), material->GetOutermost());

        return true;
      }

      bool PackageMapper::registerMaterial(UMaterialInstanceConstant* matInstance) {
        USceneProtocolUserData* userData = (USceneProtocolUserData*)matInstance->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());

        if (!checkUserDataValidity(userData))
        {
            return false;
        }

        // Add the object
        std::vector<UObject*> objects;
        objects.push_back(matInstance);
        registerObjectVect(objects, userData, FString(MATERIAL_INST_PACKAGE_ROOT), matInstance->GetOutermost());

        return true;
      }

      bool PackageMapper::registerTexture(UTexture2D* texture) {
        USceneProtocolUserData* userData = (USceneProtocolUserData*)texture->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());

        if (!checkUserDataValidity(userData))
        {
            return false;
        }

        // Add the object
        std::vector<UObject*> objects;
        objects.push_back(texture);
        registerObjectVect(objects, userData, FString(TEXTURES_PACKAGE_ROOT), texture->GetOutermost());

        return true;
      }

      bool PackageMapper::registerCamera(ACameraActor* cameraPawn) {
        USceneProtocolUserData* userData = (USceneProtocolUserData*)cameraPawn->GetCameraComponent()->GetAssetUserDataOfClass(USceneProtocolUserData::StaticClass());

        if (!checkUserDataValidity(userData))
        {
            return false;
        }

        // Add the object
        std::vector<UObject*> objects;
        objects.push_back(cameraPawn);
        registerObjectVect(objects, userData, FString(CAMERA_PACKAGE_ROOT), cameraPawn->GetOutermost());

        return true;
      }

      UObject* PackageMapper::findObject(const std::string& itemToken) const
      {
        auto uniqueNamePair = _tokenToPackageNameMap.find(itemToken);
        if (uniqueNamePair == _tokenToPackageNameMap.end())
        {
          return nullptr;
        }

        auto packagePair = _packageNameToPackageMap.find(uniqueNamePair->second);
        if (packagePair == _packageNameToPackageMap.end())
        {
          return nullptr;
        }
        
        auto packageObjects = _packageToObjectsMap.find(packagePair->second);
        if (packageObjects == _packageToObjectsMap.end())
        {
          return nullptr;
        }
        
        return packageObjects->second[0];
      }

      bool PackageMapper::hasPackage(FName uniqueName)
      {
          auto packagePair = _packageNameToPackageMap.find(uniqueName);
          if (packagePair == _packageNameToPackageMap.end())
          {
              return false;
          }
          return true;
      }

      void PackageMapper::getPackageNameFromToken(const std::string& token, std::string& packageName) const
      {
        auto pair = _tokenToPackageNameMap.find(token);
        if (pair != _tokenToPackageNameMap.end())
        {
          packageName = TCHAR_TO_UTF8(*pair->second.ToString());
        }
      }

      SceneProtocol::NetworkBridge::BridgeProtocol::IPackage* PackageMapper::getPackageFromPackageName(FString packageName) const
      {
        auto pair = _packageNameToPackageMap.find(FName(*packageName));
        if (pair != _packageNameToPackageMap.end())
        {
          return pair->second;
        }

        return nullptr;
      }

	  UMaterialInterface* PackageMapper::getUMaterialByName (
		  const std::string &materialName,
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 23
		  const std::map<FName, SPNBBP::IPackage*, cmpFNames>& packages,
#else
		  const std::map<FName, SPNBBP::IPackage*>& packages,
#endif
		  const std::map<SPNBBP::IPackage*, std::vector<UObject*>>& packageObjects)
	  {
		  std::string packageName;
		  getPackageNameFromToken (materialName, packageName);
		  auto package = packages.find (ANSI_TO_TCHAR (packageName.c_str ()));
		  if (package != packages.end ())
		  {
			  auto objects = packageObjects.find (package->second);
			  if (objects != packageObjects.end ())
			  {
				  return Cast<UMaterialInterface> (objects->second[0]);
			  }
		  }
		  UE_LOG (LogTemp, Warning, TEXT ("\tMaterial '%s' was not found as a UObject"), *FString (materialName.c_str ()));
		  return nullptr;
	  }

      std::vector<UMaterialInterface*> PackageMapper::getMaterials(const SceneTransmissionProtocol::Client::Response *response)
      {
        STP::Readers::MeshReader meshReader(response->getContentReader());
        std::vector<UMaterialInterface*> materials;
        const auto numFacesets = meshReader.getFacesetCount();
        if (0 == numFacesets)
        {
          const auto materialName = SPNB::Unreal::stringDataToStdString(meshReader.getMaterial());
          UE_LOG(LogTemp, Log, TEXT("\tLooking for material '%s' on mesh '%s'"), *FString(materialName.c_str()), *SPNB::Unreal::stringDataToFString(response->getEntityToken()));
          materials.push_back(getUMaterialByName(materialName, _packageNameToPackageMap, _packageToObjectsMap));
        }
        else
        {
          for (auto i = 0; i < numFacesets; ++i)
          {
            const auto materialName = SPNB::Unreal::stringDataToStdString(meshReader.getFacesetMaterial(i));
            UE_LOG(LogTemp, Log, TEXT("\tLooking for material '%s' on mesh '%s' for faceset %i"), *FString(materialName.c_str()), *SPNB::Unreal::stringDataToFString(response->getEntityToken()), i);
            materials.push_back(getUMaterialByName(materialName, _packageNameToPackageMap, _packageToObjectsMap));
          }
        }
        return materials;
      }

      UTexture2D* PackageMapper::getTexture(const std::string& uniqueName) const
      {
        auto package = _packageNameToPackageMap.find(ANSI_TO_TCHAR(uniqueName.c_str()));
        if (package != _packageNameToPackageMap.end())
        {
          auto objects = _packageToObjectsMap.find(package->second);
          if (objects != _packageToObjectsMap.end())
          {
            return Cast<UTexture2D>(objects->second[0]);
          }
        }
        return nullptr;
      }

      void PackageMapper::addPackageObjects(SPNB::BridgeProtocol::IPackage* package, std::vector<UObject*>& objects)
      {
        auto itr = _packageToObjectsMap.find(package);
        if (itr != _packageToObjectsMap.end()) {
          itr->second = objects;
        }
        else {
          _packageToObjectsMap.insert(std::pair<SPNB::BridgeProtocol::IPackage*, std::vector<UObject*> >(package, objects));
        }
      }
    }
  }
}