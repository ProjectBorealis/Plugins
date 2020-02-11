// --------------------------------------------------------------------------
// Helper for constructing a server response of the material definition type.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "Types.h"
#include "ContentWriter.h"

namespace SceneTransmissionProtocol {
namespace Writers {

    /** A wrapper to aid in creating a conforming protocol-defined 
     * MaterialDefinition type Response/Request.
     *
     * This class simplifies the process of creating protocol-defined
     * MaterialDefinition Response/Request object. It provides a nicer API over 
     * the raw property interface of the core Response/Request object. It
     * provides a guide for host-applications and ensures the Response/Request
     * matches the protocol-defined structure of a texture.
     *
     * It is the responsibility of the user to ensure that the wrapped Response/
     * Request is in the correct state, which is that it has had beginTypeData()
     * called on it. Note that the MaterialDefinitionWriter will need to be
     * constructed before calling beginTypeData, so that the correct field
     * count can be calculated.
     *
     * The first thing that must occur after this is to call begin().
     * Following this, only the set* methods may be called. set* methods may
     * only be called once.
     *
     * isComplete() can be checked to ensure that all required fields have
     * been set. Once they have, Response/Request::endTypeData() should be
     * called.
     *
     * Each set* method will return one of the WriteStatus codes. Anything
     * apart from OK means that the operation has failed. Note that the error
     * checking is not intended to prevent bad data, but simply to ensure that
     * the construction is well-ordered and complete.
     *
     * Example of use constructing a response (without error checking):
     *
     *   Response* response = new response(request, STP::Status::OKStatus);
     *   response->beginContent()
     *   response->setStandardContentFields(parent, children, transform);
     *   response->setType(STP::Names::ResponseTypeMaterialDefinition);
     *
     *   MaterialDefinitionWriter materialDefWriter(*response, paramCount);
     *   response.beginTypeData(materialDefWriter.calculateFieldCount());
     *
     *   materialDefWriter.begin();
     *   materialDefWriter.setModel(...);
     *   materialDefWriter.setBlendMode(...);
     *   materialDefWriter.addParameter(...);
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
    class MaterialDefinitionWriter
    {
    public:
        /** 
         * The shading model used. The chosen type defines the conventional
         * fields available as targets.
         * 
         * Fields are as follows:
         *  StandardPBR: 
         *      Color,
         *      Metallic,
         *      Roughness,
         *      Normal,
         *      Opacity
         *      
         * This is a single-entry enum because it is anticipated further models
         * will be added in the future.
         */
        enum ShaderModel {
            StandardPBR ///< Standard physically-based metallic-roughness model.
        };

        /**
         * The blend mode to use in the shader.
         */
        enum BlendMode {
            Opaque, ///< No opacity, opacity field will be ignored.
            Translucent ///< Opacity field will be used.
        };

        /**
         * The type of the parameter to be mapped to a target.
         */
        enum ParameterType {
            Float,  ///< Single floating point constant.
            Float2, ///< Double floating point constant.
            Float3, ///< Triple floating point constant.
            Float4, ///< Quadruple floating point constant.
            RGBA8,  ///< 4 element 8 bit color.
            Texture ///< Entity token reference to texture location.
        };

    public:
        /** Construct the MaterialDefinitionWriter wrapper around a Response 
         *  object:
         *
         * @param response The Response object containing a material definition 
         *                 type.
         * @param parameterCount Number of inputs to be added to the definition.
         */
        MaterialDefinitionWriter(ContentWriter& response, int32_t parameterCount);

        /** Set any initial fields.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus begin();

        /** Set the shader model for this definition.
         * 
         * @param model The ShaderModel to use.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus setModel(ShaderModel model);

        /** Set the blend mode for this definition.
         * 
         * @param mode The blend mode to use.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus setBlendMode(BlendMode mode);

        /** Add a single float parameter to the definition.
         * 
         * @param name The name of the parameter.
         * @param target The Shader Model input to target.
         * @param defaultValue The default value to use for the parameter.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus addParameter(const char* name, const char* target, float defaultValue);

        /** Add an array of floats parameter to the definition.
         *
         * @param name The name of the parameter.
         * @param target The Shader Model input to target.
         * @param defaultValue The default value to use for the parameter.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus addParameter(const char* name, const char* target, const float* defaultValues, size_t count);

        /** Add a texture parameter to the definition.
         *
         * @param name The name of the parameter.
         * @param target The Shader Model input to target.
         * @param textureToken The default texture to use for the parameter.
         * @return Status of construction.
         */
        ContentWriter::WriteStatus addParameter(const char* name, const char* target, const char* textureToken);

        /** Finalize the MaterialDefinitionWriter object.
         *
         *  Should be called once after all the necessary fields have been set.
         *  If this returns an OK status, response.endTypeData() can be called.
         *
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
        int32_t _parameterCount;
        int32_t _parametersAdded;
        bool _begun;
        bool _hadModel;
        bool _hadBlendMode;
        bool _completed;
    };
} // namespace Writers
} // namespace SceneTransmissionProtocol
