// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Base.WeakUUID.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declaration for the Weak UUID utility functions.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include <string>

namespace ULIS {
/////////////////////////////////////////////////////
// Weak UUID
/// @fn         GenerateWeakUUID()
/// @brief      Generate a weak UUID.
/// @details    Weak UUID is an UUID that is pseudo random but with no guarantee to be unique at all. This it can be suitable for light uses.
std::string GenerateWeakUUID( uint8 iLen );

} // ULIS

