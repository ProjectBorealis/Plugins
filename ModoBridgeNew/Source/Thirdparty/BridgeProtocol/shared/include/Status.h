// --------------------------------------------------------------------------
// Helper API for SGP Status.
//
// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
// --------------------------------------------------------------------------

#pragma once

#include "StringData.h"

namespace SceneTransmissionProtocol {
namespace Status {

    enum Status
    {
        OKStatus,
        InvalidRequestStatus,
        OperationNotSupportedStatus,
        MissingStatus,
        MovedStatus,
        UnknownStatus
    };

    extern const char* kOKStatusString;
    extern const char* kInvalidRequestStatusString;
    extern const char* kOperationNotSupportedStatusString;
    extern const char* kMissingStatusString;
    extern const char* kMovedStatusString;
    extern const char* kUnknownStatusString;

    Status fromString(const char* str);
    Status fromString(const String::Data& str);

    const char* toString(Status status);
} // namespace Status
} // namespace SceneTransmissionProtocol
