// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Base.WeakPRNG.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the definition for the Weak PRNG utility functions.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Weak PRNG
static uint32 sWeakPRNGSeed = 5323;  ///< An arbitrary base seed for Weak PRNG.


/// @fn         ResetWeakPRNGSeed()
/// @brief      Reset the PRNG seed.
static inline void ResetWeakPRNGSeed()
{
    sWeakPRNGSeed = 5323;
}


/// @fn         GenerateWeakPRNG()
/// @brief      Reset the PRNG seed.
/// @return     An uint32 representing a pseudo random number beetween 0 and 65537.
static inline uint32 GenerateWeakPRNG()
{
    sWeakPRNGSeed = 8253729 * sWeakPRNGSeed + 2396403;
    return sWeakPRNGSeed % 65537;
}

} // ULIS

