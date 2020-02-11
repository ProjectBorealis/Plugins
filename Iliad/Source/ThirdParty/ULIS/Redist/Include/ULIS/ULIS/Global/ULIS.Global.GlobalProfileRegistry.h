// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Global.GlobalProfileRegistry.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include "ULIS/Color/ULIS.Color.ProfileRegistry.h"


namespace ULIS {
/////////////////////////////////////////////////////
// FGlobalProfileRegistry
class FGlobalProfileRegistry
{
public:
    // Public API
    static FProfileRegistry& Get();
}; 

} // namespace ULIS

