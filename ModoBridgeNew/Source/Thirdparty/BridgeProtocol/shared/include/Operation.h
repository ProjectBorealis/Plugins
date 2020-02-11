// --------------------------------------------------------------------------
// Helper API for SGP Operations.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------
#pragma once
#include "StringData.h"

namespace SceneTransmissionProtocol {
    /** REST-like CRUD operations.
     *
     * Only Read need be supported in all conforming clients.
     */
namespace Operation {
    /** Possible operations for a Request.
     *
     * The protocol-specified string representation of these are:
     *
     * Enum Value | String
     * -----------|-------
     * CreateOp   | "create"
     * ReadOp     | "read"
     * UpdateOp   | "update"
     * DeleteOp   | "delete"
     * UnknownOp  | "unknown"
     *
     */
    enum Operation
    {
        CreateOp, ///< Create a scene graph location using the supplied data.

        ReadOp, ///< Read a scene graph location.

        UpdateOp, ///< Update a scene graph location.

        DeleteOp, ///< Delete a scene graph location.

        UnknownOp ///< Error value.
    };

    extern const char* kCreateOpString; ///< CreateOp stringified
    extern const char* kReadOpString; ///< ReadOp stringified
    extern const char* kUpdateOpString; ///< UpdateOp stringified
    extern const char* kDeleteOpString; ///< DeleteOp stringified
    extern const char* kUnknownOpString; ///< UnkonwnOp stringified

    /** Convert an Operation to its specified string representation.
     * @param op Operation to stringify.
     * @return Stringified char array of the operation.
     */
    const char* toString(Operation op);

    /** Convert a specified string representation to its Operation
     *  representation.
     * @param str Char array containing a string representation of the operation
     * @return The matching Operation to the string, or UnknownOp if not recognized.
     */
    Operation fromString(const char* str);

    /** Convert a specified string representation to its Operation
    *  representation.
    * @param str String::Data containing a string representation of the operation
    * @return The matching Operation to the string, or UnknownOp if not recognized.
    */
    Operation fromString(String::Data str);

} // namespace Operation
} // namespace SceneTransmissionProtocol

