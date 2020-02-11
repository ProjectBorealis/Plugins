// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Spec.Spec.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <coal_core>
#include <limits>
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Spec/ULIS.Spec.PreprocessorFramework.h"
#include "lcms2.h"

#ifdef ULIS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
#endif

namespace ULIS {
/////////////////////////////////////////////////////
// Enum and Keyword Declaration
/* We declare keys for type decl keywords */
#define ULIS_KEYS_TP ULIS_KEYS( uint8, uint16, uint32, float, double )
#define ULIS_KEYS_CM ULIS_KEYS( G, RGB, HSL, HSV, CMYK, Lab, XYZ )
#define ULIS_KEYS_EA ULIS_KEYS( hasAlpha, noAlpha )
#define ULIS_KEYS_NM ULIS_KEYS( normalized, typeLimits )
#define ULIS_SEQ_TP  BOOST_PP_TUPLE_TO_SEQ( ULIS_KEYS_TP )
#define ULIS_SEQ_CM  BOOST_PP_TUPLE_TO_SEQ( ULIS_KEYS_CM )
#define ULIS_SEQ_EA  BOOST_PP_TUPLE_TO_SEQ( ULIS_KEYS_EA )
#define ULIS_SEQ_NM  BOOST_PP_TUPLE_TO_SEQ( ULIS_KEYS_NM )
/* We make enum and cstring keywords out of theme */
// Enums:       enum class          e_xx : int { kXXX,  ... };
// Keywords:    constexpr char*     kw_xx[] =  { "XXX", ... };
ULIS_MAKE_KEYS_ENUM_AND_KEYWORDS( _tp, ULIS_KEYS_TP ) // type
ULIS_MAKE_KEYS_ENUM_AND_KEYWORDS( _cm, ULIS_KEYS_CM ) // color_model
ULIS_MAKE_KEYS_ENUM_AND_KEYWORDS( _ea, ULIS_KEYS_EA ) // extra_alpha
ULIS_MAKE_KEYS_ENUM_AND_KEYWORDS( _nm, ULIS_KEYS_NM ) // normal_mode

#define ULIS_CHANNEL_INDEX_RGBA_RED     0
#define ULIS_CHANNEL_INDEX_RGBA_GREEN   1
#define ULIS_CHANNEL_INDEX_RGBA_BLUE    2
#define ULIS_CHANNEL_INDEX_RGBA_ALPHA   3
#define ULIS_CHANNEL_BIT_RGBA_RED       1
#define ULIS_CHANNEL_BIT_RGBA_GREEN     2
#define ULIS_CHANNEL_BIT_RGBA_BLUE      4
#define ULIS_CHANNEL_BIT_RGBA_ALPHA     8

/////////////////////////////////////////////////////
// FSpec
/* Non-template Spec structure */
struct FSpec
{
    // Core properties
    const char*         _ss; // spec_str
    const uint32_t      _sh; // spec_hash
    const  e_tp         _tp; // type
    const  e_cm         _cm; // color_model
    const  e_ea         _ea; // extra_alpha
    const  char*        _cl; // channel_layout
    const  e_nm         _nm; // normal_mode

    // Infered properties
    const  bool         _dm; // decimal
    const  uint8_t      _rc; // real_channels
    const  uint8_t      _nc; // num_channels
    const  uint16_t     _pd; // pixel_depth
    const  double       _tm; // type_max
    const  double       _rm; // range_max
    const  uint8_t      _ma; // memory_alignment
    const  uint32_t     _lh; // layout_hash
};

/////////////////////////////////////////////////////
// TBlockInfo
/* Template specialized Block Spec structure */
template< uint32_t > struct TBlockInfo {
    ULIS_PREDECL_NF static constexpr FSpec _nf = { "Invalid"            // spec-str
                                                   , 0                  // spec-hash
                                                   , e_tp::kuint8       // type
                                                   , e_cm::kRGB         // color_model
                                                   , e_ea::knoAlpha     // extra_alpha
                                                   , "None"             // channel_layout
                                                   , e_nm::ktypeLimits  // normal_mode
                                                   , false              // decimal
                                                   , 0                  // real_channels
                                                   , 0                  // num_channels
                                                   , 0                  // pixel_depth
                                                   , 0                  // type_max
                                                   , 0                  // range_max
                                                   , 16                 // memory_alignment
                                                   , 0                  // layout_hash
    };
};

/////////////////////////////////////////////////////
// Info
/* We check that a type is decimal */
constexpr  inline bool is_decimal( ::ULIS::e_tp iTp ) { return  iTp == ::ULIS::e_tp::kdouble || iTp == ::ULIS::e_tp::kfloat ? true : false; }

/* Getters for all types size & max value */
template< e_tp iTp > constexpr  uint16_t    type_size() { return  0; }
template< e_tp iTp > constexpr  double      type_max()  { return  0; }
/* Repeater macros for type getters */
#define ULIS_TYPE_SIZE_REPEAT( r, data, elem )  template<> inline constexpr  uint16_t  type_size<  BOOST_PP_CAT( e_tp::k, elem ) >() { return  sizeof( elem ); }
#define ULIS_TYPE_MAX_REPEAT( r, data, elem )   template<> inline constexpr  double    type_max<   BOOST_PP_CAT( e_tp::k, elem ) >() { return  std::numeric_limits< elem >::max(); }
/* specialize type size & max for all types */
BOOST_PP_SEQ_FOR_EACH( ULIS_TYPE_SIZE_REPEAT, void, ULIS_SEQ_TP )
BOOST_PP_SEQ_FOR_EACH( ULIS_TYPE_MAX_REPEAT,  void, ULIS_SEQ_TP )
#undef ULIS_TYPE_SIZE_REPEAT
#undef ULIS_TYPE_MAX_REPEAT

/////////////////////////////////////////////////////
// Parsing
/* Parse an FSpec from the macro input */
template< ::ULIS::e_tp iTp, ::ULIS::e_cm iCm, ::ULIS::e_ea iEa, ::ULIS::e_nm iNm >
constexpr  FSpec parseSpec( const char* iSs, uint32_t iSh, const char* iCl )
{
    constexpr bool      dm = is_decimal( iTp );
    uint8_t             rc = ::__coal__::strlen( iCl );
    uint8_t             nc = iEa == ::ULIS::e_ea::khasAlpha ? rc - 1 : rc;
    uint16_t            pd = type_size< iTp >() * rc;
    double              tm = iNm == e_nm::knormalized ? 1.0 : type_max< iTp >();
    double              rm = dm ? tm : tm + 1;
    uint8_t             ma = 16;
    uint32_t            lh = COAL_CRC32_STR( iCl );
    static_assert( iNm == e_nm::knormalized ? dm : true, "Integer types cannot be normalized" );
    return  { iSs, iSh, iTp, iCm, iEa, iCl, iNm, dm, rc, nc, pd, tm, rm, ma, lh };
}

/* small wrapper for readability in other macros */
#define ULIS_PARSE_FSPEC( ss, sh, tp, cm, ea, cl, nm )              \
    ::ULIS::parseSpec< BOOST_PP_CAT( ::ULIS::e_tp::k, tp )          \
                     , BOOST_PP_CAT( ::ULIS::e_cm::k, cm )          \
                     , BOOST_PP_CAT( ::ULIS::e_ea::k, ea )          \
                     , BOOST_PP_CAT( ::ULIS::e_nm::k, nm ) >        \
                     ( ss, sh, BOOST_PP_STRINGIZE( cl ) );

/////////////////////////////////////////////////////
// Post decl _nf template struct member of TBlockInfo specializations, branching workaround for GCC / CLANG / MSVC
#if defined(__clang__)
#define ULIS_POSTDECL_NF( spec )    constexpr FSpec TBlockInfo< ULIS_SPEC_SH( spec ) >::_nf;
#elif defined(__GNUC__) || defined(__GNUG__)
#define ULIS_POSTDECL_NF( spec )
#elif defined(_MSC_VER)
#define ULIS_POSTDECL_NF( spec )    constexpr FSpec TBlockInfo< ULIS_SPEC_SH( spec ) >::_nf;
#endif

/////////////////////////////////////////////////////
// TBlockInfo Specialization
/* build a specialization of TBlockInfo from token and compute string and hash equivalents */
#define ULIS_DECLSPEC_IMP( tp, cm, ea, cl, nm, spec )                                                   \
    namespace generated {                                                                               \
    coal                ULIS_SPEC_SS( spec ) = ULIS_SPEC_TO_STR( spec );                                \
    } /* namespace generated */                                                                         \
    namespace Format {                                                                                  \
    constexpr uint32_t  ULIS_SPEC_SH( spec ) = ::ULIS::generated:: ULIS_SPEC_SS( spec ).hash();         \
    } /* namespace Format */                                                                            \
    template<> struct TBlockInfo< Format:: ULIS_SPEC_SH( spec ) > {                                     \
        ULIS_PREDECL_NF static constexpr FSpec _nf = ULIS_PARSE_FSPEC( generated:: ULIS_SPEC_SS( spec ) \
                                                                     , Format:: ULIS_SPEC_SH( spec )    \
                                                                     , tp, cm, ea, cl, nm )             \
    };                                                                                                  \
    namespace Format {                                                                                  \
    typedef TBlock< Format:: ULIS_SPEC_SH( spec ) > BOOST_PP_CAT( FBlock, spec );                       \
    } /* namespace eFormats */                                                                          \
    ULIS_APPEND_REG( ULIS_REG_BUILDER, Format:: ULIS_SPEC_SH( spec ) )

/* Public macro for block spec */
#define ULIS_DECLSPEC( tp, cm, ea, cl, nm ) ULIS_DECLSPEC_IMP( tp, cm, ea, cl, nm, ULIS_BLOCK_SPEC(   tp, cm, ea, cl, nm ) )
#define ULIS_GETSPEC( i ) ::ULIS::TBlockInfo< i >::_nf;

/////////////////////////////////////////////////////
// TDefaultModelFormat
template< e_cm _CM > constexpr uint32 TDefaultModelFormat() { return  0; }
template< e_cm _CM > constexpr uint32 TCMSConnectionType() { return  0; }
template< e_cm _CM > constexpr uint32 TModelConnectionFormat() { return  0; }

} // namespace ULIS

#ifdef ULIS_CLANG
#pragma GCC diagnostic pop
#endif

