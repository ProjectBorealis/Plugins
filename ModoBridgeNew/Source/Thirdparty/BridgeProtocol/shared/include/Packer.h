// --------------------------------------------------------------------------
// Provides an API for packing data into a Request or Response.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "StringData.h"
#include <cstdint>

namespace SceneTransmissionProtocol {

    /** Provides an interface over the underlying data and its hides the format
    *  Used.
    */
    class Packer
    {
    public:
        Packer();
        ~Packer();

        Packer(const Packer&) = delete;
        void operator=(const Packer&) = delete;

        /** Start to pack a map.
        *
        * Uses the packer to begin packing a map.
        *
        * @param fieldCount The number of fields the map will have.
        */
        void packMap(size_t fieldCount);

        /** Start to pack an array.
        *
        * Uses the packer to begin packing an array.
        *
        * @param fieldCount The number of fields the array will have.
        */
        void packArray(size_t fieldCount);

        /** Pack a blob of binary.
        *
        * Uses the packer to pack a blob of binary.
        *
        * @param data The binary to pack.
        * @param length The length of the binary data.
        */
        void packBinary(const uint8_t* data, size_t length);

        /** Pack an individual value.
        *
        * @param val The value to pack.
        */
        void pack(float val);
        void pack(double val);
        void pack(int32_t val);
        void pack(const char* val);
        void pack(const String::Data& val);

        /** Get the packed data.
        *
        * The SendRequest::Pimpl retain ownership of the buffer.
        *
        * @return The packed data.
        */
        const char * getData() const;

        /** Get the size of the packed data.
        *
        * @return The size of the packed data.
        */
        size_t getDataSize() const;

        /** Take the packed data and pass ownership to the caller
        *
        * The buffer will be null after this has been called. Ensure that the
        * size has been retrieved with getDataSize before calling this.
        *
        * @return The packed data.
        */
        char* takeData();

    private:
        class Pimpl;
        Pimpl* _pimpl;
    };
} // namespace SceneTransmissionProtocol
