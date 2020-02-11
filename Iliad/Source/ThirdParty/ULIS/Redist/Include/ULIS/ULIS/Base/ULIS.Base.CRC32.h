// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Base.CRC32.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declaration for the CRC32 function.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"

namespace ULIS {
/////////////////////////////////////////////////////
/// @fn         CRC32( const uint8* iData, int iLen )
/// @brief      Simple CRC32 computation for hashing purposes.
/// @details    Hashes the data of with specified size in bytes. The implementation is not optimal in terms of efficiency, but is simple and straightforward.
/// @param      iData The data to hash, cast as pointer to const uint8.
/// @param      iLen The size of the data to hash in bytes.
/// @return     An uint32 representing the CRC32 hash.
uint32 CRC32( const uint8* iData, int iLen );

} // namespace ULIS

