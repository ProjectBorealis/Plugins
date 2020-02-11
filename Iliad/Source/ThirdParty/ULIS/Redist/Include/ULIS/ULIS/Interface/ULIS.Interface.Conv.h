// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Interface.Conv.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declarations for the FConversionContext class.
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"

ULIS_CLASS_FORWARD_DECLARATION( IBlock )

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      FConversionContext
/// @brief      The FConversionContext class provides a context for Converting layout, types and colorspaces on Blocks.
class FConversionContext
{
public:
//--------------------------------------------------------------------------------------
//-------------------------------------------------------------------- Public Static API
    /// @fn         ConvTypeAndLayoutInto( const IBlock* iBlockSrc, IBlock* iBlockDst, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    /// @brief      Convert a block from source format and layout to the destination format and layout.
    /// @details    The source and destination can be any layout and type but they must share the same underlying color model.
    ///                 - Warning ! Source and destination must share the same width and height.
    ///                 - If the source and destination are the exact same type and layout, a copy of source into destination is performed instead.
    /// @param      iBlockSrc               The pointer to the source \e IBlock, const, it will not be modified.
    /// @param      iBlockDst               The pointer to the destination \e IBlock, it will receive the conversion result.
    /// @param      iPerformanceOptions     The Performance Options for this operation, see \e FPerformanceOptions.
    static  void  ConvTypeAndLayoutInto( const IBlock* iBlockSrc
                                       , IBlock* iBlockDst
                                       , const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() );
};

} // namespace ULIS

