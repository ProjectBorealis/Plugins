// --------------------------------------------------------------------------
// Helper for accessing the fields of a update-type SGP server reader. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Types.h"
#include "ArrayData.h"

namespace SceneTransmissionProtocol {
    namespace Readers {
        class ContentReader;
        /** Provides constraints on a ContentReader that is of the protocol-specified
        *  Update type.
        *
        * This aids the host-application in reading a protocol-specified Update
        * type.
        */
        class UpdateReader
        {
        public:
            /** Describes the encoding of the textures.
             *
             * Individual only updates items in the token array
             * Recursive goes over children of the token arrays
             *
             * Recursive supported for now
             */
            enum Format {
                Individual,
                Recursive
            };

            /**
            Construct an instance of a mesh reader from a source ContentReader object.
            */
            UpdateReader(const ContentReader& reader);

            /**
            Returns the validity of the update reader
            */
            bool isValid();

            /** Get update tokens in string array.
            *
            * @param tokenVect String vector of tokens
            */
            Array::Handle getUpdateTokens() const;

            /** Get the underlying ContentReader
            *
            * @return The underlying ContentReader
            */
            const ContentReader& getContentReader() const;


        private:
            /** The underlying reader object. */
            const ContentReader& _reader;
        };

        inline UpdateReader::UpdateReader(const ContentReader& reader)
            : _reader(reader) {}

        inline const ContentReader& UpdateReader::getContentReader() const
        {
            return _reader;
        }

    } // namespace Readers
} // namespace SceneTransmissionProtocol
