// --------------------------------------------------------------------------
// Helper for constructing a server response of the texture type.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Types.h"
#include "ContentWriter.h"

namespace SceneTransmissionProtocol {
namespace Writers {

    /** A wrapper to aid in creating a conformant protocol-defined Texture type
     *  Response/Request.
     *
     * This class simplifies the process of creating protocol-defined Texture
     * Response/Request object. It provides a nicer API over the raw property
     * interface of the core Response/Request object. It provides a guide for
     * host-applications and ensures the Response/Request matches the protocol-
     * defined structure of a texture.
     *
     * It is the responsibility of the user to ensure that the wrapped Response/
     * Request is in the correct state, which is that it has had beginTypeData()
     * called on it. Note that the TextureWriter will need to be constructed before
     * calling beginTypeData, so that the correct field count can be calculated.
     *
     * The first thing that must occur after this is to call begin().
     * Following this, only the set* methods may be called. set* methods may
     * only be called once.
     *
     * isComplete() can be checked to ensure that all required fields have
     * been set. Once they have, Response/Request::endTypeData() should be called.
     *
     * Each set* method will return one of the ProgressStatus codes. Anything
     * apart from OK means that the operation has failed. Note that the error
     * checking is not intended to prevent bad data, but simply to ensure that
     * the construction is well-ordered and complete.
     *
     * Example of use constructing a Response (without error checking):
     *
     *   Response* response = new response(request, STP::Status::OKStatus);
     *   response->beginContent()
     *   response->setStandardContentFields(parent, children, transform);
     *   response->setType(STP::Names::ResponseTypeTexture);

     *   TextureWriter textureWriter(*response, format);
     *   response.beginTypeData(textureWriter.calculateFieldCount());
     *   
     *   textureWriter.begin();
     *   textureWriter.setRawData(...);
     *   
     *   response->endTypeData();
     *
     *   response->beginDynamicData(0);
     *   response->endDynamicData();

     *   response->endContent();
     *   if (response->isValid()) {
     *       // send response...
     *   }
     */
    class TextureWriter
    {
    public:
        /** Describes the encoding of the textures.
         *
         * Currently only Raw format is supported.
         */
        enum Format {
            RawRGBA, ///< 32 bit uncompressed RGBA.
            RawRGB,  ///< 24 bit uncompressed RGB.
            RawGray  ///< 8 bit uncompressed grayscale.
        };

        /** Construct a TextureWriter wrapping a ContentWriter object
         *  and configuring it to accept the optional fields.
         *
         * It is not valid to call setRequest after using this constructor.
         *
         * @param response The response object upon which we will set type data.
         * @param format The encoding of the texture.
         */
        TextureWriter(ContentWriter& writer, Format format);

        /**
         */
        ~TextureWriter() {}

        /**
         * Set any initial fields.
         * @param type Type to set.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus begin();


        /** Set the raw byte data in 32 bit uncompressed RGBA format.
         *
         * Mandatory method. May only be called once. May only be called on 
         * TextureWriters that have TextureWriter::Raw format.
         *
         * @param width Width of the texture in pixels.
         * @param height Height of the texture in pixels.
         * @param bytes width x height of image data in bytes.
         * @param count Number of bytes.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus setRawData(int32_t width, int32_t height, uint8_t* bytes, size_t count);


        /** Finalize the TextureWriter object.
         *
         *  Should be called once after all the necessary fields have been set.
         *  If this returns an OK status, response/request.endTypeData() can be called.
         * @return Status of the construction.
         */
        ContentWriter::WriteStatus isComplete();

        /** Calculate the total number of content fields required for this
         *  object.
         *
         *  @return The number of fields in the content section.
         */
        size_t calculateFieldCount() const;

    private:
        /** wrapped Response object. */
        ContentWriter& _writer;

        /** The texture format. */
        Format _format;

        /** Various flags to indicate how completion status of the
         *  TextureWriter.
         */
        bool _begun;
        bool _hadData;
        bool _completed;
    };
} // namespace Writers
} // namespace SceneTransmissionProtocol
