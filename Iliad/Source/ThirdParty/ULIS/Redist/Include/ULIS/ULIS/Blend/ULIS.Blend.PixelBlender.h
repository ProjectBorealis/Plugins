// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Blend.PixelBlender.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <assert.h>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Blend/ULIS.Blend.BlendingModes.h"
#include "ULIS/Data/ULIS.Data.Block.h"
#include "ULIS/Blend/ULIS.Blend.BasicCompositing.h"
#include "ULIS/Conv/ULIS.Conv.ConversionContext.h"

namespace ULIS {
/////////////////////////////////////////////////////
// Defines
#define tSpec TBlockInfo< _SH >

/////////////////////////////////////////////////////
// TPixelBlender
// Default Implementation of Pixel Blender for Standard Separable Blending Modes.
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM
        , bool          _NS >
struct TPixelBlender
{
    TPixelBlender( TBlock< _SH >*                       iBlockTop
                 , TBlock< _SH >*                       iBlockBack
                 , typename TBlock< _SH >::tPixelType   iOpacity
                 , const FPoint&                        iShift )
        : top       ( iBlockTop     )
        , back      ( iBlockBack    )
        , opacity   ( iOpacity      )
        , shift     ( iShift        )
    {}

    void Apply( int x, int y )
    {
        typename TBlock< _SH >::tPixelProxy         pixelBack   = back->PixelProxy( x, y );
        typename TBlock< _SH >::tPixelProxy         pixelTop    = top->PixelProxy( x + shift.x, y + shift.y );
        const typename TBlock< _SH >::tPixelType    alphaBack   = pixelBack.GetAlpha();
        const typename TBlock< _SH >::tPixelType    alphaTop    = ConvType< typename TBlock< _SH >::tNextPixelType, typename TBlock< _SH >::tPixelType >( (typename TBlock< _SH >::tNextPixelType)( pixelTop.GetAlpha() * opacity ) );
        const typename TBlock< _SH >::tPixelType    alphaComp = BlendAlpha< _SH, eAlphaMode::kNormal >::Compute( alphaBack, alphaTop );
        const typename TBlock< _SH >::tPixelType    alphaResult = BlendAlpha< _SH, _AM >::Compute( alphaBack, alphaTop );
        const typename TBlock< _SH >::tPixelType    var         = alphaComp == 0 ? 0 : ( alphaTop * TBlock< _SH >::StaticMax() ) / alphaComp;
        const int max_chan = TBlock< _SH >::StaticNumColorChannels();
        for( int i = 0; i < max_chan; ++i )
            pixelBack.SetComponent( i, Composer< _SH, _BM >::BasicCompositing( pixelBack.GetComponent( i ), pixelTop.GetComponent( i ), alphaBack, var ) );
        pixelBack.SetAlpha( alphaResult );
    }

    TBlock< _SH >* top;
    TBlock< _SH >* back;
    typename TBlock< _SH >::tPixelType opacity;
    const FPoint& shift;
};

/////////////////////////////////////////////////////
// TPixelBlender
// Implementation of Pixel Blender for Non-Separable Blending Modes.
template< uint32        _SH
        , eBlendingMode _BM
        , eAlphaMode    _AM >
struct TPixelBlender< _SH
                    , _BM
                    , _AM
                    , true >
{
    TPixelBlender( TBlock< _SH >*                       iBlockTop
                 , TBlock< _SH >*                       iBlockBack
                 , typename TBlock< _SH >::tPixelType   iOpacity
                 , const FPoint&                        iShift )
        : top       ( iBlockTop     )
        , back      ( iBlockBack    )
        , opacity   ( iOpacity      )
        , shift     ( iShift        )
    {
        forwardConverter.Build( iBlockTop->ColorProfile(), nullptr );
        backwardConverter.Build( nullptr, iBlockTop->ColorProfile() );
    }

    ~TPixelBlender()
    {
        forwardConverter.Destroy();
        backwardConverter.Destroy();
    }

    void Apply( int x, int y )
    {
        typename TBlock< _SH >::tPixelProxy         pixelBack   = back->PixelProxy( x, y );
        typename TBlock< _SH >::tPixelProxy         pixelTop    = top->PixelProxy( x + shift.x, y + shift.y );
        typename TBlock< _SH >::tPixelValue         componentsResult;
        BlendFuncNS< _SH, _BM >::Compute( pixelBack, pixelTop, componentsResult, &forwardConverter, &backwardConverter );

        const typename TBlock< _SH >::tPixelType    alphaBack   = pixelBack.GetAlpha();
        const typename TBlock< _SH >::tPixelType    alphaTop    = ConvType< typename TBlock< _SH >::tNextPixelType, typename TBlock< _SH >::tPixelType >( (typename TBlock< _SH >::tNextPixelType)( pixelTop.GetAlpha() * opacity ) );
        const typename TBlock< _SH >::tPixelType    alphaComp = BlendAlpha< _SH, eAlphaMode::kNormal >::Compute( alphaBack, alphaTop );
        const typename TBlock< _SH >::tPixelType    alphaResult = BlendAlpha< _SH, _AM >::Compute( alphaBack, alphaTop );
        const typename TBlock< _SH >::tPixelType    var         = alphaComp == 0 ? 0 : ( alphaTop * TBlock< _SH >::StaticMax() ) / alphaComp;
        const int max_chan = TBlock< _SH >::StaticNumColorChannels();
        for( int i = 0; i < max_chan; ++i )
            pixelBack.SetComponent( i, Composer< _SH, _BM >::NonSeparableCompositing( pixelBack.GetComponent( i ), pixelTop.GetComponent( i ), componentsResult.GetComponent( i ), alphaBack, var ) );
        pixelBack.SetAlpha( alphaResult );
    }

    TBlock< _SH >* top;
    TBlock< _SH >* back;
    typename TBlock< _SH >::tPixelType opacity;
    const FPoint& shift;
    TConversionContext::TReusableConverter< _SH, TDefaultModelFormat< e_cm::kHSL >() > forwardConverter;
    TConversionContext::TReusableConverter< TDefaultModelFormat< e_cm::kHSL >(), _SH > backwardConverter;
};

/////////////////////////////////////////////////////
// TPixelBlender
// Implementation of Pixel Blender for Dissolve
template< uint32        _SH
        , eAlphaMode    _AM >
struct TPixelBlender< _SH
                    , eBlendingMode::kDissolve
                    , _AM
                    , true >
{
    TPixelBlender( TBlock< _SH >*                       iBlockTop
                 , TBlock< _SH >*                       iBlockBack
                 , typename TBlock< _SH >::tPixelType   iOpacity
                 , const FPoint&                        iShift )
        : top       ( iBlockTop     )
        , back      ( iBlockBack    )
        , opacity   ( iOpacity      )
        , shift     ( iShift        )
    {}

    void Apply( int x, int y )
    {
        typename TBlock< _SH >::tPixelProxy         pixelBack   = back->PixelProxy( x, y );
        typename TBlock< _SH >::tPixelProxy         pixelTop    = top->PixelProxy( x + shift.x, y + shift.y );

        float threshold = ConvType< typename TBlock< _SH >::tNextPixelType, float >( (typename TBlock< _SH >::tNextPixelType)( pixelTop.GetAlpha() * opacity ) );
        float toss = 0.f;
        {
            float sum = x * y + ( x + y );
            toss = float( ( int( 8253729 * ( ( sin( sum ) + cos( sum ) + 2.0f ) / 4.0f ) ) + 2396403 ) % 65537 ) / 65537.f;
        }

        if( toss < threshold )
        {
            const typename TBlock< _SH >::tPixelType    alphaBack   = pixelBack.GetAlpha();
            const typename TBlock< _SH >::tPixelType alphaResult = BlendAlpha< _SH, _AM >::Compute( alphaBack, TBlock< _SH >::StaticMax() );
            pixelBack.TPixelBase< _SH >::operator=( pixelTop );
            pixelBack.SetAlpha( alphaResult );
        }
    }

    TBlock< _SH >* top;
    TBlock< _SH >* back;
    typename TBlock< _SH >::tPixelType opacity;
    const FPoint& shift;
    TConversionContext::TReusableConverter< _SH, TDefaultModelFormat< e_cm::kHSL >() > forwardConverter;
    TConversionContext::TReusableConverter< TDefaultModelFormat< e_cm::kHSL >(), _SH > backwardConverter;
};


/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS

