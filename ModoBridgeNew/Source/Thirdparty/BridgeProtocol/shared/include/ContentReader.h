// --------------------------------------------------------------------------
// API for reading STP body data. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once
#include "ArrayData.h"
#include <cstddef>

namespace SceneTransmissionProtocol {
    class INetworkData;

namespace Readers {

    class ContentReader
    {
    public:
        /**
        * Defines the part of the content section that the accessor functions
        * should search for their key.
        */
        enum Section {
            Type = 0,           ///< For predefined types.
            Dynamic = 1,        ///< For custom types and additional metadata.
            SectionCount
        };

    public:
        ContentReader(const INetworkData* data);
        ~ContentReader();

        /**
        * The following functions retrieve data from the type- or dynamic-data
        * sections by key.
        *
        * The functions return proxies for the underlying data that prevents
        * the need for a potentially expensive copy operation and keeps the
        * STL out of the public API.
        */

        /** Get a handle to an array by key from the given section.
        *
        * This will fail if the key does not exist in the given section. In
        * this instance, if isOK is non-null it will contain false on return.
        *
        * @param key Key of the array in the type section.
        * @param isOK If this is non-null, it will contain the true if the
        *             retrieval was successful, otherwise false.
        * @return A handle to the array if successful, otherwise
        *         kInvalidArrayHandle.
        */
        Array::Handle getArray(Section section, const char* key, bool* isOK) const;

        /** Get a handle to a binary array by key from the given section.
        *
        * Binary data is accessed separately to allow for packing schemes that
        * treat it differently.
        *
        * This will fail if the key does not exist in the given section. In
        * this instance, if isOK is non-null it will contain false on return.
        *
        * @param section The data section in which to look for the key.
        * @param key Key of the array in the type section.
        * @param isOK If this is non-null, it will contain the true if the
        *             retrieval was successful, otherwise false.
        * @return A handle to the array if successful, otherwise
        *         kInvalidArrayHandle.
        */
        Array::BinaryHandle getBinary(Section section, const char* key, bool* isOK) const;

        /** Get a single-precision floating-point value from the given
        *  section.
        *
        * This will fail if the key does not exist in the given section or the
        * value it contains is not a single-precision floating-point.
        *
        * If it fails and isOK is non-null, isOK will contain false on return.
        *
        * @param section The data section in which to look for the key.
        * @param key Key of the value in the type section.
        * @param isOK If non-null, will contain true if the retrieval was
        *             successful, otherwise false.
        * @return The value found at key. In case of error, it will return 0.f,
        *         so isOK must be used to detect errors.
        */
        float getFloat(Section section, const char* key, bool* isOK, float defaultValue = 0.f) const;

        /** Get a double-precision floating-point value from the given
        *  section.
        *
        * This will fail if the key does not exist in the given section or the
        * value it contains is not a double-precision floating-point.
        *
        * If it fails and isOK is non-null, isOK will contain false on return.
        *
        * @param section The data section in which to look for the key.
        * @param key Key of the value in the type section.
        * @param isOK If non-null, will contain true if the retrieval was
        *             successful, otherwise false.
        * @return The value found at key. In case of error, it will return 0.0
        *         so isOK must be used to detect errors.
        */
        double getDouble(Section section, const char* key, bool* isOK, double defaultValue = 0.0) const;

        /** Get a 32-bit signed integer value from the given section.
        *
        * This will fail if the key does not exist in the given section or the
        * value it contains is not a 32-bit signed integer.
        *
        * If it fails and isOK is non-null, isOK will contain false on return.
        *
        * @param section The data section in which to look for the key.
        * @param key Key of the value in the type section.
        * @param isOK If non-null, will contain true if the retrieval was
        *             successful, otherwise false.
        * @return The value found at key. In case of error, it will return 0
        *         so isOK must be used to detect errors.
        */
        int32_t getInteger(Section section, const char* key, bool* isOK, int defaultValue = 0.0) const;

        /** Get a string value from the given section.
        *
        * See the notes on \ref String::Data before using the return value.
        *
        * This will fail if the key does not exist in the type section or the
        * value it contains is not a string.
        *
        * If it fails and isOK is non-null, isOK will contain false on return.
        *
        * @param section The data section in which to look for the key.
        * @param key Key of the value in the type section.
        * @param isOK If non-null, will contain true if the retrieval was
        *             successful, otherwise false.
        * @return The value found at key. In case of error, it will return an
        *         invalid String::Data value. isOK can also be used to verify the
        *         result.
        */
        String::Data getString(Section section, const char* key, bool* isOK) const;

        /** Check whether a key exists within the given data section.
        *
        * @param key The key to check for.
        * @return True if the key exists, false if not.
        */
        bool hasKey(Section section, const char* key) const;

        /** Get the number of the fields in the given data section.
        *
        * @return The number of fields.
        */
        size_t getKeyCount(Section section) const;

        /** Get a key by index from the given data section.
        *
        * @param index The index of the key.
        * @return The key. If the index is invalid, an empty string is
        *         returned.
        */
        String::Data getKey(Section section, size_t index) const;

        /** Get the DataType of a field's value by its key from the given data
        *  section.
        *
        * @param key The key of the field whose value will be retrieved.
        * @return The data type of the value at that key. This will return
        *         Unknown if no field exists with the key.
        */
        Types::DataType getDataType(Section section, const char* key) const;

        /** Get the data type of a field's value by its index from the given
        *  data section.
        *
        * @param index The index of the field.
        * @return The DataType of the value at that index. This will return
        *         Unknown if the index is invalid.
        */
        Types::DataType getDataType(Section section, size_t index) const;

    private:
        class Pimpl;
        Pimpl* _pimpl;
    };
} // namespace Readers
} // namespace SceneTransmissionProtocol