// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Data.Layout.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <utility>
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Spec/ULIS.Spec.Spec.h"

namespace ULIS {


/////////////////////////////////////////////////////
// Defines
#define tSpec       TBlockInfo< _SH >
#define tLayout     TPixelLayout< _SH >

/////////////////////////////////////////////////////
// TRedirector
/* A simple array for storing the redirection index */
template< int N > struct TRedirector { const uint8_t arr[ N ]; };

/////////////////////////////////////////////////////
// Layout Redirection Parsing Utility
constexpr uint8 ParseLayoutRedirector_Imp_GetIndex( const char* iModel, const char* iLayout, bool iAlpha, int i )
{
    /* Layout ordered : */    //return  i < ::__coal__::strlen( iModel ) ? ::__coal__::indexof( iModel[i], iLayout, 0 ) : ::__coal__::indexof( 'A', iLayout, 0 );
    /* Alpha first  :   */    //return  iAlpha ? i == 0 ? ::__coal__::indexof( 'A', iLayout, 0 ) : ::__coal__::indexof( iModel[i-1], iLayout, 0 ) : ::__coal__::indexof( iModel[i-1], iLayout, 0 );
    /* Alpha last   :   */      return  iAlpha ? i == ::__coal__::strlen( iModel ) ? ::__coal__::indexof( 'A', iLayout, 0 ) : ::__coal__::indexof( iModel[i], iLayout, 0 ) : ::__coal__::indexof( iModel[i], iLayout, 0 );
}


template< int N, typename T, T... Nums >
constexpr TRedirector< N > ParseLayoutRedirector_Imp( const char* iModel, const char* iLayout, bool iAlpha, std::integer_sequence< T, Nums... > )
{
    return { ParseLayoutRedirector_Imp_GetIndex( iModel, iLayout, iAlpha, Nums ) ... };
}


template< int N >
constexpr TRedirector< N > ParseLayoutRedirector( const char* iModel, const char* iLayout, bool iAlpha )
{
    return  ParseLayoutRedirector_Imp< N >( iModel, iLayout, iAlpha, std::make_integer_sequence< int, N >() );
}

/////////////////////////////////////////////////////
// TLayout
/* Pixel Layout obtained from hash, holding the Redirector */
template< uint32_t _SH >
struct TPixelLayout
{
    static constexpr TRedirector< tSpec::_nf._rc > red = ParseLayoutRedirector< tSpec::_nf._rc >( kw_cm[ (int)tSpec::_nf._cm ], tSpec::_nf._cl, tSpec::_nf._ea == e_ea::khasAlpha );
};

/* Post definition of template redirector 'red' member to ensure it has a location in memory for external use */
template< uint32_t _SH > constexpr TRedirector< tSpec::_nf._rc > TPixelLayout< _SH >::red;

/////////////////////////////////////////////////////
// Undefines
#undef tSpec
#undef tLayout

} // namespace ULIS

