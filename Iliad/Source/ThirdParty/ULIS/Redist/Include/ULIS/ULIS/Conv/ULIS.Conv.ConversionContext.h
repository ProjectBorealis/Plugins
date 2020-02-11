// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Conv.ConversionContext.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Conv/ULIS.Conv.BlockTypeConverter.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Make/ULIS.Make.MakeContext.h"
#include "lcms2.h"

namespace ULIS {
/////////////////////////////////////////////////////
// TConversionContext
class TConversionContext
{
public:
    template< uint32 _SHSrc, uint32 _SHDst >
    static void ConvertTypeAndLayoutInto( const TBlock< _SHSrc >* iBlockSrc, TBlock< _SHDst >* iBlockDst, const FPerformanceOptions& iPerformanceOptions= FPerformanceOptions() )
    {
        // For:
        // Any Types
        // Any Layout
        // Profile Agnostic
        // Same Model
        assert( iBlockSrc );
        assert( iBlockDst );
        assert( iBlockSrc->Width() == iBlockDst->Width() );
        assert( iBlockSrc->Height() == iBlockDst->Height() );
        using src_info = TBlockInfo< _SHSrc >;
        using dst_info = TBlockInfo< _SHDst >;
        assert( src_info::_nf._cm == dst_info::_nf._cm ); // Color Model
        if( _SHSrc == _SHDst )
            TMakeContext< _SHSrc >::CopyBlockInto( (const TBlock< _SHSrc >*)iBlockSrc, (TBlock< _SHSrc >*)iBlockDst, iPerformanceOptions);
        else
            TBlockTypeConverter< _SHSrc, _SHDst, ( (int)src_info::_nf._cm - (int)dst_info::_nf._cm ) >::Run( iBlockSrc, iBlockDst, iPerformanceOptions);
    }

    template< uint32 _SHSrc, uint32 _SHDst >
    static void ConvertTypeAndLayoutInto( const TPixelBase< _SHSrc >& iSrc, TPixelBase< _SHDst >& iDst )
    {
        using src_info = TBlockInfo< _SHSrc >;
        using dst_info = TBlockInfo< _SHDst >;
        assert( src_info::_nf._cm == dst_info::_nf._cm ); // Color Model

        if( _SHSrc == _SHDst )
        {
            memcpy( iDst.Ptr(), iSrc.Ptr(), src_info::_nf._pd );
            iDst.AssignColorProfile( iSrc.ColorProfile() );
        }
        else
        {
            TPixelTypeConverter< _SHSrc, _SHDst, ( (int)src_info::_nf._cm - (int)dst_info::_nf._cm ) >::Apply( iSrc, iDst );
        }
    }

    struct FConversionDiagnosis
    {
        bool bSameFormat;
        bool bSameType;
        bool bSameModel;
        bool bSameLayout;
        bool bSameProfile;
    };

    /////////////////////////////////////////////////////
    // TForwardConnector
    template< uint32 _SH, e_cm _CM >
    struct TForwardConnector
    {
        static TPixelValue< TModelConnectionFormat< _CM >() > ConnectionModelFormat( const TPixelBase< _SH >& iValue )
        {
            TPixelValue< TModelConnectionFormat< _CM >() > ret;
            ConvertTypeAndLayoutInto< _SH, TModelConnectionFormat< _CM >() >( iValue, ret );
            return  ret;
        }
    };

    /////////////////////////////////////////////////////
    // TForwardConnector Specialization
    template< uint32 _SH >
    struct TForwardConnector< _SH, e_cm::kHSL >
    {
        static TPixelValue< TModelConnectionFormat< e_cm::kHSL >() > ConnectionModelFormat( const TPixelAcessor< _SH, TBlockInfo< _SH >::_nf._cm >& iValue )
        {
            TPixelValue< TModelConnectionFormat< e_cm::kHSL >() > ret;
            ret.SetColor( iValue.GetColor() );
            return  ret;
        }
    };


    template< uint32 _SH >
    struct TForwardConnector< _SH, e_cm::kHSV >
    {
        static TPixelValue< TModelConnectionFormat< e_cm::kHSV >() > ConnectionModelFormat( const TPixelAcessor< _SH, TBlockInfo< _SH >::_nf._cm >& iValue )
        {
            TPixelValue< TModelConnectionFormat< e_cm::kHSV >() > ret;
            ret.SetColor( iValue.GetColor() );
            return  ret;
        }
    };

    /////////////////////////////////////////////////////
    // TBackwardConnector
    template< uint32 _SHR, uint32 _SHCO, e_cm _CM >
    struct TDropConnector
    {
        static void Apply( TPixelBase< _SHR >& iDst, const TPixelBase< _SHCO >& iConnector )
        {
            ConvertTypeAndLayoutInto< _SHCO, _SHR >( iConnector, iDst );
        }
    };

    /////////////////////////////////////////////////////
    // TForwardConnector Specialization
    template< uint32 _SHR, uint32 _SHCO >
    struct TDropConnector< _SHR, _SHCO, e_cm::kHSL >
    {
        static void Apply( TPixelAcessor< _SHR, TBlockInfo< _SHR >::_nf._cm >& iDst, const TPixelAcessor< _SHCO, TBlockInfo< _SHCO >::_nf._cm >& iConnector )
        {
            iDst.SetColor( iConnector.GetColor() );
        }
    };


    template< uint32 _SHR, uint32 _SHCO >
    struct TDropConnector< _SHR, _SHCO, e_cm::kHSV >
    {
        static void Apply( TPixelAcessor< _SHR, TBlockInfo< _SHR >::_nf._cm >& iDst, const TPixelAcessor< _SHCO, TBlockInfo< _SHCO >::_nf._cm >& iConnector )
        {
            iDst.SetColor( iConnector.GetColor() );
        }
    };

    /////////////////////////////////////////////////////
    // General Convert
    template< uint32 _SHSrc, uint32 _SHDst >
    static void Convert( const TPixelAcessor< _SHSrc, TBlockInfo< _SHSrc >::_nf._cm >& iSrc, TPixelAcessor< _SHDst, TBlockInfo< _SHDst >::_nf._cm >& iDst )
    {
        using src_info = TBlockInfo< _SHSrc >;
        using dst_info = TBlockInfo< _SHDst >;

        FConversionDiagnosis diag;
        diag.bSameFormat    = src_info::_nf._sh == dst_info::_nf._sh;
        diag.bSameType      = src_info::_nf._tp == dst_info::_nf._tp;
        diag.bSameModel     = src_info::_nf._cm == dst_info::_nf._cm;
        diag.bSameLayout    = src_info::_nf._lh == dst_info::_nf._lh;
        diag.bSameProfile   = iSrc.ColorProfile() == iDst.ColorProfile();

        if( diag.bSameFormat && diag.bSameProfile )
        {
            memcpy( iDst.Ptr(), iSrc.Ptr(), src_info::_nf._pd );
            iDst.AssignColorProfile( iSrc.ColorProfile() );
            return;
        }

        using tSrcConnectionType = TPixelValue< TModelConnectionFormat< src_info::_nf._cm >() >;
        using tDstConnectionType = TPixelValue< TModelConnectionFormat< dst_info::_nf._cm >() >;
        tSrcConnectionType srcConnectionValue = TForwardConnector< _SHSrc, src_info::_nf._cm >::ConnectionModelFormat( iSrc );
        tDstConnectionType dstConnectionValue = TForwardConnector< _SHDst, dst_info::_nf._cm >::ConnectionModelFormat( iDst );

        FColorProfile* src_profile = iSrc.ColorProfile();
        FColorProfile* dst_profile = iDst.ColorProfile();
        if( src_info::_nf._cm == dst_info::_nf._cm )
        {
            if( src_profile == nullptr ) src_profile = dst_profile;
            if( dst_profile == nullptr ) dst_profile = src_profile;
        }
        else
        {
            if( src_profile == nullptr ) src_profile = FGlobalProfileRegistry::Get().GetDefaultProfileForModel( src_info::_nf._cm );
            if( dst_profile == nullptr ) dst_profile = FGlobalProfileRegistry::Get().GetDefaultProfileForModel( dst_info::_nf._cm );
        }

        cmsHPROFILE hInProfile  = src_profile->ProfileHandle();
        cmsHPROFILE hOutProfile = dst_profile->ProfileHandle();
        cmsHTRANSFORM hTransform;
        hTransform = cmsCreateTransform( hInProfile
                                       , TCMSConnectionType< tSrcConnectionType::ColorModel() >()
                                       , hOutProfile
                                       , TCMSConnectionType< tDstConnectionType::ColorModel() >()
                                       , INTENT_PERCEPTUAL, 0 );


        cmsDoTransform( hTransform, srcConnectionValue.Ptr(), dstConnectionValue.Ptr(), 1 );
        cmsDeleteTransform( hTransform );

        TDropConnector< _SHDst, TModelConnectionFormat< dst_info::_nf._cm >(), dst_info::_nf._cm >::Apply( iDst, dstConnectionValue );
    }

    template< uint32 _SHSrc, uint32 _SHDst >
    struct TReusableConverter {
        using src_info = TBlockInfo< _SHSrc >;
        using dst_info = TBlockInfo< _SHDst >;
        using tSrcConnectionType = TPixelValue< TModelConnectionFormat< src_info::_nf._cm >() >;
        using tDstConnectionType = TPixelValue< TModelConnectionFormat< dst_info::_nf._cm >() >;
        tSrcConnectionType srcConnectionValue;
        tDstConnectionType dstConnectionValue;
        FConversionDiagnosis diag;
        cmsHPROFILE     hInProfile;
        cmsHPROFILE     hOutProfile;
        cmsHTRANSFORM   hTransform;
        void Build( FColorProfile* iSrcProfile, FColorProfile* iDstProfile ) {
            diag.bSameFormat    = src_info::_nf._sh == dst_info::_nf._sh;
            diag.bSameType      = src_info::_nf._tp == dst_info::_nf._tp;
            diag.bSameModel     = src_info::_nf._cm == dst_info::_nf._cm;
            diag.bSameLayout    = src_info::_nf._lh == dst_info::_nf._lh;
            diag.bSameProfile   = iSrcProfile == iDstProfile;
            if( src_info::_nf._cm == dst_info::_nf._cm )
            {
                if( iSrcProfile == nullptr ) iSrcProfile = iDstProfile;
                if( iDstProfile == nullptr ) iDstProfile = iSrcProfile;
            }
            else
            {
                if( iSrcProfile == nullptr ) iSrcProfile = FGlobalProfileRegistry::Get().GetDefaultProfileForModel( src_info::_nf._cm );
                if( iDstProfile == nullptr ) iDstProfile = FGlobalProfileRegistry::Get().GetDefaultProfileForModel( dst_info::_nf._cm );
            }
            hInProfile  = iSrcProfile->ProfileHandle();
            hOutProfile = iDstProfile->ProfileHandle();
            hTransform = cmsCreateTransform( hInProfile
                                           , TCMSConnectionType< tSrcConnectionType::ColorModel() >()
                                           , hOutProfile
                                           , TCMSConnectionType< tDstConnectionType::ColorModel() >()
                                           , INTENT_PERCEPTUAL, 0 );
        }

        void Destroy() {
            cmsDeleteTransform( hTransform );
        }

        void Convert( const TPixelAcessor< _SHSrc, TBlockInfo< _SHSrc >::_nf._cm >& iSrc, TPixelAcessor< _SHDst, TBlockInfo< _SHDst >::_nf._cm >& iDst ) {
            srcConnectionValue.TPixelBase< TModelConnectionFormat< src_info::_nf._cm >() >::operator=( TForwardConnector< _SHSrc, src_info::_nf._cm >::ConnectionModelFormat( iSrc ) );
            dstConnectionValue.TPixelBase< TModelConnectionFormat< dst_info::_nf._cm >() >::operator=( TForwardConnector< _SHDst, dst_info::_nf._cm >::ConnectionModelFormat( iDst ) );
            cmsDoTransform( hTransform, srcConnectionValue.Ptr(), dstConnectionValue.Ptr(), 1 );
            TDropConnector< _SHDst, TModelConnectionFormat< dst_info::_nf._cm >(), dst_info::_nf._cm >::Apply( iDst, dstConnectionValue );
        }
    };
};


} // namespace ULIS

