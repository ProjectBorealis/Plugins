// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Maths.Geometry.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */
#pragma once
#include "ULIS/ULIS.Config.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Maths/ULIS.Maths.Utility.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <vector>

namespace ULIS {

struct FRect;
struct FPoint;
struct FPoint64;
struct FTransformAABB;
struct FTransformOBB;
struct FLinef;
struct FOBBSlopeExpression;
struct FOBBHullExpression;

/////////////////////////////////////////////////////
// FRect
struct FRect
{
    int x;
    int y;
    int w;
    int h;

    FRect();
    FRect( int iX, int iY, int iW, int iH );
    static FRect FromXYWH( int iX, int iY, int iW, int iH );
    static FRect FromMinMax( int iXMin, int iYMin, int iXMax, int iYMax );
    FRect operator&( const FRect& iOther ) const;
    FRect operator|( const FRect& iOther ) const;
    bool operator==( const FRect& iOther ) const;
    int Area() const;
};


/////////////////////////////////////////////////////
// FPoint
struct FPoint
{
    int x;
    int y;

    FPoint();
    FPoint( int iX, int iY );
    FPoint RotateAround( FPoint pivotPoint, double radianRotation);
    FPoint AxialSymmetry( float a, float b );
};
typedef FPoint FSize;


/////////////////////////////////////////////////////
// FPoint64 bits
struct FPoint64
{
    int64 x;
    int64 y;

    FPoint64();
    FPoint64( int64 iX, int64 iY );
    FPoint64 RotateAround( FPoint pivotPoint, double radianRotation);
    FPoint64 AxialSymmetry( float a, float b );
};

/////////////////////////////////////////////////////
// FTransformAABB
// A rectangle used to preview the transformation bounding box result
// Used for precomputation and allocation before actual transforms
struct FTransformAABB
{
    float x1, y1, x2, y2;

    FTransformAABB( float iX1, float iY1, float iX2, float iY2 );
    FTransformAABB( const  std::vector< glm::vec2 >& iPoints );
    FTransformAABB( const  FTransformOBB& iOBB );
    void GetPoints( std::vector< glm::vec2 >* oPoints ) const;
    void GetPoints( std::vector< glm::vec3 >* opoints ) const;
    void SetPoints( const std::vector< glm::vec2 >& points );
    void SetPoints( const std::vector< glm::vec3 >& points );
    void Transform( const glm::mat3& mat );
    void FitInPositiveRange();
    void Shift( const glm::vec2& iVec );
    glm::vec2 GetShift();
    float Width() const;
    float Height() const;
};

/////////////////////////////////////////////////////
// Sort
bool SortCompareLesserX( const glm::vec2& iA, const glm::vec2& iB );
bool SortCompareLesserY( const glm::vec2& iA, const glm::vec2& iB );

/////////////////////////////////////////////////////
// FTransformOBB
// A oriented rectangle used to preview the transformation bounding box result
// Used for precomputation and allocation before actual transforms
struct FTransformOBB
{
    glm::vec2 m00, m10, m11, m01;

    FTransformOBB( const glm::vec2& iM00, const glm::vec2& iM10, const glm::vec2& iM11, const glm::vec2& iM01 );
    FTransformOBB( float iX1, float iY1, float iX2, float iY2 );
    FTransformOBB( const FTransformAABB& iAABB );
    void Transform( const glm::mat3& mat );
    void Shift( const glm::vec2& iVec );
    void GetPoints( std::vector< glm::vec2 >* oPoints ) const;
    FOBBHullExpression BuildHullExpression() const;
};

/////////////////////////////////////////////////////
// FLinef
struct FLinef
{
    float a, b;

    FLinef( float iA, float iB );
    static FLinef FromPointsVertical( const glm::vec2& iPointA, const glm::vec2& iPointB );
    float Eval( float iX ) const;
};


/////////////////////////////////////////////////////
// FOBBSlopeExpression
struct FOBBSlopeExpression
{
    std::vector< FLinef > lines;
    int indexer;

    FOBBSlopeExpression();
    float Eval( int iX ) const;
};


/////////////////////////////////////////////////////
// FOBBHullExpression
struct FOBBHullExpression
{
    FOBBSlopeExpression left, right;

    FOBBHullExpression();
    FOBBHullExpression( const FOBBSlopeExpression& iLeft, const FOBBSlopeExpression& iRight );
};

} // namespace ULIS

