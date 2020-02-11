// --------------------------------------------------------------------------
// Custom type for efficiently accessing array data in a Response or Request 
// object.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include "StringData.h"
#include "Types.h"
#include <cstddef>

namespace SceneTransmissionProtocol {
namespace Array {

    /**
    * A structure containing an opaque pointer to the underlying array data.
    * This exists so that the structure of the array data can be hidden from
    * the user, but they can still access the array's items as efficiently as
    * possible.
    */
    struct Handle
    {
        Handle() : ptr(nullptr), size(0) {}
        Handle(void* p, uint32_t s) : ptr(p), size(s) {}
        void* ptr;
        uint32_t size;
    };

    /**
     * A structure containing an opaque pointer to the underlying binary data.
     * This exists so that the structure of the array data can be hidden from
     * the user, but they can still access the array's items as efficiently as
     * possible.
     */
    struct BinaryHandle
    {
        BinaryHandle() : ptr(nullptr), size(0) {}
        BinaryHandle(void* p, uint32_t s) : ptr(p), size(s) {}
        void* ptr = nullptr;
        uint32_t size = 0;
    };

    /**
    * Invalid array handle.
    */
    extern const Handle kInvalidHandle;

    /**
    * Invalid array handle.
    */
    extern const BinaryHandle kInvalidBinaryHandle;

    /** Gets the DataType of the array.
    *
    * @param handle A handle to the array.
    * @return The type of the array's items.
    */
    Types::DataType getArrayType(Handle handle);

    /** Gets the size of the array
    *
    * @param handle A handle to the array.
    * @return The size of the array.
    */
    size_t getArraySize(Handle handle);

    /** Get a single-precision floating point value from an array.
    *
    * The array handle must contain items of the correct type. This should
    * have been verified with \ref getArrayType.
    *
    * The index requested should be smaller than the size of the array. This
    * should have been verified with \ref getArraySize.
    *
    * If there is an error due to either of the above conditions being false,
    * then 0.0f is returned and isOK is set to false, if this is non-null.
    *
    * @param handle A handle to the array
    * @param index The index of the item to retrieve.
    * @param isOK If this is non-null, then the success of the operation will
    *             be stored in it.
    * @return The value at index in the array if this is valid, otherwise 0.0f.
    */
    float getFloatArrayItem(Handle handle, size_t index, bool* isOK);

    /** Get a double-precision floating point value from an array.
    *
    * The array handle must contain items of the correct type. This should
    * have been verified with \ref getArrayType.
    *
    * The index requested should be smaller than the size of the array. This
    * should have been verified with \ref getArraySize.
    *
    * If there is an error due to either of the above conditions being false,
    * then 0.0 is returned and isOK is set to false, if this is non-null.
    *
    * @param handle A handle to the array
    * @param index The index of the item to retrieve.
    * @param isOK If this is non-null, then the success of the operation will
    *             be stored in it.
    * @return The value at index in the array if this is valid, otherwise 0.0.
    */
    double getDoubleArrayItem(Handle handle, size_t index, bool* isOK);

    /** Get a signed 32-bit integer value from an array.
    *
    * The array handle must contain items of the correct type. This should
    * have been verified with \ref getArrayType.
    *
    * The index requested should be smaller than the size of the array. This
    * should have been verified with \ref getArraySize.
    *
    * If there is an error due to either of the above conditions being false,
    * then 0 is returned and isOK is set to false, if this is non-null.
    *
    * @param handle A handle to the array
    * @param index The index of the item to retrieve.
    * @param isOK If this is non-null, then the success of the operation will
    *             be stored in it.
    * @return The value at index in the array if this is valid, otherwise 0.
    */
    int32_t getIntegerArrayItem(Handle handle, size_t index, bool* isOK);

    /** Get a string value from an array.
    *
    * The array handle must contain items of the correct type. This should
    * have been verified with \ref getArrayType.
    *
    * The index requested should be smaller than the size of the array. This
    * should have been verified with \ref getArraySize.
    *
    * If there is an error due to either of the above conditions being false,
    * then the returned StringData will have null values. If isOK is non-null
    * then it will contain false on return.
    *
    * @param handle A handle to the array
    * @param index The index of the item to retrieve.
    * @param isOK If this is non-null, then the success of the operation will
    *             be stored in it.
    * @return A StringData containing the potentially non-null delimited string
    *         and its length, otherwise a StringData with null string data and
    *         zero length.
    */
    String::Data getStringArrayItem(Handle handle, size_t index, bool* isOK);

    /**
     * Get the underlying binary data.
     * 
     * @param handle A handle to the binary data.
     * @return The underlying binary data.
     */
    uint8_t* getBinaryData(BinaryHandle handle);

    /**
     * Get the size of the underlying binary data.
     * 
     * @param handle A handle to the binary data whose size is being queried.
     * @return The size of the binary data in bytes.
     */
    size_t getBinarySize(BinaryHandle handle);

} // namespace Array
} // namespace SceneTransmissionProtocol
