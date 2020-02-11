/*************************************************************************
*
*   coal
*__________________
*
* coal.list.h
* Clement Berthaud - Layl
* Please refer to LICENSE.md
*/

#pragma once

#include "coal/coal.coal.h"


namespace __coal__ {


/////////////////////////////////////////////////////
// coal list maker from coal
template< typename iCoal, typename iSep, typename T, T... Nums >
constexpr  auto
make_coal_list_as_tuple_imp( std::integer_sequence< T, Nums... > )
{
    return  std::make_tuple( coalSplitElem( iCoal::value(), iSep::value(), Nums ) ... );
}


template< typename iCoal, typename iSep >
constexpr  auto
make_coal_list_as_tuple( iCoal, iSep )
{
    return  make_coal_list_as_tuple_imp< iCoal, iSep >( std::make_integer_sequence< int, iCoal::value().querySplitNum( iSep::value() ) >() );
}


/////////////////////////////////////////////////////
// coal list cat
template< typename iList, typename iSep >
constexpr  auto
coal_list_cat( iList, iSep )
{
    return  0;
}

/////////////////////////////////////////////////////
// list utility macro
#define coalList    constexpr  auto
#define coalSplit( iCoal, iSep )   ::__coal__::make_coal_list_as_tuple( iCoal, iSep )

#define coalMakeSplitList( iName, iCoal, iSep )     \
    coalMakeConstexprArg( __FILE__ ## __LINE__ ## __const__ ## iCoal, iCoal ); \
    coalMakeConstexprArg( __FILE__ ## __LINE__ ## __const__ ## sep,  iSep );  \
    coalList iName = coalSplit( __FILE__ ## __LINE__ ## __const__ ## iCoal {}, __FILE__ ## __LINE__ ## __const__ ## sep {} );

#define coalListGet( iName, iIndex )    std::get< iIndex >( iName )
#define coalListSize( iName )   std::tuple_size< decltype(iName) >::value

} // namespace __coal__

