// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.FX.Convolution.Kernel.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 * @brief       This file provides the declaration for the Kernel tools.
 */
#pragma once
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Maths/ULIS.Maths.Geometry.h"
#include "ULIS/Maths/ULIS.Maths.Utility.h"
#include <initializer_list>
#include <vector>
#include <memory>

namespace ULIS {
/////////////////////////////////////////////////////
// Kernel Edge Handling
enum class eKernelEdgeMode : uint8
{
      kExtend
    , kConstant
    , kMirror
    , kWrap
    , kTransparent
    , kSkip
};

/////////////////////////////////////////////////////
// Kernel Edge Handling
enum class eConvolutionOperator : uint8
{
      kExtend
    , kConstant
    , kMirror
    , kWrap
    , kTransparent
    , kSkip
};


/////////////////////////////////////////////////////
// FKernel
class  FKernel
{
public:
    // Construction / Destruction
    ~FKernel();
    FKernel( const FSize& iSize, float iValue = 0.f, const FPoint& iAnchor = FPoint( -1, -1 ), eKernelEdgeMode iKernelEdgeMode = eKernelEdgeMode::kExtend );
    FKernel( const FSize& iSize, std::initializer_list< float > iNums, const FPoint& iAnchor = FPoint( -1, -1 ), eKernelEdgeMode iKernelEdgeMode = eKernelEdgeMode::kExtend );
    FKernel( const FKernel& iRhs );             // Copy Constructor
    FKernel& operator=( const FKernel& iRhs );  // Copy Assignment Operator
    FKernel( FKernel&& iRhs );                  // Move Constructor
    FKernel& operator=( FKernel&& iRhs );       // Move Assignment Operator

public:
    // Public API
    void  Set( std::initializer_list< float > iNums );
    float At( uint iX, uint iY ) const;
    float At( FPoint iPoint ) const;
    void SetAt( uint iX, uint iY, float iValue );
    void SetAt( FPoint iPoint, float iValue );
    float& operator[] ( uint iIndex );
    const float& operator[] ( uint iIndex ) const;
    void Clear();
    void Fill( float iValue );
    void SetZeroes();
    void SetOnes();
    float Sum() const;
    void Shift( float iValue );
    void Mul( float iValue );
    void Normalize();
    bool IsNormalized() const;
    void FlipX();
    void FlipY();
    void Rotate90CW();
    void Rotate90CCW();
    void Rotate180();
    FKernel Normalized() const;
    FKernel FlippedX() const;
    FKernel FlippedY() const;
    FKernel Rotated90CW() const;
    FKernel Rotated90CCW() const;
    FKernel Rotated180() const;
    int Width() const;
    int Height() const;
    int Length() const;
    const FSize& Size() const;
    const FPoint& Anchor() const;
    eKernelEdgeMode KernelEdgeMode() const;

private:
    // Private API
    void Init();
    void ComputeAnchor();
    void AssertValid() const;
    void Allocate();
    void Deallocate();

private:
    // Private member data
    FSize mSize;
    FPoint mAnchor;
    eKernelEdgeMode mKernelEdgeMode;
    std::unique_ptr< float[] > mData;
};

typedef  std::vector< FKernel > FKernelChain;

} // namespace ULIS

