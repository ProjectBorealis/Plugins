// --------------------------------------------------------------------------
// Helpers for accessing msgpack data.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once
#include "msgpack.h"
#include "ArrayData.h"
#include "StringData.h"

namespace SceneTransmissionProtocol {

/** A small set of useful functions for the Client and Server libraries to use
 *  when interacting with MessagePack's C API, which is complete, but very
 *  simple. These should simplify interaction.
 *
 *  This header is for use by the libraries only. It is not expected to
 *  be published as part of the libraries' public APIs.
 */
namespace MsgPackUtils {
    /** Retrieve the msgpack object from a dictionary, when specifying a key.
     * @param obj Dictionary of other msgpack objects.
     * @param key Char array describing the name of the key to look up in the dictionary.
     * @return Value associated with the keyi in the dictionary.
     */
    msgpack_object getMapValue(msgpack_object obj, const char* key);

    /** Determine if two msgpack types are the same.
     * @param t1 First type
     * @param t2 Second type
     * @return true if they are the same, false otherwise.
     */
    bool typeEqual(msgpack_object_type t1, msgpack_object_type t2);

    /** Determine if a msgpack object has the same name as the given string.
     * @param obj msgpack object to compare.
     * @param str Char array to compare name of.
     * @return true if they are the same, false otherwise.
     */
    bool strequal(msgpack_object& obj, const char* str);

    /** Template for converting a MsgPack object to a type.
     *
     * The generic template is not implemented, as only a small number of
     * specific types can be converted.
     *
     * @param obj The MessagePack object to convert.
     * @return The value of the object of type T.
     */
    template <typename T>
    T as(msgpack_object obj);

    /** Convert a MessagePack object to a float.
     *
     * @param obj The MessagePack object to convert.
     * @return The value of the object as a float.
     */
    template <>
    inline float as<float>(msgpack_object obj)
    {
        assert(obj.type == MSGPACK_OBJECT_FLOAT32);
        return static_cast<float>(obj.via.f64);
    }

    /** Convert a MessagePack object to a double.
     *
     * @param obj The MessagePack object to convert.
     * @return The value of the object as a double.
     */
    template <>
    inline double as<double>(msgpack_object obj)
    {
        assert(obj.type == MSGPACK_OBJECT_FLOAT64);
        return obj.via.f64;
    }

    /** Convert a MessagePack object to a String::Data.
     *
     * @param obj The MessagePack object to convert.
     * @return The value of the obj as a string.
     */
    template <>
    inline String::Data as<String::Data>(msgpack_object obj)
    {
        assert(obj.type == MSGPACK_OBJECT_STR);

        return String::Data(obj.via.str.ptr, obj.via.str.size);
    }

    /** Convert a MessagePack object to an int32_t.
     *
     * @param obj The MessagePack object to convert.
     * @return The value of the object as an int32_t
     */
    template <>
    inline int32_t as<int32_t>(msgpack_object obj)
    {
        return static_cast<int32_t>(obj.via.i64);
    }

    /** Converts an Array::Handle to the underlying MessagePack array type.
    *
    * @param handle The Array::Handle to convert.
    * @return The msgpack_object_array.
    */
    inline msgpack_object_array handleToArray(Array::Handle handle) {
        msgpack_object_array arr = { handle.size, (msgpack_object*)handle.ptr };
        return arr;
    }

    inline Array::Handle arrayToHandle(msgpack_object_array* arr) {
        Array::Handle handle = { arr->ptr, arr->size };
        return handle;
    }

    inline msgpack_object_bin handleToBin(Array::BinaryHandle handle) {
        msgpack_object_bin bin = {handle.size, (const char*)handle.ptr };
        return bin;
    }

    inline Array::BinaryHandle binToHandle(msgpack_object_bin* bin) {
        Array::BinaryHandle handle = { (void*)bin->ptr, bin->size };
        return handle;
    }

    /** Get the Response::DataType type of a MessagePack object.
    *
    * @param obj The object whose type is to be retrived.
    * @return The Response::DataType of the object. This can be
    *         Response::DataType::Unsupported if the MessagePack object
    *         type is not supported.
    */
    Types::DataType getObjectDataType(msgpack_object obj);

    /** Get the Response::DataType type of a MessagePack array.
    *
    * Note that the STP expects that arrays contain items of only one type
    * and will not construct arrays otherwise, so this only checks the
    * first index to determine the type.
    *
    * @param obj The array whose type is to be retrieved.
    * @return The Response::DataType of the array. This can be
    *         Response::DataType::Unsupported if the MessagePack array
    *         type is not supported.
    */
    Types::DataType getArrayDataType(msgpack_object_array arr);

} // namespace MsgPackUtils 
} // namespace SceneTransmissionProtocol
