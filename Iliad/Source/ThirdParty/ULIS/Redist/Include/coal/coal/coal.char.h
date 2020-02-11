/*************************************************************************
*
*   coal
*__________________
*
* coal.char.h
* Clement Berthaud - Layl
* Please refer to LICENSE.md
*/

#pragma once


namespace __coal__ {
/////////////////////////////////////////////////////
// c++14 compliant Constexpr Functions
constexpr int
strlen( const char* s )
{
    for( int i = 0;; ++i ) if( !s[i] ) return  i;
}


constexpr bool
streq( char const * a, char const * b )
{
    int lena = strlen( a );
    int lenb = strlen( b );
    if( lena != lenb ) return  false;

    for( int i = 0; i < lena; ++i )
        if( a[i] != b[i] )
            return  false;

    return  true;
}


constexpr bool
is_digit( char c )
{
    return  c >= '0' && c <= '9';
}


constexpr bool
is_number( const char* s )
{
    for( int i = 0; i < strlen( s ); ++i )
        if( !is_digit( s[i] ) )
            return  false;

    return  true;
}


constexpr int
stoi( const char* s )
{
    int r = 0;
    for( int i = 0; i < strlen( s ); ++i )
        r = r * 10 + int( s[i] - '0' );
    return  r;
}


constexpr int
indexof( const char* word, const char* str, int from = 0 )
{
    int ls = strlen( str );
    int lw = strlen( word );

    for( int i = from; i < ls; ++i )
    {
        for( int j = 0; j < lw; ++j )
        {
            if( str[i+j] != word[j] )
                break;

            if( j == lw - 1 )
            {
                return  i;
            }
        }
    }

    return  -1;
}


constexpr int
indexof( char letter, const char* str, int from = 0 )
{
    int ls = strlen( str );

    for( int i = from; i < ls; ++i )
        if( str[i] == letter )
            return  i;

    return  -1;
}


constexpr int
count_digits( int n )
{
    int i = 0;
    for( i = 0; n > 0; ++i )
        n /= 10;

    return  i == 0 ? 1 : i;
}


constexpr int
strip_most_significant_digits( int iNumber, int iIndex )
{
    if( iIndex == 0 ) return  iNumber;

    int result = iNumber;
    int reduction = count_digits( iNumber ) - 1;
    for( int i = 0; i < reduction; ++i )
        result /= 10;
    for( int i = 0; i < reduction; ++i )
        result *= 10;
    result = iNumber - result;
    return  strip_most_significant_digits( result, iIndex - 1 );
}


} // namespace __coal__

