// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Base.CPUConfig.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the definition for the FCPUConfig struct.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/FeatureDetector/cpu_x86.h"

namespace ULIS {
/////////////////////////////////////////////////////
/// @struct     FCPUConfig
/// @brief      The FCPUConfig struct provides a small wrapper around the FeatureDetector interface.
struct FCPUConfig
{
public:
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------- Construction
    /// @fn         FCPUConfig()
    /// @brief      Default Constructor.
    /// @details    Initializes members by calling the appropriate functions in FeatureDetector.
    FCPUConfig()
    {
        info.detect_host();
    }

public:
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------- Public Data Members
    ::FeatureDetector::cpu_x86 info;    ///< public member info of type ::FeatureDetector::cpu_x86, all fields should be initialized by constructor.
};

} // namespace ULIS

