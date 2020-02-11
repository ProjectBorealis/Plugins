// --------------------------------------------------------------------------
// Helper for accessing the fields of a texture-type STP response or request. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "StringData.h"
#include "ArrayData.h"
#include <cstdint>

namespace SceneTransmissionProtocol {
namespace Readers {
    class ContentReader;

    /** Provides constraints on a ContentReader that is of the protocol-specified
     *  texture type.
     */
    class TextureReader
    {
    public:
        /**
         Standard texture projection types.
         */
        enum Format
        {
            RawRGBA, ///< 32 bit uncompressed RGBA.
            RawRGB, ///< 24 bit uncompressed RGB.
            RawGray, ///< 8 bit uncompressed gray-scale.
            Unknown ///< Unknown format
        };

    public:
        /** Construct the TextureReader wrapper around a ContentReader object:
         *
         * @param reader The ContentReader object containing a texture type.
         */
        TextureReader(const ContentReader& reader);

        /** Checks whether the wrapped ContentReader conforms to the texture type
         *  specified in the protocol.
         *
         * @return True if the ContentReader conforms, false otherwise.
         */
        bool isValid();

        /** Get the type of texture.
         *
         * @return One of the projection types. Will return Unknown if the
         *         wrapped ContentReader is not a valid texture.
         */
        Format getFormat() const;

        /** Get the width of the texture.
         * 
         * @return The width of the texture in pixels.
         */
        int32_t getWidth() const;

        /** Get the height of the texture.
         * 
         * @return The height of the texture in pixels.
         */
        int32_t getHeight() const;

        /** Get the texture pixels.
         * 
         * @return width * height * bytesPerPixel bytes of pixel data.
         */
        Array::BinaryHandle getData() const;

        /** Get the underlying ContentReader
        *
        * @return The underlying ContentReader
        */
        const ContentReader& getContentReader() const;

        static Format nameToFormat(const String::Data& name);
        static int32_t formatBytesPerPixel(Format fmt);

    private:
        const ContentReader& _reader;
    };


    inline TextureReader::TextureReader(const ContentReader& reader)
        : _reader(reader) {}

    inline const ContentReader& TextureReader::getContentReader() const
    {
        return _reader;
    }
} // namespace Readers
} // namespace SceneTransmissionProtocol
