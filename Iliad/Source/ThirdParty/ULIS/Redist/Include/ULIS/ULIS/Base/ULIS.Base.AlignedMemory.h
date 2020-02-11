// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Base.AlignedMemory.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the definition for the template class TAlignedMemory.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"

namespace ULIS {
/////////////////////////////////////////////////////
/// @class      TAlignedMemory
/// @brief      [Deprecated] The TAlignedMemory template class provides a mean of managing arbitrary alignement for data in memory.
/// @details    There are two template parameters:
///                 - The template type parameter \a T is the type of the data to allocate in an aligned fashion.
///                 - The template value parameter \a _Align of type size_t is the requested alignement ( any unsigned integer value, but more likely
///                 a power of 2, e.g: 8, 16, 32, 64, ... ).
template< typename T, size_t _Align >
class TAlignedMemory
{
public:
//--------------------------------------------------------------------------------------
//----------------------------------------------------------- Construction / Destruction
    /// @fn         TAlignedMemory()
    /// @brief      Default Constructor.
    /// @details    Initializes all members to 0 or nullmPtr.
    TAlignedMemory()
        : mRaw              ( nullptr   )
        , mPtr              ( nullptr   )
        , mAllocatedSize    ( 0         )
        , mPadLeft          ( 0         )
        , mPadRight         ( 0         )
    {}


    /// @fn         ~TAlignedMemory()
    /// @brief      Default Destructor.
    /// @details    Deletes the underlying data if it was allocated in the first place with \e Allocate.
    ~TAlignedMemory()
    {
        if( mRaw ) delete[] mRaw;
    }

public:
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------- Public API
    /// @fn         Allocate( size_t iSizeBytes )
    /// @brief      Allocate a certain number of bytes of aligned data based on parameter.
    /// @details    The method allocates paddings around the data to ensure it is complies on both bounds to the specified template alignment.
    /// @param      iSizeBytes      The size of the data to allocate in bytes.
    void  Allocate( size_t iSizeBytes )
    {
        size_t offset = _Align - 1;
        mAllocatedSize = iSizeBytes;
        mPadRight = ( _Align - ( iSizeBytes & offset ) ) & offset;
        size_t total = offset + iSizeBytes + mPadRight;
        mRaw = new uint8[ total ];

        size_t memloc_mRaw_start = (size_t)mRaw;
        mPadLeft = ( _Align - ( memloc_mRaw_start & offset ) ) & offset;
        mPtr = mRaw + mPadLeft;
    }


    /// @fn         AllocatedSize()  const
    /// @brief      Query the requested allocated size in bytes.
    /// @return     A size_t representing the requested allocated size in bytes.
    size_t  AllocatedSize()  const  { return  mAllocatedSize; }


    /// @fn         Alignment()  const
    /// @brief      Query the requested template alignment.
    /// @return     A size_t representing the requested template alignment.
    size_t  Alignment()  const  { return  _Align; }


    /// @fn         Waste()  const
    /// @brief      Query the wasted size in bytes of data that has been allocated to ensure alignment.
    /// @details    The wasted size is computed as the sum of the paddings.
    /// @return     A size_t representing the wasted size in bytes to ensure alignment.
    size_t  Waste()  const  { return  mPadLeft + mPadRight; }


    /// @fn         Ptr()  const
    /// @brief      Query the base adress of the aligned data.
    /// @return     A pointer representing the aligned data.
    T*  Ptr()  const  { return  mPtr; }


private:
//--------------------------------------------------------------------------------------
//----------------------------------------------------------------- Private Data Members
    T*      mRaw;               ///< Pointer to \a T, if Allocated, points to the base address ( before left padding ) of the unaligned allocated data.
    T*      mPtr;               ///< Pointer to \a T, if Allocated, points to the base address ( after left padding ) of the aligned allocated data.
    size_t  mAllocatedSize;     ///< Size in bytes of the requested size, the actual allocation can be greater.
    size_t  mPadLeft;           ///< Left padding in bytes of the aligned data, can be zero.
    size_t  mPadRight;          ///< Right padding in bytes of the aligned data, can be zero.

};

} // namespace ULIS

