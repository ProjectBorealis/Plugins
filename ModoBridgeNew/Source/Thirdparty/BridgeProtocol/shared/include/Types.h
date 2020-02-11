// --------------------------------------------------------------------------
// Declares SGP types.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "StringData.h"
#include <cstdint>

namespace SceneTransmissionProtocol {
/** Provides typedefs for the types used in the STP.
 */
namespace Types {
    typedef float vertex_elem_t; ///< Per position (triplets required)
    typedef int32_t index_elem_t; ///< Per index
    typedef float normal_elem_t; ///< Per normal component (triplets required)
    typedef float uv_elem_t; ///< Per UV index (pairs required)
    typedef int32_t face_elem_t; ///< Per face index
	typedef float rgba_elem_t; ///< Per face RGBA vertex color (quads required)

    /** The type of data stored in the Response object.
    */
    enum DataType {
        Unsupported,    //! Error value, when translating.
        Unknown,        //! Error value, when retrieving.
        Float,          //! Single precision floating point
        Double,         //! Double precision floating point
        String,         //! STP::String::Data
        Integer,        //! int32_t
        FloatArray,     //! Array of single precision floating point values
        DoubleArray,    //! Array of double precision floating point values.
        StringArray,    //! Array of STP::String::Data
        IntegerArray,   //! Array of int32_t
        ByteArray       //! Array of uint8_t
    };

    /** Convert a DataType to its string representation.
    *
    * @param dt The DataType to convert.
    * @return The string representation of the data type.
    */
    const char* dataTypeToString(DataType dt);

    /** Construct a DataType from its string representation.
    *
    * This version works on null-terminated strings.
    * 
    * @param str The string to convert.
    * @return The DataType constructed from the string.
    */
    DataType dataTypeFromString(const char* str);

    /** Construct a DataType from its string representation.
    *
    * This version works on String::Data string, which are not null-terminated.
    * @param str The string to convert.
    * @return The DataType constructed from the string.
    */

    DataType dataTypeFromString(const String::Data& str);

    struct Transform
    {
        Transform() 
            : elements{
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0
                }
        {}

        double elements[16];
    };

} // namespace Types
} // namespace SceneTransmissionProtocol
