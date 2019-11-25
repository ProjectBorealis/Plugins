//! @file memory.h
//! @brief Substance Source Memory Management
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
#ifndef _SUBSTANCE_SOURCE_MEMORY_H_
#define _SUBSTANCE_SOURCE_MEMORY_H_

#if !defined(SUBSTANCE_SOURCE_DEFAULT_ALIGNMENT)
#define SUBSTANCE_SOURCE_DEFAULT_ALIGNMENT 16
#endif

namespace Alg
{
namespace Source
{

void* alignedMalloc(size_t bytesCount, size_t alignment);
void alignedFree(void* ptr);

/**/
template<typename T, int ALIGNMENT = SUBSTANCE_SOURCE_DEFAULT_ALIGNMENT> struct AlignedAllocator
{
	typedef T value_type;

	/* Rebind structure for casting to a different allocator type */
	template<class U> struct rebind
	{
		typedef AlignedAllocator<U, ALIGNMENT> other;
	};

	/* Construct allocator */
	AlignedAllocator() = default;

	/* Conversion constructor */
	template<typename U> AlignedAllocator(const AlignedAllocator<U>&)
	{
	}

	/* Allocate memory */
	T* allocate(std::size_t n)
	{
		return static_cast<T*>(alignedMalloc(sizeof(T) * n, ALIGNMENT));
	}

	/* Free memory */
	void deallocate(T* ptr, std::size_t)
	{
		alignedFree(ptr);
	}
};

/**/
template <class T, class U, int ALIGNMENT> constexpr bool operator==(const AlignedAllocator<T, ALIGNMENT>&, const AlignedAllocator<U, ALIGNMENT>&)
{
	return true;
}

/**/
template <class T, class U, int ALIGNMENT> constexpr bool operator!=(const AlignedAllocator<T, ALIGNMENT>&, const AlignedAllocator<U, ALIGNMENT>&)
{
	return false;
}

//delete "placement new" style allocations
template<typename T> void deleteObject(T* ptr)
{
	ptr->~T();
	alignedFree(ptr);
}

//passed to stl functions to ensure proper deletion of objects
template<typename T> struct Deleter
{
	void operator()(T* ptr)
	{
		deleteObject(ptr);
	}
};

//used to ensure only our macros use our global new/delete operators
struct MallocDummy
{
	int unused;
};

} //Source
} //Alg

inline void* operator new (size_t size, Alg::Source::MallocDummy*, const std::nothrow_t&)
{
	return Alg::Source::alignedMalloc(size, SUBSTANCE_SOURCE_DEFAULT_ALIGNMENT);
}

//compiler complains if we don't have this, but do NOT use it!
inline void operator delete (void*, Alg::Source::MallocDummy*, const std::nothrow_t&)
{
	assert(false);
}

#define SUBSTANCE_SOURCE_NEW(type) new ((Alg::Source::MallocDummy*)0, std::nothrow) type
#define SUBSTANCE_SOURCE_DELETE(ptr) Alg::Source::deleteObject(ptr)

#endif //_SUBSTANCE_SOURCE_MEMORY_H_
