// --------------------------------------------------------------------------
// Declaration of utility string types.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once

#include <cstddef>

namespace SceneTransmissionProtocol {
namespace String {
    /**
    * Stores a potentially non-null delimited string and its size.
    */
    struct Data
    {
        Data() : str(nullptr), size(0) {}
        Data(const char* s, size_t length) : str(s), size(length) {}
        Data(const char* s);
        const char* str;
        size_t size;
    };

    /**
     * Determines whether a string is valid.
     * @param str The string to check.
     * @return Whether the string is valid.
     */
    bool stringDataIsValid(const Data& str);

    /**
     * Compares two \ref String::Data objects.
     * 
     * String::Data are equal if they have the same contents. They do not need
     * to be pointing at the same underlying data.
     * 
     * @param s1 First \ref String::Data to compare.
     * @parma s2 Second \ref String::Data to compare.
     * @return Whether the strings are equal.
     */
    bool equal(const Data& s1, const Data& s2);

    /**
    * Compares a \ref String::Data object and a null-terminated string.
    *
    * String::Data and a null-terminated string are equal if they have the same 
    * contents.
    *
    * @param s1 \ref String::Data to compare.
    * @parma s2 Null-terminated string to compare.
    * @return Whether the strings are equal.
    */
    bool equal(const Data& s1, const char* s2);
    
    /**
    * Compares a \ref String::Data object and a null-terminated string.
    *
    * String::Data and a null-terminated string are equal if they have the same
    * contents.
    *
    * @parma s1 Null-terminated string to compare.
    * @param s2 \ref String::Data to compare.
    * @return Whether the strings are equal.
    */
    bool equal(const char* s2, const Data& s1);

    /**
     * Certain parts of the STP API return strings. To ensure that the strings
     * are deallocated and because it is not possible to use std::string this
     * class provide RAII behaviour.
     * 
     * It is possible to override the class static allocation and deallocation
     * functions. These default to standard malloc and free calls.
     */
    class RAII
    {
    public:
      /**
       * Construct an RAII object, coying the data parameter to newly allocated
       * memory.
       */
        RAII(const char* data);

        /**
         * Ensure that the data is freed on destruction of the object.
         */
        ~RAII() {
            if (_data) {
                dealloc(_data);
            }
        }

        /**
         * Get a reference to the underlying string. Note that this may return
         * nullptr.
         */
        const char* operator*() const {
            return _data;
        }

        /**
         * Check whether the object contains a string.
         */
        explicit operator bool() const {
            return _data != nullptr;
        }

        /** 
         * Move constructor. Take ownership of the data from the other object
         * and clear its data.
         */
        RAII(RAII&& that) {
            _data = that._data;
            that._data = nullptr;
        }

        /**
         * Assignment operator. Take ownership of the data from the other object
         * and clear its data.
         */
        RAII& operator=(RAII&& that) {
            _data = that._data;
            that._data = nullptr;
            return *this;
        }

        RAII(RAII& that) = delete;
        RAII& operator=(RAII& that) = delete;

        /** Allocation API. This can be overridden at the class level. It will
         * default to using malloc and free.
         */
        typedef void* (*AllocCB)(size_t len);
        typedef void(*DeallocCB)(void* data);

        static AllocCB allocCB;
        static DeallocCB deallocCB;


    private:
      char* alloc(size_t len);
      void dealloc(char* data);
    private:
        char* _data;
    };


} // namespace String 
} // namespace SceneTransmissionProtocol
