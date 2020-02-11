// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Transform.BlockTransformer.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"
#include "ULIS/Transform/ULIS.Transform.ResamplingMethods.h"
#include "ULIS/Transform/ULIS.Transform.BlockTransformerNearestNeighbour.h"
#include "ULIS/Transform/ULIS.Transform.BlockTransformerLinear.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

/////////////////////////////////////////////////////
// TBlockTransformer_Default
template< uint32 _SH >
class TBlockTransformer_Default
{
public:
    static void Run( const TBlock< _SH >*        iSrcBlock
                   , TBlock< _SH >*              iDstBlock
                   , const glm::mat3&            iInverseTransform
                   , eResamplingMethod           iResamplingMethod = eResamplingMethod::kBilinear
                   , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        switch( iResamplingMethod )
        {
            case eResamplingMethod::kNearestNeighbour:
            {
                TBlockTransformer_NearestNeighbour< _SH >::Run( iSrcBlock, iDstBlock, iInverseTransform, iPerformanceOptions );
                break;
            }

            case eResamplingMethod::kBilinear:
            {
                TBlockTransformer_Linear< _SH >::Run( iSrcBlock, iDstBlock, iInverseTransform, iPerformanceOptions );
                break;
            }

            default: ULIS_CRASH_DELIBERATE;
        }
    }
};


/////////////////////////////////////////////////////
// TBlockTransformer_Imp
template< uint32    _SH
        , e_tp      _TP
        , e_cm      _CM
        , e_ea      _EA >
class TBlockTransformer_Imp
{
public:
    static inline void Run( const TBlock< _SH >*        iSrcBlock
                          , TBlock< _SH >*              iDstBlock
                          , const glm::mat3&            iInverseTransform
                          , eResamplingMethod           iResamplingMethod = eResamplingMethod::kBilinear
                          , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockTransformer_Default< _SH >::Run( iSrcBlock
                                             , iDstBlock
                                             , iInverseTransform
                                             , iResamplingMethod
                                             , iPerformanceOptions );
    }
};


/////////////////////////////////////////////////////
// TBlockTransformer
template< uint32 _SH >
class TBlockTransformer
{
public:
    static inline void Run( const TBlock< _SH >*        iSrcBlock
                          , TBlock< _SH >*              iDstBlock
                          , const glm::mat3&            iInverseTransform
                          , eResamplingMethod           iResamplingMethod = eResamplingMethod::kBilinear
                          , const FPerformanceOptions&  iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockTransformer_Imp< _SH
                             , tSpec::_nf._tp
                             , tSpec::_nf._cm
                             , tSpec::_nf._ea >
                            ::Run( iSrcBlock
                                 , iDstBlock
                                 , iInverseTransform
                                 , iResamplingMethod
                                 , iPerformanceOptions );
    }
};

/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS

