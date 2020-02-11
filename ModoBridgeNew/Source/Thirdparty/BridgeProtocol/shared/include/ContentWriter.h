// --------------------------------------------------------------------------
// API for writing STP content data. 
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once
#include "ArrayData.h"
#include <cstddef>

namespace SceneTransmissionProtocol {
    class Packer;
namespace Writers {

    /**
     * A ContenWriter provides an API for writing the content part of a 
     * \ref Response or \ref Request.
     */
    class ContentWriter
    {
    public:
        /**
         * Returned from the API to indicate success or failure, including a
         * reason in the latter case.
         */
        enum WriteStatus {
            OK,                    ///> Operation was successful.
            InvalidActiveSection,  ///> Active section is None.
            SectionsComplete,      ///> Attempting to progress beyond Dynamic active section
            InsufficientFields,    ///> Not enough fields added before ending stage.
            TooManyFields,         ///> Too many fields added to data section.
            IncorrectFieldForType, ///> Attempted to set a type-data field which is incorrect for the type.
            DuplicateField,        ///> Attempted to add named field again.
            ConfigMismatchForType  ///> Attempted to set a type-data field which breaks a configuration constraint (i.e. adding too many instances of an optional field)

        };

    private:
        /**
         * The ContentWriter has three stages in which it can write and starts
         * in fourth, non-writable stage.
         * 
         * The ActiveSection is used to track progress through these stages.
         */
        enum ActiveStage {
            None,
            Standard,
            Type,
            Dynamic
        };
    public:
        /**
         * A ContentWriter must be initialized with a Packer.
         */
        ContentWriter(Packer& packer);

        /**
         * 
         */
        WriteStatus nextStage(size_t fieldCount);

        /**
         * Check whether the current stage is complete.
         */
        bool currentStageIsComplete() const;

        /** Add a single float values to the current content data section.
        *
        * @param key The key to which the value is mapped.
        * @param value The value.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, float value);

        /** Add a single float values to the current content data section.
        *
        * @param key The key to which the value is mapped.
        * @param value The value.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, double value);

        /** Add a single float values to the current content data section.
        *
        * @param key The key to which the value is mapped.
        * @param value The value.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, const char* value);

        /** Add a single float values to the current content data section.
        *
        * @param key The key to which the value is mapped.
        * @param value The value.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, int32_t value);

        /** Add an array of integer values to the current content data section.
        *
        * @param key The key to which the values are mapped.
        * @param values The array of values.
        * @param len The length of the array.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, const int32_t* values, size_t len);

        /** Add an array of float values to the current content data section.
        *
        * @param key The key to which the values are mapped.
        * @param values The array of values.
        * @param len The length of the array.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, const float* values, size_t len);

        /** Add an array of values to the current content data section.
        *
        * @param key The key to which the values are mapped.
        * @param values The array of values.
        * @param len The length of the array.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, const double* values, size_t len);

        /** Add an array of string values to the current content data section.
        *
        * @param key The key to which the values are mapped.
        * @param values The array of values.
        * @param len The length of the array.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, const char* const* values, size_t len);

        /** Add an array of byte values to the current content data section.
        *
        * Always stored as a ByteArray, as there is no concept of a single
        * Byte.
        *
        * @param key The key to which the values are mapped.
        * @param values The array of values.
        * @param len The length of the array.
        * @return The status of the construction.
        */
        WriteStatus addField(const char* key, const uint8_t* values, size_t len);

    private:
        WriteStatus validateAddition() const;
        void reduceCount();
    private:
        Packer& _packer;
        size_t _fieldCountRemaining;
        ActiveStage _activeSection;
    };
} // namespace Writers
} // namespace SceneTransmissionProtocol
