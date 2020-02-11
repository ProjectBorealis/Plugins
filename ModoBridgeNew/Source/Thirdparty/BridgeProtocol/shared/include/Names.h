// --------------------------------------------------------------------------
// Declares all SGP-specified keys.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "StringData.h"
#include <cstdint>

namespace SceneTransmissionProtocol {

/** This header provides the the canonical names for protocol-specified keys.
 *
 * The library and users must only ever use these constants for these keys.
 * Doing otherwise is dangerous, as names may change in the future.
 *
 * It also provides a small number of functions for constructing keys that can
 * contain variable parts.
 */

namespace Names {
    /** Type names */
    extern const char* TypeInteger;
    extern const char* TypeFloat;
    extern const char* TypeDouble;
    extern const char* TypeString;

    extern const char* TypeIntegerArray;
    extern const char* TypeFloatArray;
    extern const char* TypeDoubleArray;
    extern const char* TypeStringArray;
    extern const char* TypeByteArray;

    extern const char* TypeInvalid;
    extern const char* TypeUnknown;
    extern const char* TypeUnsupported;

    /** Request names. */
    extern const char* RequestHeader;
    extern const char* RequestEntityToken;
    extern const char* RequestIdentifier;
    extern const char* RequestOperation;

    extern const char* RequestContent;
    extern const char* RequestType;
    extern const char* RequestParent;
    extern const char* RequestChildren;
    extern const char* RequestTransform;
    extern const char* RequestDisplayName;
    extern const char* RequestTypeData;
    extern const char* RequestDynamicData;

    extern const char* RequestTypeMesh;
    extern const char* RequestTypeMeshInstance;
    extern const char* RequestTypeMaterialDefinition;
    extern const char* RequestTypeMaterial;
    extern const char* RequestTypeLight;
    extern const char* RequestTypeCamera;
    extern const char* RequestTypeTexture;
    extern const char* RequestTypeUpdate;

    /** Response names. */
    extern const char* ResponseHeader;
    extern const char* ResponseEntityToken;
    extern const char* ResponseIdentifier;
    extern const char* ResponseStatus;
    extern const char* ResponseDisplayName;

    extern const char* ResponseContent;
    extern const char* ResponseType;
    extern const char* ResponseParent;
    extern const char* ResponseChildren;
    extern const char* ResponseTransform;
    extern const char* ResponseTypeData;
    extern const char* ResponseDynamicData;

    extern const char* ResponseTypeMesh;
    extern const char* ResponseTypeMeshInstance;
    extern const char* ResponseTypeMaterialDefinition;
    extern const char* ResponseTypeMaterial;
    extern const char* ResponseTypeLight;
    extern const char* ResponseTypeCamera;
    extern const char* ResponseTypeTexture;
    extern const char* ResponseTypeUpdate;

    /** Mesh names. */
    extern const char* MeshVertices;
    extern const char* MeshIndices;
    extern const char* MeshNormals;
    extern const char* MeshMaterial;
    extern const char* MeshFacesetCount;
    extern const char* MeshUVSetCount;
    extern const char* MeshRGBASetCount;

    /** UV set names.
     *
     * Note that UV sets have the form:
     *
     *   prefix.<N>.suffix
     *
     * where N is the index of the UV set. Use the functions below to
     * construct these forms.
     */
    extern const char* MeshUVSetPrefix;
    extern const char* MeshUVSetUVsSuffix;
    extern const char* MeshUVSetIndicesSuffix;

    String::RAII buildUVSetUVsName(int32_t index);
    String::RAII buildUVSetIndicesName(int32_t index);



    /** Mesh faceset names.
     *
     * Note that mesh facesets have the form:
     *
     *      prefix.<N>.suffix
     *
     * where N is the index of the faceset. Use the functions below to
     * construct these forms.
     */
    extern const char* MeshFacesetPrefix;
    extern const char* MeshFacesetFacesSuffix;
    extern const char* MeshFacesetMaterialSuffix;

    /** Construct a faceset face name with a specific index.
     *
     * The name will have the form:
     *
     *      MeshFacesetPrefix.<index>.MeshFacesetFacesSuffix
     *
     * @param index The index of the faceset.
     * @return A string containing the constructed key.
     */
    String::RAII buildFacesetFacesName(int32_t index);

    /** Construct a faceset material name with a specific index.
     *
     * The name will have the form:
     *
     *      MeshFacesetPrefix.<index>.MeshFacesetNameSuffix
     *
     * @param index The index of the faceset.
     * @return A string containing the constructed key.
     */
    String::RAII buildFacesetMaterialName(int32_t index);

    /** Color set names.
    *
    * Note that color sets have the form:
    *
    *   prefix.<N>.suffix
    *
    * where N is the index of the color set. Use the functions below to
    * construct these forms.
    */
    extern const char* MeshRGBASetPrefix;
    extern const char* MeshRGBASetColorsSuffix;
    extern const char* MeshRGBASetIndicesSuffix;

    String::RAII buildRGBASetRGBAName(int32_t index);
    String::RAII buildRGBASetIndicesName(int32_t index);

    /** Material definition common names. */
    extern const char* MaterialDefinitionModel;
    extern const char* MaterialDefinitionModelPBR;

    extern const char* MaterialDefinitionBlendMode;
    extern const char* MaterialDefinitionBlendOpaque;
    extern const char* MaterialDefinitionBlendTranslucent;

    extern const char* MaterialDefinitionTypeTexture;
    extern const char* MaterialDefinitionTypeFloat;
    extern const char* MaterialDefinitionTypeFloat2;
    extern const char* MaterialDefinitionTypeFloat3;
    extern const char* MaterialDefinitionTypeFloat4;
    extern const char* MaterialDefinitionTypeRGBA8;

    extern const char* MaterialDefinitionTargetColor;
    extern const char* MaterialDefinitionTargetMetallic;
    extern const char* MaterialDefinitionTargetRoughness;
    extern const char* MaterialDefinitionTargetNormal;
    extern const char* MaterialDefinitionTargetOpacity;
    extern const char* MaterialDefinitionTargetEmissiveColor;
    extern const char* MaterialDefinitionTargetSpecular;
    extern const char* MaterialDefinitionTargetSubSurfaceColor;
    extern const char* MaterialDefinitionTargetClearCoatAmount;
    extern const char* MaterialDefinitionTargetClearCoatRoughness;
    
    /** Material definition parameter names.
     * 
     * Note that material definition parameters have the form:
     *      
     *      prefix.<N>.suffix
     *      
     * where N is the index of the parameter and is larger than zero and less
     * than the value of found at MaterialDefinitionParameterCount. Use the
     * functions below to construct these forms.
     */
    extern const char* MaterialDefinitionParameterCount;
    extern const char* MaterialDefinitionParameterPrefix;
    extern const char* MaterialDefinitionParameterNameSuffix;
    extern const char* MaterialDefinitionParameterTypeSuffix;
    extern const char* MaterialDefinitionParameterTargetSuffix;
    extern const char* MaterialDefinitionParameterDefaultSuffix;

    /** Construct a material definition parameter name with a specific index.
     *
     * The name will have the form:
     *
     *      MaterialDefinitionParameterPrefix.<index>.MaterialDefinitionParameterNameSuffix
     *
     * @param index The index of the parameter.
     * @return A string containing the constructed key.
     */
    String::RAII buildMaterialDefinitionParameterName(int32_t index);

    /** Construct a material definition parameter type name with a specific index.
     *
     * The name will have the form:
     *
     *      MaterialDefinitionParameterPrefix.<index>.MaterialDefinitionParameterTypeSuffix
     *
     * @param index The index of the parameter.
     * @return A string containing the constructed key.
     */
    String::RAII buildMaterialDefinitionParameterTypeName(int32_t index);

    /** Construct a material definition parameter target name with a specific
     * index.
     *
     * The name will have the form:
     *
     *      MaterialDefinitionParameterPrefix.<index>.MaterialDefinitionParameterTargetSuffix
     *
     * @param index The index of the parameter.
     * @return A string containing the constructed key.
     */
    String::RAII buildMaterialDefinitionParameterTargetName(int32_t index);

    /** Construct a material definition parameter default value name with a specific
     * index.
     *
     * The name will have the form:
     *
     *      MaterialDefinitionParameterPrefix.<index>.MaterialDefinitionParameterValueSuffix
     *
     * @param index The index of the parameter.
     * @return A string containing the constructed key.
     */
    String::RAII buildMaterialDefinitionParameterValueName(int32_t index);


    /** Material common names. */
    extern const char* MaterialType;
    extern const char* MaterialTarget;

    extern const char* MaterialTypeCustom;

    /** Material input names.
     *
     * Note that material inputs have the form:
     *
     *      prefix.<N>.suffix
     *
     * where N is the index of the input and is larger than zero and less
     * than the value of found at MaterialInputCount. Use the functions below
     * to construct these forms.
     */
    extern const char* MaterialInputCount;
    extern const char* MaterialInputPrefix;
    extern const char* MaterialInputNameSuffix;
    extern const char* MaterialInputTypeSuffix;
    extern const char* MaterialInputValueSuffix;

    /** Construct a material input name with a specific index.
     *
     * The name will have the form:
     *
     *      MaterialInputPrefix.<index>.MaterialInputNameSuffix
     *
     * @param index The index of the input.
     * @return A string containing the constructed key.
     */
    String::RAII buildMaterialInputName(int32_t index);

    /** Construct a material input type name with a specific index.
     *
     * The name will have the form:
     *
     *      MaterialInputPrefix.<index>.MaterialInputTypeSuffix
     *
     * @param index The index of the input.
     * @return A string containing the constructed key.
     */
    String::RAII buildMaterialInputTypeName(int32_t index);

    /** Construct a material input value name with a specific index.
     *
     * The name will have the form:
     *
     *      MaterialInputPrefix.<index>.MaterialInputValueSuffix
     *
     * @param index The index of the input.
     * @return A string containing the constructed key.
     */
    String::RAII buildMaterialInputValueName(int32_t index);


    /** Camera names. */
    extern const char* CameraPerspective;
    extern const char* CameraOrthographic;

    extern const char* CameraType;
    extern const char* CameraNearClip;
    extern const char* CameraFarClip;
    extern const char* CameraPerspectiveVerticalFOV;
    extern const char* CameraPerspectiveAspectRatio;
    extern const char* CameraOrthoHorizMagnification;
    extern const char* CameraOrthoVertMagnification;

    /** Light names. */
    extern const char* LightDirectional;
    extern const char* LightPoint;
    extern const char* LightSpot;

    extern const char* LightType;
    extern const char* LightIntensity;
    extern const char* LightColor;

    extern const char* LightSpotInnerConeAngle;
    extern const char* LightSpotOuterConeAngle;
    extern const char* LightSpotRadius;

    extern const char* LightPointRadius;

    /** Texture names */
    extern const char* TextureFormat;
    extern const char* TextureFormatRawRGBA;
    extern const char* TextureFormatRawRGB;
    extern const char* TextureFormatRawGray;
    extern const char* TextureData;
    extern const char* TextureWidth;
    extern const char* TextureHeight;

    /** Update names */
    extern const char* UpdateFormat;
    extern const char* UpdateFormatIndividual;
    extern const char* UpdateFormatRecursive;
    extern const char* UpdateTokens;

    /** Mesh instance names */
    extern const char* MeshInstanceToken;

} // namespace Names
} // namespace SceneTransmissionProtocol
