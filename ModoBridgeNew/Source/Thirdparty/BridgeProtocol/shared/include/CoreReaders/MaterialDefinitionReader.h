// --------------------------------------------------------------------------
// Helper for accessing the fields of a material definition-type reader or
// request object.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "StringData.h"
#include <cstdint>

namespace SceneTransmissionProtocol {
namespace Readers {
    class ContentReader;

    /** Provides constraints on a ContentReader that is of the protocol-specified
    *  material definition type.
    */
    class MaterialDefinitionReader
    {
    public:
        /**
         * The shader model to be used. 
         * 
         * Currently only a single model is available, but this probably will 
         * be expanded in future.
         * 
         * ModelUnknown means that the server has sent a value that cannot be
         * mapped back to this enum. This is probably due to the server being
         * of a different version.
         */
        enum ShaderModel {
            StandardPBR,    ///< Physically-based, metallic roughness model
            ModelUnknown    ///< Model is not recognised.
        };


        /**
         * The blend mode of the shader model
         * 
         * ModeUnknown means that the server has sent a value that cannot be
         * mapped back to this enum. This is probably due to the server being
         * of a different version.
         */
        enum BlendMode {
            Opaque,         ///< Opaque blend mode
            Translucent,    ///< Translucent blend mode
            ModeUnknown     ///< Blend mode is not recognised.
        };

        /**
         * The type of a parameter to be mapped to a target.
         * 
         * TypeUnknown means that the server has sent a value that cannot be
         * mapped back to this enum. This is probably due to the server being
         * of a different version.
         */
        enum ParameterType {
            Float,          ///< Single floating point constant
            Float2,         ///< Double floating point constant
            Float3,         ///< Triple floating point constant
            Float4,         ///< Quadruple floating point constant
            RGBA8,          ///< 8 bit four element color.
            Texture,        ///< Entity token of a texture asset.
            TypeUnknown     ///< Type was not recognised.
        };

    public:
        /** Construct the MaterialDefinitionReader wrapper around a ContentReader 
         *  object:
         *
         * @param reader The ContentReader object containing a material definition
         *                 type.
         */
        MaterialDefinitionReader(const ContentReader& reader);

        /** Checks whether the wrapped ContentReader conforms to the material
         *  definition type specified in the protocol.
         *
         * @return True if the ContentReader conforms, false otherwise.
         */
        bool isValid() const;

        /** Get the shader model of the material definition.
         *
         * @return The shader model of the material definition.
         */
        ShaderModel getShaderModel() const; 

        /** Get the blend mode of the material definition.
         *
         * @return The blend mode of the material definition.
         */
        BlendMode getBlendMode() const;

        /** Get the number of parameters in this material definition.
         *
         * @return The number of parameters.
         */
        int32_t getParameterCount() const;

        /** Get the name of an parameter.
         *
         * @param index The index of the parameter whose name is being queried.
         * @return The name of the parameter.
         */
        String::Data getParameterName(int32_t index) const;

        /** Get the target of a parameter
         * 
         * @param index The index of the parameter whose target is being queried.
         * @return The target of the parameter.
         */
        String::Data getParameterTarget(int32_t index) const;

        /** Get the type of an parameter.
         *
         * @param index The index of the parameter whose type is being queried.
         * @return The type of the parameter.
         */
        ParameterType getParameterType(int32_t index) const;

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

    inline MaterialDefinitionReader::MaterialDefinitionReader(
        const ContentReader& reader) : _reader(reader)
    {
    }

    inline const ContentReader& MaterialDefinitionReader::getContentReader() const
    {
        return _reader;
    }
} // namespace Readers
} // namespace SceneTransmissionProtocol
