// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.BlockBlender.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Blend/ULIS.Blend.BlendingModes.h"
#include "ULIS/Blend/ULIS.Blend.PixelBlender.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

/////////////////////////////////////////////////////
// TBlockBlender_Default_ScanLine
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM
        , bool          _NS
        , e_tp          _TP
        , e_cm          _CM
        , e_ea          _EA
        , uint32        _LH
        , e_nm          _NM
        , bool          _DM >
class TBlockBlender_Default_ScanLine
{
public:
    static void ProcessScanLine( TBlock< _SH >*                     iBlockTop
                               , TBlock< _SH >*                     iBlockBack
                               , typename TBlock< _SH >::tPixelType iOpacity
                               , const int                          iLine
                               , const int                          iX1
                               , const int                          iX2
                               , const FPoint&                      iShift )
    {
        TPixelBlender< _SH, _BM, _AM, _NS > pixel_blender( iBlockTop, iBlockBack, iOpacity, iShift );
        for( int x = iX1; x < iX2; ++x )
            pixel_blender.Apply( x, iLine );
    }

    static void Run( TBlock< _SH >*                     iBlockTop
                   , TBlock< _SH >*                     iBlockBack
                   , typename TBlock< _SH >::tPixelType iOpacity
                   , const FRect&                       iROI
                   , const FPoint&                      iShift
                   , const FPerformanceOptions&         iPerformanceOptions= FPerformanceOptions() )
    {
        const int x1 = iROI.x;
        const int y1 = iROI.y;
        const int x2 = x1 + iROI.w;
        const int y2 = y1 + iROI.h;
        FThreadPool& global_pool = FGlobalThreadPool::Get();
        for( int y = y1; y < y2; ++y )
            global_pool.ScheduleJob( ProcessScanLine, iBlockTop, iBlockBack, iOpacity, y, x1, x2, iShift );

        global_pool.WaitForCompletion();
    }
};


/////////////////////////////////////////////////////
// TBlockBlender_Default_MonoThread
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM
        , bool          _NS
        , e_tp          _TP
        , e_cm          _CM
        , e_ea          _EA
        , uint32        _LH
        , e_nm          _NM
        , bool          _DM >
class TBlockBlender_Default_MonoThread
{
public:
    static void Run( TBlock< _SH >*                     iBlockTop
                   , TBlock< _SH >*                     iBlockBack
                   , typename TBlock< _SH >::tPixelType iOpacity
                   , const FRect&                       iROI
                   , const FPoint&                      iShift
                   , const FPerformanceOptions&         iPerformanceOptions= FPerformanceOptions() )
    {
        const int x1 = iROI.x;
        const int y1 = iROI.y;
        const int x2 = x1 + iROI.w;
        const int y2 = y1 + iROI.h;
        TPixelBlender< _SH, _BM, _AM, _NS > pixel_blender( iBlockTop, iBlockBack, iOpacity, iShift );
        for( int y = y1; y < y2; ++y )
            for( int x = x1; x < x2; ++x )
                pixel_blender.Apply( x, y );
    }
};

/////////////////////////////////////////////////////
// TBlockBlender_Default
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM
        , bool          _NS
        , e_tp          _TP
        , e_cm          _CM
        , e_ea          _EA
        , uint32        _LH
        , e_nm          _NM
        , bool          _DM >
class TBlockBlender_Default
{
public:
    static void Run( TBlock< _SH >*                     iBlockTop
                   , TBlock< _SH >*                     iBlockBack
                   , typename TBlock< _SH >::tPixelType iOpacity
                   , const FRect&                       iROI
                   , const FPoint&                      iShift
                   , const FPerformanceOptions&         iPerformanceOptions= FPerformanceOptions() )
    {
        if( iPerformanceOptions.desired_workers > 1 )
        {
            TBlockBlender_Default_ScanLine< _SH
                                          , _BM
                                          , _AM
                                          , _NS
                                          , tSpec::_nf._tp
                                          , tSpec::_nf._cm
                                          , tSpec::_nf._ea
                                          , tSpec::_nf._lh
                                          , tSpec::_nf._nm
                                          , tSpec::_nf._dm >
                                          ::Run( iBlockTop
                                               , iBlockBack
                                               , iOpacity
                                               , iROI
                                               , iShift
                                               , iPerformanceOptions);
        }
        else
        {
            TBlockBlender_Default_MonoThread< _SH
                                            , _BM
                                            , _AM
                                            , _NS
                                            , tSpec::_nf._tp
                                            , tSpec::_nf._cm
                                            , tSpec::_nf._ea
                                            , tSpec::_nf._lh
                                            , tSpec::_nf._nm
                                            , tSpec::_nf._dm >
                                            ::Run( iBlockTop
                                                 , iBlockBack
                                                 , iOpacity
                                                 , iROI
                                                 , iShift
                                                 , iPerformanceOptions);
        }
    }
};


/////////////////////////////////////////////////////
// TBlockBlender_Imp
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM
        , bool          _NS
        , e_tp          _TP
        , e_cm          _CM
        , e_ea          _EA
        , uint32        _LH
        , e_nm          _NM
        , bool          _DM >
class TBlockBlender_Imp
{
public:
    static inline void Run( TBlock< _SH >*                      iBlockTop
                          , TBlock< _SH >*                      iBlockBack
                          , typename TBlock< _SH >::tPixelType  iOpacity
                          , const FRect&                        iROI
                          , const FPoint&                       iShift
                          , const FPerformanceOptions&          iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockBlender_Default< _SH
                             , _BM
                             , _AM
                             , _NS
                             , tSpec::_nf._tp
                             , tSpec::_nf._cm
                             , tSpec::_nf._ea
                             , tSpec::_nf._lh
                             , tSpec::_nf._nm
                             , tSpec::_nf._dm >
                             ::Run( iBlockTop
                                  , iBlockBack
                                  , iOpacity
                                  , iROI
                                  , iShift
                                  , iPerformanceOptions);
    }
};

/////////////////////////////////////////////////////
// TBlockBlender
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM >
class TBlockBlender
{
public:
    static inline void Run( TBlock< _SH >*                      iBlockTop
                          , TBlock< _SH >*                      iBlockBack
                          , typename TBlock< _SH >::tPixelType  iOpacity
                          , const FRect&                        iROI
                          , const FPoint&                       iShift
                          , const FPerformanceOptions&          iPerformanceOptions= FPerformanceOptions() )
    {
        TBlockBlender_Imp< _SH
                         , _BM
                         , _AM
                         , IsNonSeparableBlendingMode( _BM )
                         , tSpec::_nf._tp
                         , tSpec::_nf._cm
                         , tSpec::_nf._ea
                         , tSpec::_nf._lh
                         , tSpec::_nf._nm
                         , tSpec::_nf._dm >
                         ::Run( iBlockTop
                              , iBlockBack
                              , iOpacity
                              , iROI
                              , iShift
                              , iPerformanceOptions);
    }
};

/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS