/*************************************************************************
*
*   coal
*__________________
*
* coal.coal.h
* Clement Berthaud - Layl
* Please refer to LICENSE.md
*/

#pragma once

#include <utility>
#include <stdint.h>
#include "coal/coal.char.h"
#include "coal/coal.crc32.h"


namespace __coal__ {


/////////////////////////////////////////////////////
// coal Constexpr compliant string manipulation operations
template< int N >
struct  coal_t
{
    // Data
    /* primary container holding the cstr */
    const char s[N];

    // Basic
    /* utility functions */
    constexpr operator const char*() const { return s; }
    constexpr char  operator[]( int i )                         const { return s[i]; }
    constexpr bool  operator==( const coal_t& other )           const { return streq( s, other.s ); }
    constexpr const char*   data()                              const { return s; }
    constexpr int           size()                              const { return N; }
    constexpr int           length()                            const { return N; }
    constexpr int           count()                             const { return N; }
    constexpr int           toInt()                             const { return stoi( s ); }
    constexpr bool          isInt()                             const { return is_number( s ); }
    constexpr uint32_t      hash()                              const { return COAL_CRC32_STR( s ); }
    constexpr int           indexOf( const char* w, int f=0 )   const { return indexof( w, s, f ); }

    // Advanced
    /* split */
    constexpr int querySplitNum( const char* sep )  const {
        int count = 0;
        for( int ind = 0; ind >= 0; ind = indexOf( sep, ind + 1 ) )
            ++count;
        return  count;
    }

    constexpr int querySplitWordStart( const char* sep, int wordIndex )  const
    {
        int charIndex = 0;
        for( int count = 0; count <= wordIndex - 1; charIndex = indexOf( sep, charIndex + 1 ) )
            ++count;
        if( wordIndex == 0 )  return  0;
        return  charIndex + 1;
    }

    constexpr int querySplitWordSize( const char* sep, int wordIndex )  const
    {
        int charIndex = 0;
        for( int count = 0; count <= wordIndex - 1; charIndex = indexOf( sep, charIndex + 1 ) )
            ++count;
        int nextIndex = indexOf( sep, charIndex + 1 );
        if( wordIndex == 0 )    return  nextIndex;
        if( nextIndex == -1 )   return  N - charIndex - 2;
        return  nextIndex - charIndex - 1;
    }

    // Advanced Template
    /* substring */
    static constexpr char coal_impl_getchar_substring( const char* str, int start, int num ) { return str[num+start]; }
    template< int L, typename T, T... Nums > static constexpr const coal_t< L > make_coal_substring_impl(  const char* str, int start, std::integer_sequence< T, Nums... > ) { return { coal_impl_getchar_substring( str, start, Nums ) ..., '\0' }; }
    template< int L > static constexpr const coal_t< L > make_coal_substring( const char* str, int start = 0 ) { return make_coal_substring_impl< L >( str, start, std::make_integer_sequence< int, L-1 >() ); }
    template< int L > constexpr const coal_t< L + 1 > substring( int start = 0 ) const { return make_coal_substring< L + 1 >( s, start );}

    /* append */
    static constexpr char coal_impl_getchar_append( const char* strA, const char* strB, int lenA, int num ) { return num < lenA ? strA[num] : strB[num-lenA]; }
    template< int L, typename T, T... Nums > static constexpr const coal_t< L > make_coal_append_impl(  const char* strA, const char* strB, std::integer_sequence< T, Nums... > ) { return { coal_impl_getchar_append( strA, strB, strlen( strA ), Nums ) ..., '\0' }; }
    template< int L > static constexpr const coal_t< L > make_coal_append( const char* strA, const char* strB ) { return make_coal_append_impl< L >( strA, strB, std::make_integer_sequence< int, L-1 >() ); }
    template< int L > constexpr const coal_t< N + L - 1 > append( const char (&str)[L] ) const { return make_coal_append< N + L - 1 >( s, str ); }
    template< int L > constexpr const coal_t< N + L - 1 > append( const coal_t< L >& i ) const { return  append( i.s ); }

    /* prepend */
    static constexpr char coal_impl_getchar_prepend( const char* strA, const char* strB, int lenB, int num ) { return num < lenB ? strB[num] : strA[num-lenB]; }
    template< int L, typename T, T... Nums > static constexpr const coal_t< L > make_coal_prepend_impl(  const char* strA, const char* strB, std::integer_sequence< T, Nums... > ) { return { coal_impl_getchar_prepend( strA, strB, strlen( strB ), Nums ) ..., '\0' }; }
    template< int L > static constexpr const coal_t< L > make_coal_prepend( const char* strA, const char* strB ) { return make_coal_prepend_impl< L >( strA, strB, std::make_integer_sequence< int, L-1 >() ); }
    template< int L > constexpr const coal_t< N + L - 1 > prepend( const char (&str)[L] ) const { return make_coal_prepend< N + L - 1 >( s, str ); }
    template< int L > constexpr const coal_t< N + L - 1 > prepend( const coal_t< L >& i ) const { return  prepend( i.s ); }

    /* copy */
    constexpr const coal_t< N > copy() const { return  *this; }
};


/////////////////////////////////////////////////////
// coal maker from string
template< int N, typename T, T... Nums >    constexpr coal_t< N > make_coal_from_string_impl( const char* str, std::integer_sequence< T, Nums... > )    { return { str[Nums] ... }; }
template< int N >                           constexpr coal_t< N > make_coal_from_string     ( const char (&str)[N] )                                    { return make_coal_from_string_impl< N >( str, std::make_integer_sequence< int, N >() ); }

/////////////////////////////////////////////////////
// coal maker from cstr
template< int N > constexpr coal_t< N > make_coal_from_cstr( const char* str ) { return make_coal_from_string_impl< N >( str, std::make_integer_sequence< int, N >() ); }

/////////////////////////////////////////////////////
// coal maker from int
constexpr char
make_coal_from_int_impl_getchar( int i, int num )
{
    int result = strip_most_significant_digits( i, num );
    int reduction = count_digits( result ) - 1;
    for( int j = 0; j < reduction; ++j )
        result /= 10;
    return  result + '0';
}
template< int N, typename T, T... Nums >    constexpr coal_t< N > make_coal_from_int_impl   ( int i, std::integer_sequence< T, Nums... > )  { return { make_coal_from_int_impl_getchar( i, Nums ) ... }; }
template< int N >                           constexpr coal_t< N > make_coal_from_int        ( int i )                                       { return make_coal_from_int_impl< N >( i, std::make_integer_sequence< int, N - 1 >() ); }

/////////////////////////////////////////////////////
// coal operators
template< int N, int L > constexpr coal_t< N + L - 1> operator+( const coal_t< N >& A,  const coal_t< L >& B )  { return  A.append( B ); }                              // coal coal
template< int N, int L > constexpr coal_t< N + L - 1> operator+( const coal_t< N >& A,  const char (&B)[L]   )  { return  A.append( make_coal_from_string( B ) ); }     // coal char
template< int N, int L > constexpr coal_t< N + L - 1> operator+( const char (&A)[N],    const coal_t< L >& B )  { return  B.prepend( make_coal_from_string( A ) ); }    // char coal

/////////////////////////////////////////////////////
// coal utility macro
#define coal                            constexpr  auto
#define coalMakeFromString(i)           ::__coal__::make_coal_from_string( i )
#define coalMakeFromCstr(i)             ::__coal__::make_coal_from_cstr< ::__coal__::strlen( i ) + 1 >( i )
#define coalMakeFromInt(i)              ::__coal__::make_coal_from_int< ::__coal__::count_digits( i ) + 1 >( i )
#define coalSplitElem( obj, sep, ind )  obj.substring< obj.querySplitWordSize( sep, ind ) >( obj.querySplitWordStart( sep, ind ) )


#if defined(__clang__)
#define coalSubstring( obj, start, len ) obj.template substring< len >( start )
#elif defined(__GNUC__) || defined(__GNUG__)
#define coalSubstring( obj, start, len ) obj.template substring< len >( start )
#elif defined(_MSC_VER)
#define coalSubstring( obj, start, len ) obj.substring< len >( start )
#endif

/////////////////////////////////////////////////////
// coal constexpr args macro
#define coalConstexprWrapper(...)               struct { static constexpr auto value() { return __VA_ARGS__; } }
#define coalMakeConstexprArg( iName, iValue )   using iName = coalConstexprWrapper( iValue );
// This is C++17, not compliant with C++14 as the rest of the lib
//#define coalConstexprArg(...)                 [] { using R = coalConstexprWrapper(__VA_ARGS__); return R{}; }()


} // namespace __coal__

