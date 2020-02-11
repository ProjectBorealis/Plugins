// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.GlobalCPUConfig.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include "ULIS/Base/ULIS.Base.CPUConfig.h"


namespace ULIS {
/////////////////////////////////////////////////////
// FGlobalCPUConfig
class FGlobalCPUConfig
{
public:
    // Public API
    static FCPUConfig& Get();
}; 

} // namespace ULIS

