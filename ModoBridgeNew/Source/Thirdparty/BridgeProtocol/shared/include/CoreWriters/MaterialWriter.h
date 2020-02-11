// --------------------------------------------------------------------------
// Helper for constructing a response or request of the material type.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "ContentWriter.h"
#include "Types.h"

namespace SceneTransmissionProtocol {
namespace Writers {

    /** A wrapper to aid in creating a conforming protocol-defined Material type
     *  Response/Request.
     *
     * This class simplifies the process of creating protocol-defined Material
     * Response/Request object. It provides a nicer API over the raw property
     * interface of the core Response/Request object. It provides a guide for
     * host-applications and ensures the Response/Request matches the protocol-
     * defined structure of a texture.
     *
     * It is the responsibility of the user to ensure that the wrapped Response/
     * Request is in the correct state, which is that it has had beginTypeData()
     * called on it. Note that the MaterialWriter will need to be constructed 
     * before calling beginTypeData, so that the correct field count can be 
     * calculated.
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
     * Example of use in constructing a Response (without error checking):
     *
     *   Response* response = new response(request, STP::Status::OKStatus);
     *   response->beginContent()
     *   response->setStandardContentFields(parent, children, transform);
     *   response->setType(STP::Names::ResponseTypeMaterial);
     *
     *   MaterialWriter materialWriter(*response, format);
     *   response.beginTypeData(materialWriter.calculateFieldCount());
     *
     *   materialWriter.begin();
     *   materialWriter.setType(...);
     *   materialWriter.setTarget(...);
     *   materialWriter.addInput(...);
     *
     *   response->endTypeData();
     *
     *   response->beginDynamicData(0);
     *   response->endDynamicData();
     *
     *   response->endContent();
     *   if (response->isValid()) {
     *       // send response...
     *   }
     */
    class MaterialWriter
    {
    public:
        /** Construct the MaterialWriter wrapper around a Response object:
         *
         * @param response The Response object containing a material type.
         * @param inputCount Number of inputs to be added to the material.
         */
        MaterialWriter(ContentWriter& writer, int32_t inputCount);

        /** Destruct this instance of the response.
         */
        ~MaterialWriter() {}

        /**
         Set any initial fields.
         @return Status of construction.
        */
        ContentWriter::WriteStatus begin();

        /**
         Set the type on the response.
         @param type Type to set.
         @return Status of construction.
         */
        ContentWriter::WriteStatus setType(const char* type);

        /**
         Set the target on the response.
         @param type Target to set.
         @return Status of construction.
         */
        ContentWriter::WriteStatus setTarget(const char* type);

        /**
         Add an input to the response.
         @param name Name of the input.
         @param values Array of char arrays as the value of the input.
         @param count Number of entries in the array.
         @return Status of construction.
         */
        ContentWriter::WriteStatus addInput(const char* name,
                                const char* const* values,
                                size_t count);

        /**
         Add an input to the response.
         @param name Name of the input.
         @param values Array of integers as the value of the input.
         @param count Number of entries in the array.
         @return Status of construction.
         */
        ContentWriter::WriteStatus addInput(const char* name,
                                const int32_t* values,
                                size_t count);

        /**
         Add an input to the response.
         @param name Name of the input.
         @param values Array of floats as the value of the input.
         @param count Number of entries in the array.
         @return Status of construction.
         */
        ContentWriter::WriteStatus addInput(const char* name,
                                const float* values,
                                size_t count);

        /**
         Add an input to the response.
         @param name Name of the input.
         @param values Array of doubles as the value of the input.
         @param count Number of entries in the array.
         @return Status of construction.
         */
        ContentWriter::WriteStatus addInput(const char* name,
                                const double* values,
                                size_t count);

        /** Check whether the MaterialWriter is complete.
         *
         *  Should be called once after all the necessary fields have been set.
         *  If this returns an OK status, response.endTypeData() can be called.
         *  @return Status of construction.
         */
        ContentWriter::WriteStatus isComplete();

        /** Calculate the total number of content fields required for this
        *  object.
        *
        *  @return The number of fields in the content section.
        */
        size_t calculateFieldCount() const;

    private:
        ContentWriter& _writer;
        int32_t _inputCount;
        int32_t _inputsAdded;
        bool _begun;
        bool _hadType;
        bool _hadTarget;
        bool _completed;
    };

} // namespace Writers
} // namespace SceneTransmissionProtocol

