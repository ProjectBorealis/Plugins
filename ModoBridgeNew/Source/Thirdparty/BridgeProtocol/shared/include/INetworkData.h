// --------------------------------------------------------------------------
// Defines an interface for agnostic access of message data.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

namespace SceneTransmissionProtocol {
    /** Provides an interface for retrieval of message payloads.
     *
     * This interface provides the client a mechanism to access
     * data sent across the network, agnostic of the underlying
     * technology used.
     *
     */
    class INetworkData
    {
    public:
        virtual ~INetworkData();

        /** Retrieve the data sent from the server.
        */
        virtual const char *getData() const = 0;

        /** Retrieve the size of the data sent from the server.
        */
        virtual size_t getSize() const = 0;
    };

    inline INetworkData::~INetworkData() {}

} // namespace SceneTransmissionProtocol

