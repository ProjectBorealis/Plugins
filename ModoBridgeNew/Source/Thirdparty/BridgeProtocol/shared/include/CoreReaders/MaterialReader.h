// --------------------------------------------------------------------------
// Helper for accessing the fields of a material-type STP reader or request. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "ContentReader.h"
#include "Types.h"

namespace SceneTransmissionProtocol {
namespace Readers {
    class ContentReader;

    /** Provides constraints on a ContentReader that is of the protocol-specified
     *  material type.
     */
    class MaterialReader
    {
    public:
        /** The type of the material.
         *
         * This can currently only be Custom as this is the only type
         * specified by the protocol. It exists to support future additions.
         */
        enum Type {
            Unknown,
            Custom
        };

    public:
        /** Construct a MaterialReader around a ContentReader.
         *
         * @param reader The ContentReader object to wrap.
         */
        MaterialReader(const ContentReader& reader);

        /** Check whether the ContentReader object contains a valid material.
         *
         * @return Whether the ContentReader object is a valid material.
         */
        bool isValid() const;

        /** Get the type of the material
         *
         * @return The type of the material.
         */
        Type getType() const;

        /** Get the client-side name of the material.
         *
         * @return The client-side name of the material.
         */
        String::Data getTarget() const;

        /** Get the number of inputs in this material.
         *
         * @return The number of inputs.
         */
        int32_t getInputCount() const;

        /** Get the name of an input.
         *
         * @param index The index of the input whose name is being queried.
         * @return The name of the input.
         */
        String::Data getInputName(int32_t index) const;

        /** Get the type of an input.
         *
         * @param index The index of the input whose type is being queried.
         * @return The type of the input.
         */
        Types::DataType getInputType(int32_t index) const;

        /** Get the underlying ContentReader
         * 
         * @return The underlying ContentReader
         */
        const ContentReader& getContentReader() const;

    private:
        /** The underlying ContentReader object.
         */
        const ContentReader& _reader;
    };

    inline MaterialReader::MaterialReader(
            const ContentReader& reader) : _reader(reader)
    {
    }

    inline const ContentReader& MaterialReader::getContentReader() const
    {
        return _reader;
    }
} // namespace Readers
} // namespace SceneTransmissionProtocol

