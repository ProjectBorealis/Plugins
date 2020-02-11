// --------------------------------------------------------------------------
// Group handler.
//
// Copyright (c) 2019 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#pragma once

#include "SceneProtocol.h"

#ifdef STP_ENABLE_SCENEGRAPH_HIERARCHY

#include "IAssetHandler.h"
#include "NodeHandler.h"
#include "HandlerUtils.h"
#include "PackageMapper.h"
#include "SceneProtocolOptions.h"

#include "Editor/GroupActor.h"

namespace STP = SceneTransmissionProtocol;
namespace SPNB = SceneProtocol::NetworkBridge;

namespace {
    class GroupHandler : public SPNB::Unreal::IAssetHandler
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
    GroupHandler _theGroupHandler;

    bool GroupHandler::create(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
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

        auto &packageMapper = SPNB::Unreal::PackageMapper::instance();

        FActorSpawnParameters spawnParams;
        spawnParams.Name = FName(*nodeName); // note, not using displayName here, or there are name clashes, causing actor deletions
        auto worldTransform = SPNB::Unreal::getWorldTransform(response);

        // using an AGroupActor, as it is all about grouping, and has a USceneComponent already
        // so therefore has a transform
        AGroupActor* actor = GEditor->GetEditorWorldContext().World()->SpawnActor<AGroupActor>(AGroupActor::StaticClass(),
            worldTransform,
            spawnParams);

        if (nullptr == actor)
        {
            return false;
        }

        SPNB::Unreal::attachParentActor(response, actor);

        // re-assign the transform to the Actor, otherwise it is concatenated against the value set in previous imports
        // see AActor::PostSpawnInitialize
        auto SceneRootComponent = actor->GetRootComponent();
        check(nullptr != SceneRootComponent)
        SceneRootComponent->SetWorldTransform(worldTransform);

        // AttachToActor in attachParentActor() will fail only with a log message if this does not get set
        SceneRootComponent->SetMobility(EComponentMobility::Static);

        uniqueName = package->name();

        std::vector<UObject*> objects;
        objects.push_back(actor); // in order query for parentage from child groups
        packageMapper.addPackageObjects(package, objects);

        actor->Modify(true);
        actor->SetActorLabel(*displayName);

        return true;
    }

    bool GroupHandler::update(SPNB::BridgeProtocol::IPackage* package, const STP::Client::Response *response, std::string& uniqueName)
    {
        return true;
    }

    /** Factory creation function.
    */
    SPNB::Unreal::IAssetHandler *createGroupHandler()
    {
        return &_theGroupHandler;
    }

    /** Factory destruction function */
    void destroyGroupHandler(SPNB::Unreal::IAssetHandler* handler)
    {
        (void)(handler); // statically allocated, don't destroy.
    }

    /** Registration of DebugHandler against a type name, with its factory
    *  creation/destruction functions.
    */
    NETBRIDGE_ADD_ASSET_HANDLER(GroupHandler, "group", createGroupHandler, destroyGroupHandler);
} // anonymous namespace

#endif // STP_ENABLE_SCENEGRAPH_HIERARCHY
