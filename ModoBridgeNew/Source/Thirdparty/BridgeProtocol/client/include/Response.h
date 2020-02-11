// --------------------------------------------------------------------------
// API for accessing SGP server response data. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once
#include "Status.h"
#include "ArrayData.h"
#include "StringData.h"
#include "Types.h"

#include <stdint.h>
#include <cstddef>

namespace SceneTransmissionProtocol {
    /* Forward declaration of the interface describing network message data */
    class INetworkData;

    /* Forward declaration of the type containing the Response's content */
    namespace Readers { class ContentReader; }

namespace Client {    
    /** Wraps the data returned from the server and provides an API for
     *  unpacking.
     */
    class Response
    {
    public:
        /** Create a response around a buffer.
         * 
         * Note that it is assumed that the Response now has ownership of the
         * buffer and will dispose of it when the Response is destroyed.
         *
         * @param data The interface to the buffer containing the response data.
         */
        Response(INetworkData* data);

        /** Destroy the Response data, including destroying the buffer
         */
        ~Response();

        /** Gets the entity token from the Response.
         *
         * @return The entity token as a string.
         */
        String::Data getEntityToken() const;

        /** Gets the identifier from the Response.
         *
         * @return The identifier as a string.
         */
        String::Data getIdentifier() const;

        /** Gets the status from the Response.
         *
         * This will always return a valid value.
         *
         * @return the status as one of the Status enum values.
         */
        Status::Status getStatus() const;

        /** Get the type of the entity in this Response.
         *
         * @return The type string.
         */
        String::Data getType() const;

        /** Get the parent token of the entity in this Response.
         *
         * This can be empty if this is the root entity.
         *
         * @return The parent token string.
         */
        String::Data getParentToken() const;

        /** Get the count of children that the entity in this Response contains.
         *
         * @return The count of tokens.
         */
        size_t getChildCount() const;

        /** Get the child token at the given index.
         *
         * Index must be greater or equal to zero and less than the return
         * value from getChildCount().
         *
         * @param index The index of the token to retrieve.
         * @return Token associated with the child index specified.
         */
        String::Data getChildToken(size_t index) const;

        /** Get the local transform of this entity.
         * 
         * @return Handle to the transform array. Will be kInvalidArrayHandle if
         *         there is an error.
         */
        Array::Handle getTransform() const;

        /** Get the display name of the entity in this Response.
        *
        * This can be empty.
        *
        * @return The display name string.
        */
        String::Data getDisplayName() const;

        /** Get the \ref Body containing the \ref Response's content
         * 
         * @return The Response's content Body.
         */
        const Readers::ContentReader& getContentReader() const;

    private:
        class Pimpl;
        Pimpl* _pimpl;
    };

} // namespace Client
} // namespace SceneTransmissionProtocol

