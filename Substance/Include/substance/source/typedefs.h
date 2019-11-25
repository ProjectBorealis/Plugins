//! @file memory.h
//! @brief Substance Source Typedefs
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
#ifndef _SUBSTANCE_SOURCE_TYPEDEFS_H_
#define _SUBSTANCE_SOURCE_TYPEDEFS_H_

//C++ STL includes used as base for specialization
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

namespace Alg
{
namespace Source
{

/** STL aliases */
template<typename T, std::size_t N>		using Array			= std::array<T, N>;
template<typename T>					using SharedPtr		= std::shared_ptr<T>;
template<typename T>					using WeakPtr		= std::weak_ptr<T>;
template<typename K, typename V>		using Map			= std::map<K, V, std::less<K>, AlignedAllocator<std::pair<const K, V>>>;
template<typename K>					using Set			= std::set<K, std::less<K>, AlignedAllocator<K>>;
template<typename K, typename V>		using UnorderedMap	= std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, AlignedAllocator<std::pair<const K, V>>>;
template<typename T>					using Vector		= std::vector<T, AlignedAllocator<T>>;

/** Typedef specializations */
typedef std::basic_istringstream<char, std::char_traits<char>, AlignedAllocator<char>> Istringstream;
typedef std::basic_string<char, std::char_traits<char>, AlignedAllocator<char>> String;
typedef std::basic_stringstream<char, std::char_traits<char>, AlignedAllocator<char>> Stringstream;

/** Convert to string */
template<typename T> String to_string(T value)
{
	//gross but simple
	std::string result = std::to_string(value);
	return String(result.c_str());
}

/** Create shared pointer */
template<typename T, class... Args> SharedPtr<T> MakeShared(Args&& ... args)
{
	return std::allocate_shared<T>(AlignedAllocator<T>(), std::forward<Args>(args)...);
}

/** Static Cast Shared Ptr */
template<typename T, typename U> SharedPtr<T> StaticPointerCast(const SharedPtr<U>& r) noexcept
{
	return std::static_pointer_cast<T>(r);
}

} //Source
} //Alg

#endif //_SUBSTANCE_SOURCE_TYPEDEFS_H_
