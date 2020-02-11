// --------------------------------------------------------------------------
// Helper for accessing the fields of a mesh instance type STP reader or request. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Types.h"
#include "ArrayData.h"
#include "StringData.h"

namespace SceneTransmissionProtocol {
namespace Readers {
    class ContentReader;

    /** Provides constraints on a ContentReader that is of the protocol-specified
     *  MeshInstance type.
     *
     * This aids the host-application in reading a protocol-specified Mesh
     * type.
     */
    class MeshInstanceReader
    {
    public:
        /**
         Construct an instance of a mesh instance reader from a source ContentReader object.
         */
        MeshInstanceReader(const ContentReader& reader);

        /** Determines whether the reader contains a properly formed mesh instance.
         *
         * @return Whether the reader contains a properly formed mesh instance.
         */
        bool isValid() const;

        /** Get the mesh token that this mesh instance is linking
         * 
         * @return Mesh token string
         */
        String::Data getMeshToken() const;

        /** Get the underlying ContentReader
        *
        * @return The underlying ContentReader
        */
        const ContentReader& getContentReader() const;

    private:
        /** The underlying reader object. */
        const ContentReader& _reader;
    };


    inline MeshInstanceReader::MeshInstanceReader(const ContentReader& reader)
        : _reader(reader) {}

    inline const ContentReader& MeshInstanceReader::getContentReader() const
    {
        return _reader;
    }

} // namespace Readers
} // namespace SceneTransmissionProtocol

