// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Spec.PreprocessorFramework.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/apply.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>


namespace ULIS {

/////////////////////////////////////////////////////
// Enum and Keyword
/* Make a tuple from variadic args */
#define ULIS_KEYS( ... ) ( __VA_ARGS__ )

/* concatenate a token with 'kw' prefix to build a keyword category name */
#define ULIS_KEYWORDS_CAT( cat ) kw##cat

/* concatenate a token with 'e' prefix to build an enum category name */
#define ULIS_ENUM_CAT( cat ) e##cat

/* delegate macro for sequential 'stringization' of token */
#define ULIS_MAKE_KEYWORD_SEQ( r, data, elem ) BOOST_PP_STRINGIZE( elem ),

/* delegate macro for sequential 'enumerization' of token */
#define ULIS_MAKE_ENUM_SEQ( r, data, elem ) BOOST_PP_CAT( k, elem ),

/* build a keyword categroy as an array of strings from tokens */
#define ULIS_KEYS_TO_KEYWORDS( cat, keys )                                              \
    constexpr const char* ULIS_KEYWORDS_CAT( cat )[] = {                                \
        BOOST_PP_SEQ_FOR_EACH( ULIS_MAKE_KEYWORD_SEQ, 0, BOOST_PP_TUPLE_TO_SEQ( keys ) ) };

/* build an enum categroy as a cpp enum of integer values from tokens */
#define ULIS_KEYS_TO_ENUM( cat, keys )                                                  \
    enum class ULIS_ENUM_CAT( cat ) : int {                                             \
        BOOST_PP_SEQ_FOR_EACH( ULIS_MAKE_ENUM_SEQ, 0, BOOST_PP_TUPLE_TO_SEQ( keys ) ) };

/* Make Tuple and build keys */
#define ULIS_MAKE_KEYS_ENUM_AND_KEYWORDS( cat, ... ) ULIS_KEYS_TO_KEYWORDS( cat, __VA_ARGS__ ) ULIS_KEYS_TO_ENUM( cat, __VA_ARGS__ )

/* concatenate multiple keyword arguments with no prefered order */
#define ULIS_BLOCK_SPEC( ... )      BOOST_PP_SEQ_CAT( BOOST_PP_TUPLE_TO_SEQ( ( __VA_ARGS__ ) ) )
#define ULIS_SPEC_TO_STR( spec )    coalMakeFromString( BOOST_PP_STRINGIZE( spec ) )
#define ULIS_SPEC_SS( spec )        BOOST_PP_CAT( _SpecStr_, spec )
#define ULIS_SPEC_SH( spec )        BOOST_PP_CAT( Format_, spec )

/* PREDECL for inline fix on CLANG */
#if defined(__clang__)
#define ULIS_PREDECL_NF inline
#elif defined(__GNUC__) || defined(__GNUG__)
#define ULIS_PREDECL_NF
#elif defined(_MSC_VER)
#define ULIS_PREDECL_NF
#endif


} // namespace ULIS
