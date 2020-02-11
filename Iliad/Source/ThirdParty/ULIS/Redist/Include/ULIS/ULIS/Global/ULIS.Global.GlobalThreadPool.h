// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.GlobalThreadPool.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include "ULIS/Thread/ULIS.Thread.Pool.h"


namespace ULIS {
/////////////////////////////////////////////////////
// FGlobalThreadPool
class FGlobalThreadPool
{
public:
    // Public API
    static FThreadPool& Get();
}; 

} // namespace ULIS

