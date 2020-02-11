// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Paint.PainterContext.h
 * @author      Thomas Schmitt
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */
#pragma once
#include <vector>
#include <map>
#include "ULIS/ClearFill/ULIS.ClearFIll.ClearFillContext.h"
#include "ULIS/Base/ULIS.Base.BaseTypes.h"
#include "ULIS/Base/ULIS.Base.PerformanceOptions.h"
#include "ULIS/Data/ULIS.Data.Block.h"

namespace ULIS {

namespace eClippingZone
{
    const uint8 kInside   = 0b0000;
    const uint8 kLeft     = 0b0001;
    const uint8 kRight    = 0b0010;
    const uint8 kBot      = 0b0100;
    const uint8 kTop      = 0b1000;
};
typedef uint8 tClippingCode;

/////////////////////////////////////////////////////
// TPainterContext
template< uint32 _SH >
class TPainterContext
{

//Functions used for clipping of forms
private:
    
    static const int InternalComputeCodeForPoint ( const FPoint& iPoint, const FRect& iClippingRect )
    {
        // initialized as being inside
        int code = eClippingZone::kInside;
      
        if (iPoint.x < iClippingRect.x)       // to the left of rectangle
            code |= eClippingZone::kLeft;
        else if (iPoint.x > (iClippingRect.x + iClippingRect.w) )  // to the right of rectangle
            code |= eClippingZone::kRight;
        if (iPoint.y < iClippingRect.y)       // above the rectangle
            code |= eClippingZone::kTop;
        else if (iPoint.y > (iClippingRect.y + iClippingRect.h) )  // below the rectangle
            code |= eClippingZone::kBot;
      
        return code;
    }
    
    // Returns true if ioP0 and ioP1 are different at the end of the algorithm
    static const bool InternalCropLineToRectangle( FPoint& ioP0, FPoint& ioP1, const FRect& iClippingRect )
    {
        int xMax = iClippingRect.x + iClippingRect.w;
        int yMax = iClippingRect.y + iClippingRect.h;
        int xMin = iClippingRect.x;
        int yMin = iClippingRect.y;
        
        tClippingCode codeP0 = InternalComputeCodeForPoint( ioP0, iClippingRect );
        tClippingCode codeP1 = InternalComputeCodeForPoint( ioP1, iClippingRect );
        bool accept = false;

        while (true)
        {
            if ((codeP0 == 0) && (codeP1 == 0))
            {
                // If both endpoints lie within rectangle
                accept = true;
                break;
            }
            else if (codeP0 & codeP1)
            {
                // If both endpoints are outside rectangle,
                // in same region
                break;
            }
            else
            {
                // Some segment of line lies within the
                // rectangle
                tClippingCode code_out;
                double x, y;
      
                // At least one endpoint is outside the
                // rectangle, pick it.
                if (codeP0 != 0)
                    code_out = codeP0;
                else
                    code_out = codeP1;
      
                // Find intersection point;
                // using formulas y = y1 + slope * (x - x1),
                // x = x1 + (1 / slope) * (y - y1)
                if (code_out & eClippingZone::kBot)
                {
                    // point is above the clip rectangle
                    x = ioP0.x + (ioP1.x - ioP0.x) * (yMax - ioP0.y) / (ioP1.y - ioP0.y);
                    y = yMax;
                }
                else if (code_out & eClippingZone::kTop)
                {
                    // point is below the rectangle
                    x = ioP0.x + (ioP1.x - ioP0.x) * (yMin - ioP0.y) / (ioP1.y - ioP0.y);
                    y = yMin;
                }
                else if (code_out & eClippingZone::kRight)
                {
                    // point is to the right of rectangle
                    y = ioP0.y + (ioP1.y - ioP0.y) * (xMax - ioP0.x) / (ioP1.x - ioP0.x);
                    x = xMax;
                }
                else if (code_out & eClippingZone::kLeft)
                {
                    // point is to the left of rectangle
                    y = ioP0.y + (ioP1.y - ioP0.y) * (xMin - ioP0.x) / (ioP1.x - ioP0.x);
                    x = xMin;
                }
      
                // Now intersection point x,y is found
                // We replace point outside rectangle
                // by intersection point
                if (code_out == codeP0)
                {
                    ioP0.x = x;
                    ioP0.y = y;
                    codeP0 = InternalComputeCodeForPoint(ioP0, iClippingRect);
                }
                else
                {
                    ioP1.x = x;
                    ioP1.y = y;
                    codeP1 = InternalComputeCodeForPoint(ioP1, iClippingRect);
                }
            }
        }
        return accept;
    }
    
// -----------------------------------------------------------
    
    
public:
static void DrawLine( TBlock< _SH >*                        iBlock
                        , const FPoint                      iP0
                        , const FPoint                      iP1
                        , const CColor&                     iColor
                        , const FRect&                      iClippingRect
                        , const FPerformanceOptions&        iPerformanceOptions
                        , bool                              iCallInvalidCB )
{
    //Clipping ----
    
    FPoint p0 = iP0;
    FPoint p1 = iP1;

    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    if( !InternalCropLineToRectangle( p0, p1, clippingRect ) )
        return; //Nothing to draw
    
    //Drawing ----
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );

    if( ::ULIS::FMath::Abs( p1.y - p0.y ) < ::ULIS::FMath::Abs( p1.x - p0.x )) // x slope > y slope
    {
        if( p1.x < p0.x )
        {
            ULIS::FPoint temp = p0;
            p0 = p1;
            p1 = temp;
        }

        int dx = p1.x - p0.x;
        int dy = p1.y - p0.y;
        int yStep = 1;

        if( dy < 0)
        {
            yStep = -1;
            dy = -dy;
        }

        int slopeDifferential = 2 * dy - dx;
        int y = p0.y;

        for( int x = p0.x; x <= p1.x; x++)
        {
            iBlock->SetPixelValue( x, y, val );

            if( slopeDifferential > 0 )
            {
                y += yStep;
                slopeDifferential-=(2 * dx);
            }
            slopeDifferential+=(2 * dy);
        }
    }
    else //y slope > x slope
    {
        if( p1.y < p0.y )
        {
            ULIS::FPoint temp = p0;
            p0 = p1;
            p1 = temp;
        }

        int dx = p1.x - p0.x;
        int dy = p1.y - p0.y;
        int xStep = 1;

        if( dx < 0)
        {
            xStep = -1;
            dx = -dx;
        }

        int slopeDifferential = 2 * dx - dy;
        int x = p0.x;

        for( int y = p0.y; y <= p1.y; y++)
        {
            iBlock->SetPixelValue( x, y, val );

            if( slopeDifferential > 0 )
            {
                x += xStep;
                slopeDifferential-=(2 * dy);
            }
            slopeDifferential+=(2 * dx);
        }
    }
}

// ---


static void DrawLineAA( TBlock< _SH >*            iBlock
                        , const FPoint             iP0
                        , const FPoint             iP1
                        , const CColor&            iColor
                        , const FRect&                      iClippingRect
                        , const FPerformanceOptions&        iPerformanceOptions
                        , bool                     iCallInvalidCB )
{
    //Clipping ----
    
    FPoint p0 = iP0;
    FPoint p1 = iP1;
    
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int xMax = clippingRect.x + clippingRect.w;
    int yMax = clippingRect.y + clippingRect.h;
    int xMin = clippingRect.x;
    int yMin = clippingRect.y;
    
    int codeP0 = InternalComputeCodeForPoint( p0, clippingRect );
    int codeP1 = InternalComputeCodeForPoint( p1, clippingRect );
    
    bool accept = false;

    while (true)
    {
        if ((codeP0 == 0) && (codeP1 == 0))
        {
            // If both endpoints lie within rectangle
            accept = true;
            break;
        }
        else if (codeP0 & codeP1)
        {
            // If both endpoints are outside rectangle,
            // in same region
            break;
        }
        else
        {
            // Some segment of line lies within the
            // rectangle
            int code_out;
            double x, y;
  
            // At least one endpoint is outside the
            // rectangle, pick it.
            if (codeP0 != 0)
                code_out = codeP0;
            else
                code_out = codeP1;
  
            // Find intersection point;
            // using formulas y = y1 + slope * (x - x1),
            // x = x1 + (1 / slope) * (y - y1)
            if (code_out & eClippingZone::kBot)
            {
                // point is above the clip rectangle
                x = p0.x + (p1.x - p0.x) * (yMax - p0.y) / (p1.y - p0.y);
                y = yMax;
            }
            else if (code_out & eClippingZone::kTop)
            {
                // point is below the rectangle
                x = p0.x + (p1.x - p0.x) * (yMin - p0.y) / (p1.y - p0.y);
                y = yMin;
            }
            else if (code_out & eClippingZone::kRight)
            {
                // point is to the right of rectangle
                y = p0.y + (p1.y - p0.y) * (xMax - p0.x) / (p1.x - p0.x);
                x = xMax;
            }
            else if (code_out & eClippingZone::kLeft)
            {
                // point is to the left of rectangle
                y = p0.y + (p1.y - p0.y) * (xMin - p0.x) / (p1.x - p0.x);
                x = xMin;
            }
  
            // Now intersection point x,y is found
            // We replace point outside rectangle
            // by intersection point
            if (code_out == codeP0)
            {
                p0.x = x;
                p0.y = y;
                codeP0 = InternalComputeCodeForPoint(p0, clippingRect);
            }
            else
            {
                p1.x = x;
                p1.y = y;
                codeP1 = InternalComputeCodeForPoint(p1, clippingRect);
            }
        }
    }
    
    if( !accept )
        return; //Nothing to draw
    
    //Drawing
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );

    auto MaxAlpha = val.GetAlpha();

    if( ::ULIS::FMath::Abs( p1.y - p0.y ) < ::ULIS::FMath::Abs( p1.x - p0.x )) //x slope > y slope
    {
        if( p1.x < p0.x )
        {
            ULIS::FPoint temp = p0;
            p0 = p1;
            p1 = temp;
        }

        int dx = p1.x - p0.x;
        int dy = p1.y - p0.y;
        int yStep = 1;

        if( dy < 0)
        {
            yStep = -1;
            dy = -dy;
        }

        int errMax = -2 * dx + 2 * dy + 1;
        int errMin = 2 * dy - 1;
        int slopeDifferential = 2 * dy - dx;
        int y = p0.y;

        for( int x = p0.x; x <= p1.x; x++)
        {
            float alphaTop = (1 - FMath::Abs( ( float( slopeDifferential - errMax ) / float( errMin - errMax ) ) - 0.5 ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );
            iBlock->SetPixelValue( x, y, val );

            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            iBlock->SetPixelValue( x, y + yStep, val );

            if( slopeDifferential >= dx )
            {
                y += yStep;
                slopeDifferential-=(2 * dx);
            }
            slopeDifferential+=(2 * dy);
        }
    }
    else //y slope > x slope
    {
        if( p1.y < p0.y )
        {
            ULIS::FPoint temp = p0;
            p0 = p1;
            p1 = temp;
        }

        int dx = p1.x - p0.x;
        int dy = p1.y - p0.y;
        int xStep = 1;

        if( dx < 0)
        {
            xStep = -1;
            dx = -dx;
        }

        int errMax = -2 * dy + 2 * dx + 1;
        int errMin = 2 * dx - 1;
        int slopeDifferential = 2 * dx - dy;
        int x = p0.x;

        for( int y = p0.y; y <= p1.y; y++)
        {
            float alphaTop = (1 - FMath::Abs( ( float( slopeDifferential - errMax ) / float( errMin - errMax ) ) - 0.5 ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );
            iBlock->SetPixelValue( x, y, val );

            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            iBlock->SetPixelValue( x + xStep, y, val );

            if( slopeDifferential >= dy )
            {
                x += xStep;
                slopeDifferential-=(2 * dy);
            }
            slopeDifferential+=(2 * dx);
        }
    }
}

// ---


//You can draw concentric circles with this one. But multiple pixel outline at some part of the circle -> messier result
static void DrawCircleAndres( TBlock< _SH >*            iBlock
                                , const FPoint             iCenter
                                , const int                iRadius
                                , const CColor&            iColor
                                , const bool               iFilled
                                , const FRect&                      iClippingRect
                                , const FPerformanceOptions&        iPerformanceOptions
                                , bool                     iCallInvalidCB )
{
    //Clipping -----
    int x = 0;
    int y = iRadius; //We start from the top of the circle for the first octant

    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int shift45 = std::sin( FMath::DegToRad( 45 ) ) * iRadius;
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iRadius );
    FPoint point45 = FPoint( iCenter.x + shift45, iCenter.y - shift45 );
    FPoint point90 = FPoint( iCenter.x + iRadius, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45, iCenter.y + shift45 );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iRadius );
    FPoint point225 = FPoint( iCenter.x - shift45, iCenter.y + shift45 );
    FPoint point270 = FPoint( iCenter.x - iRadius, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45, iCenter.y - shift45 );
    
    FRect rectOctant1 = FRect( point0.x, point0.y, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;
    
    //Drawing -----
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );
    
    
    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        int diff = iRadius - 1;

        //Left and top clip
        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx++;
            }
        }
        
        //Right and bottom clip
        while (xx <= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx++;
            }
        }
    }
    
    
    //Octant 2 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant2 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
        
        int diff = iRadius - 1;

        //Right and bottom clip
        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy--;
            }
        }
        
        //Top and left clip
        while (xx >= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy--;
            }
        }
    }
    
    
    //Octant 3 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant3 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
        
        int diff = iRadius - 1;

        //Right and top clip
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy++;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy++;
            }
        }
    }

    
    //Octant 4 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant4 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx++;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx++;
            }
        }
    }
    
    
    //Octant 5 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant5 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx--;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx--;
            }
        }
    }
    
    
    //Octant 6 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant6 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy++;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy++;
            }
        }
    }

    
    //Octant 7 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant7 == 1 )
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy--;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy--;
            }
        }
    }
    
    
    //Octant 8 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant8 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx--;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx--;
            }
        }
    }
    
    //Fill
    
    if( iFilled )
    {
        x = 0;
        y = iRadius;
        int diff = iRadius - 1;

        while (y >= x)
        {
            if( diff >= ( 2 * x ) )
            {
                DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x + y, iCenter.y - x ), FPoint( iCenter.x + y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - y, iCenter.y - x ), FPoint( iCenter.x - y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x + y, iCenter.y - x ), FPoint( iCenter.x + y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - y, iCenter.y - x ), FPoint( iCenter.x - y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
}



//You can draw concentric circles with this one. But multiple pixel outline at some part of the circle -> messier result
static void DrawCircleAndresAA( TBlock< _SH >*            iBlock
                                , const FPoint             iCenter
                                , const int                iRadius
                                , const CColor&            iColor
                                , const bool               iFilled
                                , const FRect&                      iClippingRect
                                , const FPerformanceOptions&        iPerformanceOptions
                                , bool                     iCallInvalidCB )
{
    //Clipping -----
    int x = 0;
    int y = iRadius; //We start from the top of the circle for the first octant
    
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int shift45 = std::sin( FMath::DegToRad( 45 ) ) * iRadius;
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iRadius );
    FPoint point45 = FPoint( iCenter.x + shift45, iCenter.y - shift45 );
    FPoint point90 = FPoint( iCenter.x + iRadius, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45, iCenter.y + shift45 );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iRadius );
    FPoint point225 = FPoint( iCenter.x - shift45, iCenter.y + shift45 );
    FPoint point270 = FPoint( iCenter.x - iRadius, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45, iCenter.y - shift45 );
    
    FRect rectOctant1 = FRect( point0.x, point0.y - 1, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x + 1, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x + 1, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y + 1, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y + 1, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x - 1, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x - 1, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y - 1, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;
    
    
    //Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );

    auto MaxAlpha = val.GetAlpha();

    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val ); // 0° to 45°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        //Left and top clip
        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx++;
            }
        }
        
        // Base for drawing the aliased part of the circle on the sides on the clipping rect
        /*if( xx == rectOctant1Clipped.x || yy == rectOctant1Clipped.y ) //We draw only the bottom line of AA until we draw the next y
        {
            while ( 1 )
            {
                float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
                
                val.SetAlpha( MaxAlpha * alphaTop );
                
                iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
                
                if( diff >= ( 2 * x ) )
                {
                    diff -= ( 2 * x + 1 );
                    x++; xx++;
                }
                else if ( diff < ( 2 * ( iRadius - y ) ) )
                {
                    diff += ( 2 * y - 1 );
                    y--; yy++;
                    break;
                }
                else
                {
                    diff += (2 * ( y - x - 1 ) );
                    y--; yy++;
                    x++; xx++;
                    break;
                }
            }
        }*/
        
        //Right and bottom clip
        while (xx <= limitX && yy <= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val ); // 0° to 45°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx++;
            }
        }
    }
    
    //Octant 2 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant2 == 1)
    {
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val ); // 90° to 45°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
        
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        //Right and bottom clip
        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy--;
            }
        }
        
        //Top and left clip
        while (xx >= limitX && yy >= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val ); // 90° to 45°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy--;
            }
        }
    }
    
    //Octant 3 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant3 == 1)
    {
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val ); // 90° to 135°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
        
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        //Right and top clip
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy++;
            }
        }
        
        //Top and left clip
        while (xx >= limitX && yy <= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val ); // 90° to 135°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy++;
            }
        }
    }
    
    //Octant 4 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant4 == 1)
    {
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val ); // 180° to 135°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
        
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        //Right and top clip
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx++;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val ); // 180° to 135°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx++;
            }
        }
    }
    
    //Octant 5 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant5 == 1)
    {
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val ); // 180° to 225°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
        
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        //Left and bottom clip
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx--;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy >= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val ); // 180° to 225°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx--;
            }
        }
    }
    
    
    //Octant 6 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant6 == 1)
    {
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );  // 270° to 225°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
        
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        //Left and bottom clip
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy++;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy <= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );  // 270° to 225°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy++;
            }
        }
    }
    
    
    //Octant 7------
    x = 0;
    y = iRadius;
    if( drawRectOctant7 == 1 )
    {
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val ); // 270° to 315°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
        
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        //Left and bottom clip
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy--;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val ); // 270° to 315°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy--;
            }
        }
    }
    
    
    //Octant 8------
    x = 0;
    y = iRadius;
    if( drawRectOctant8 == 1)
    {
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val ); // 0° to 315°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
        
        int diff = iRadius - 1;
        int errMax = 2 * (iRadius - 1);
        int errMin = 0;
        
        //Left and bottom clip
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx--;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val ); // 0° to 315°
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx--;
            }
        }
    }

    //Filled
    if( iFilled )
    {
        x = 0;
        y = iRadius;
        int diff = iRadius - 1;

        while (y >= x)
        {
            if( diff >= ( 2 * x ) )
            {
                DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x + y, iCenter.y - x ), FPoint( iCenter.x + y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - y, iCenter.y - x ), FPoint( iCenter.x - y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x + y, iCenter.y - x ), FPoint( iCenter.x + y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - y, iCenter.y - x ), FPoint( iCenter.x - y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
}


//Don't draw concentric circles with this one. But 1 pixel outline all around the circle -> Cleaner result
static void DrawCircleBresenham(  TBlock< _SH >*           iBlock
                                , const FPoint             iCenter
                                , const int                iRadius
                                , const CColor&            iColor
                                , const bool               iFilled
                                , const FRect&                      iClippingRect
                                , const FPerformanceOptions&        iPerformanceOptions
                                , bool                     iCallInvalidCB )
{
    //Clipping -----
    int x = 0;
    int y = iRadius;
    
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int shift45 = std::sin( FMath::DegToRad( 45 ) ) * iRadius;
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iRadius );
    FPoint point45 = FPoint( iCenter.x + shift45, iCenter.y - shift45 );
    FPoint point90 = FPoint( iCenter.x + iRadius, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45, iCenter.y + shift45 );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iRadius );
    FPoint point225 = FPoint( iCenter.x - shift45, iCenter.y + shift45 );
    FPoint point270 = FPoint( iCenter.x - iRadius, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45, iCenter.y - shift45 );
    
    FRect rectOctant1 = FRect( point0.x, point0.y, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;
    
    // Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );
    
    
    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while( x <= y )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        int diff = 5 - 4 * iRadius;
        
        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( diff > 0 )
            {
                y--; yy++;
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
        
        while (xx <= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            if( diff > 0 )
            {
                y--; yy++;
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
    }
    
    
    //Octant 2 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant2 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while( x <= y )
        {
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
    
        int diff = 5 - 4 * iRadius;
        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( diff > 0 )
            {
                y--; xx--;
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
        while (xx >= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( diff > 0 )
            {
                y--; xx--;
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 3 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant3 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while( x <= y )
        {
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
    
        int diff = 5 - 4 * iRadius;
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( diff > 0 )
            {
                y--; xx--;
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
        while (xx >= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( diff > 0 )
            {
                y--; xx--;
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 4 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant4 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while( x <= y )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
    
        int diff = 5 - 4 * iRadius;
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( diff > 0 )
            {
                y--; yy--;
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
        while (xx <= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( diff > 0 )
            {
                y--; yy--;
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 5 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant5 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while( x <= y )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
    
        int diff = 5 - 4 * iRadius;
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( diff > 0 )
            {
                y--; yy--;
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
        while (xx >= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( diff > 0 )
            {
                y--; yy--;
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
    }
    
    
    //Octant 6 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant6 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while( x <= y )
        {
        iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
    
        int diff = 5 - 4 * iRadius;
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( diff > 0 )
            {
                y--; xx++;
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
        while (xx <= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( diff > 0 )
            {
                y--; xx++;
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
    }
    
    
    //Octant 7 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant7 == 1 )
    {
        int diff = 5 - 4 * iRadius;
        
        while( x <= y )
        {
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
    
        int diff = 5 - 4 * iRadius;
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( diff > 0 )
            {
                y--; xx++;
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
        while (xx <= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( diff > 0 )
            {
                y--; xx++;
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 8 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant8 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while( x <= y )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
    
        int diff = 5 - 4 * iRadius;
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( diff > 0 )
            {
                y--; yy++;
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
        while (xx >= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( diff > 0 )
            {
                y--; yy++;
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
    }

    //Fill
    
    if( iFilled )
    {
        x = 0;
        y = iRadius;
        int diff = 5 - 4 * iRadius;
    
        while( x <= y )
        {
            DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            DrawLine( iBlock, FPoint( iCenter.x + y, iCenter.y - x ), FPoint( iCenter.x + y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            DrawLine( iBlock, FPoint( iCenter.x - y, iCenter.y - x ), FPoint( iCenter.x - y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
}

// ---

//Don't draw concentric circles with this one. But 1 pixel outline all around the circle -> Cleaner result
static void DrawCircleBresenhamAA(  TBlock< _SH >*           iBlock
                                    , const FPoint             iCenter
                                    , const int                iRadius
                                    , const CColor&            iColor
                                    , const bool               iFilled
                                    , const FRect&                      iClippingRect
                                    , const FPerformanceOptions&        iPerformanceOptions
                                    , bool                     iCallInvalidCB )
{
    //Clipping -----
    int x = 0;
    int y = iRadius;
    
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int shift45 = std::sin( FMath::DegToRad( 45 ) ) * iRadius;
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iRadius );
    FPoint point45 = FPoint( iCenter.x + shift45, iCenter.y - shift45 );
    FPoint point90 = FPoint( iCenter.x + iRadius, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45, iCenter.y + shift45 );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iRadius );
    FPoint point225 = FPoint( iCenter.x - shift45, iCenter.y + shift45 );
    FPoint point270 = FPoint( iCenter.x - iRadius, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45, iCenter.y - shift45 );
    
    FRect rectOctant1 = FRect( point0.x, point0.y - 1, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x + 1, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x + 1, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y + 1, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y + 1, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x - 1, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x - 1, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y - 1, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;
    
    // Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );
    
    auto MaxAlpha = val.GetAlpha();

    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
    
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val ); // 0° to 45°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;

        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
        
        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( diff > 0 )
            {
                y--; yy++;
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
        
        while (xx <= limitX && yy <= limitY )
        {
           if( diff > 0 )
            {
                y--; yy++;
            }

            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val ); // 0° to 45°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
    }

    //Octant 2 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant2 == 1)
    {
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
    
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val ); // 90° to 45°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;

        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
        
        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( diff > 0 )
            {
                y--; xx--;
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
        
        while (xx >= limitX && yy >= limitY )
        {
           if( diff > 0 )
            {
                y--; xx--;
            }

            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val ); // 90° to 45°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 3 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant3 == 1)
    {
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
    
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val ); // 90° to 135°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;

        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
        
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( diff > 0 )
            {
                y--; xx--;
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
        
        while (xx >= limitX && yy <= limitY )
        {
           if( diff > 0 )
            {
                y--; xx--;
            }

            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val ); // 90° to 135°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 4 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant4 == 1)
    {
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
    
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val ); // 180° to 135°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;

        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
        
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( diff > 0 )
            {
                y--; yy--;
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
        
        while (xx <= limitX && yy >= limitY )
        {
           if( diff > 0 )
            {
                y--; yy--;
            }

            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val ); // 180° to 135°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 5 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant5 == 1)
    {
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
    
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val ); // 180° to 225°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
        
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
        
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( diff > 0 )
            {
                y--; yy--;
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
        
        while (xx >= limitX && yy >= limitY )
        {
           if( diff > 0 )
            {
                y--; yy--;
            }

            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val ); // 180° to 225°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 6 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant6 == 1)
    {
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
    
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );  // 270° to 225°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
        
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
        
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( diff > 0 )
            {
                y--; xx++;
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
        
        while (xx <= limitX && yy <= limitY )
        {
           if( diff > 0 )
            {
                y--; xx++;
            }

            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );  // 270° to 225°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 7 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant7 == 1)
    {
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
    
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val ); // 270° to 315°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
        
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
        
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( diff > 0 )
            {
                y--; xx++;
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
        
        while (xx <= limitX && yy >= limitY )
        {
           if( diff > 0 )
            {
                y--; xx++;
            }

            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val ); // 270° to 315°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 8 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant8 == 1)
    {
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
    
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val ); // 0° to 315°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
        
        int diff = 5 - 4 * iRadius;
        int errMax = 2 * ( 4 * iRadius - 5 );
        int errMin = 0;
        
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( diff > 0 )
            {
                y--; yy++;
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
        
        while (xx >= limitX && yy <= limitY )
        {
           if( diff > 0 )
            {
                y--; yy++;
            }

            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax
            
            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val ); // 0° to 315°
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Filled
    if( iFilled )
    {
        x = 0;
        y = iRadius;
        int diff = 5 - 4 * iRadius;
        while( x <= y )
        {
            if( diff > 0 )
            {
                y--;
            }
            
            DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            DrawLine( iBlock, FPoint( iCenter.x + y, iCenter.y - x ), FPoint( iCenter.x + y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            DrawLine( iBlock, FPoint( iCenter.x - y, iCenter.y - x ), FPoint( iCenter.x - y, iCenter.y + x ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
}

    // ---


//You can draw concentric arcs with this one. But multiple pixel outline at some part of the arc -> messier result
static void DrawArcAndres(  TBlock< _SH >*            iBlock
                            , const FPoint              iCenter
                            , const int                 iRadius
                            , const int                 iStartDegree
                            , const int                 iEndDegree
                            , const CColor&             iColor
                            , const FRect&                      iClippingRect
                            , const FPerformanceOptions&         iPerformanceOptions
                            , bool                      iCallInvalidCB )
{
    if( iRadius == 0 )
        return;
    
    //Clipping -----
    int x = 0;
    int y = iRadius; //We start from the top of the circle for the first octant

    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int shift45 = std::sin( FMath::DegToRad( 45 ) ) * iRadius;
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iRadius );
    FPoint point45 = FPoint( iCenter.x + shift45, iCenter.y - shift45 );
    FPoint point90 = FPoint( iCenter.x + iRadius, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45, iCenter.y + shift45 );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iRadius );
    FPoint point225 = FPoint( iCenter.x - shift45, iCenter.y + shift45 );
    FPoint point270 = FPoint( iCenter.x - iRadius, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45, iCenter.y - shift45 );
    
    FRect rectOctant1 = FRect( point0.x, point0.y, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;

    //Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );

    int sizeAngleToDraw = (iEndDegree - iStartDegree + 360) % 360; //Positive modulo
    int currentAngle = iStartDegree;

    int octantsToDraw[8] = {0, 0, 0, 0, 0 ,0 ,0 ,0 }; // 0: Don't draw the octant. 1: draw fully the octant. 2: draw part of the octant
    int directionToDraw[8][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} ,{0, 0} ,{0, 0} ,{0, 0} }; // 1 clockwise, -1 anti-clockwise, 2 both 0 irrelevant, second entry is angle to draw on octant
    
    if( sizeAngleToDraw < 45 )
    {
        octantsToDraw[ currentAngle / 45 ] = 2;
        directionToDraw[ currentAngle / 45 ][0] = 2;
        directionToDraw[ currentAngle / 45 ][1] = sizeAngleToDraw;
    }
    else if( currentAngle % 45 == 0 )
        octantsToDraw[ currentAngle / 45 ] = 1;
    else
    {
        octantsToDraw[ currentAngle / 45 ] = 2;
        directionToDraw[ currentAngle / 45 ][0] = -1;
        directionToDraw[ currentAngle / 45 ][1] = 45 - (currentAngle % 45);
    }

    sizeAngleToDraw -= ( 45 - (iStartDegree % 45) );

    while (sizeAngleToDraw >= 45 )
    {
        currentAngle = ( currentAngle + 45 ) % 360;
        octantsToDraw[ currentAngle / 45] = 1;
        sizeAngleToDraw -= 45;
    }

    if( sizeAngleToDraw > 0 )
    {
        currentAngle = ( currentAngle + 45 ) % 360;
        octantsToDraw[ currentAngle / 45] = 2;
        directionToDraw[ currentAngle / 45 ][0] = 1;
        directionToDraw[ currentAngle / 45 ][1] = sizeAngleToDraw;
    }

    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        int diff = iRadius - 1;
        
        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        int diff = iRadius - 1;

        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx++;
            }
        }
        
        while (xx <= limitX && yy <= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx++;
            }
        }
    }
    
    //Octant 2 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant2 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
            }

            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
        
        int diff = iRadius - 1;

        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy--;
            }
        }
        
        while (xx >= limitX && yy >= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy--;
            }
        }
    }
    
    //Octant 3 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant3 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
        
        int diff = iRadius - 1;

        //Right and top clip
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy++;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy++;
            }
        }
    }

    //Octant 4 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant4 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx++;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx++;
            }
        }
    }
    
    
    //Octant 5 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant5 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx--;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy >= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx--;
            }
        }
    }
    
    
    //Octant 6 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant6 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy++;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy <= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy++;
            }
        }
    }

    
    //Octant 7 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant7 == 1 )
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy--;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy--;
            }
        }
    }
    
    
    //Octant 8 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant8 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx--;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx--;
            }
        }
    }
}


//You can draw concentric arcs with this one. But multiple pixel outline at some part of the arc -> messier result
static void DrawArcAndresAA(  TBlock< _SH >*            iBlock
                            , const FPoint              iCenter
                            , const int                 iRadius
                            , const int                 iStartDegree
                            , const int                 iEndDegree
                            , const CColor&             iColor
                            , const FRect&                      iClippingRect
                            , const FPerformanceOptions&         iPerformanceOptions
                            , bool                      iCallInvalidCB )
{
    if( iRadius == 0 )
        return;
    
    //Clipping -----
    int x = 0;
    int y = iRadius; //We start from the top of the circle for the first octant
    int errMax = 2 * (iRadius - 1);
    int errMin = 0;
    
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int shift45 = std::sin( FMath::DegToRad( 45 ) ) * iRadius;
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iRadius );
    FPoint point45 = FPoint( iCenter.x + shift45, iCenter.y - shift45 );
    FPoint point90 = FPoint( iCenter.x + iRadius, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45, iCenter.y + shift45 );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iRadius );
    FPoint point225 = FPoint( iCenter.x - shift45, iCenter.y + shift45 );
    FPoint point270 = FPoint( iCenter.x - iRadius, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45, iCenter.y - shift45 );
    
    FRect rectOctant1 = FRect( point0.x, point0.y, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;

    //Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );
    
    auto MaxAlpha = val.GetAlpha();

    int sizeAngleToDraw = (iEndDegree - iStartDegree + 360) % 360; //Positive modulo
    int currentAngle = iStartDegree;

    int octantsToDraw[8] = {0, 0, 0, 0, 0 ,0 ,0 ,0 }; // 0: Don't draw the octant. 1: draw fully the octant. 2: draw part of the octant
    int directionToDraw[8][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} ,{0, 0} ,{0, 0} ,{0, 0} }; // 1 clockwise, -1 anti-clockwise, 2 both 0 irrelevant, second entry is angle to draw on octant
    
    if( sizeAngleToDraw < 45 )
    {
        octantsToDraw[ currentAngle / 45 ] = 2;
        directionToDraw[ currentAngle / 45 ][0] = 2;
        directionToDraw[ currentAngle / 45 ][1] = sizeAngleToDraw;
    }
    else if( currentAngle % 45 == 0 )
        octantsToDraw[ currentAngle / 45 ] = 1;
    else
    {
        octantsToDraw[ currentAngle / 45 ] = 2;
        directionToDraw[ currentAngle / 45 ][0] = -1;
        directionToDraw[ currentAngle / 45 ][1] = 45 - (currentAngle % 45);
    }

    sizeAngleToDraw -= ( 45 - (iStartDegree % 45) );

    while (sizeAngleToDraw >= 45 )
    {
        currentAngle = ( currentAngle + 45 ) % 360;
        octantsToDraw[ currentAngle / 45] = 1;
        sizeAngleToDraw -= 45;
    }

    if( sizeAngleToDraw > 0 )
    {
        currentAngle = ( currentAngle + 45 ) % 360;
        octantsToDraw[ currentAngle / 45] = 2;
        directionToDraw[ currentAngle / 45 ][0] = 1;
        directionToDraw[ currentAngle / 45 ][1] = sizeAngleToDraw;
    }

    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        int diff = iRadius - 1;
        
        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val ); // 0° to 45°

            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        int diff = iRadius - 1;

        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx++;
            }
        }
        
        while (xx <= limitX && yy <= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val ); // 0° to 45°

            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx++;
            }
        }
    }
    
    //Octant 2 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant2 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
        
        int diff = iRadius - 1;

        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy--;
            }
        }
        
        while (xx >= limitX && yy >= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy--;
            }
        }
    }
    
    //Octant 3 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant3 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
            }

            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
        
        int diff = iRadius - 1;

        //Right and top clip
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy++;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx--;
                x++; yy++;
            }
        }
    }

    //Octant 4 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant4 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
            }
        
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx++;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx++;
            }
        }
    }
    
    
    //Octant 5 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant5 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            }

            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
            }

            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx--;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy >= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            }

            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy--;
                x++; xx--;
            }
        }
    }
    
    
    //Octant 6 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant6 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy++;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy <= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy++;
            }
        }
    }

    
    //Octant 7 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant7 == 1 )
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy--;
            }
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; yy--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; xx++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; xx++;
                x++; yy--;
            }
        }
    }
    
    
    //Octant 8 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant8 == 1)
    {
        int diff = iRadius - 1;

        while (y >= x)
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--;
                x++;
            }
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
        
        int diff = iRadius - 1;

        //Left and bottom clip
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx--;
            }
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            float alphaTop = FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
            }
            
            if( diff >= ( 2 * x ) )
            {
                diff -= ( 2 * x + 1 );
                x++; xx--;
            }
            else if ( diff < ( 2 * ( iRadius - y ) ) )
            {
                diff += ( 2 * y - 1 );
                y--; yy++;
            }
            else
            {
                diff += (2 * ( y - x - 1 ) );
                y--; yy++;
                x++; xx--;
            }
        }
    }
}


// ---

//Don't draw concentric arcs with this one. But 1 pixel outline all around the arc -> Cleaner result
static void DrawArcBresenham( TBlock< _SH >*            iBlock
                            , const FPoint              iCenter
                            , const int                 iRadius
                            , const int                 iStartDegree
                            , const int                 iEndDegree
                            , const CColor&             iColor
                            , const FRect&                      iClippingRect
                            , const FPerformanceOptions&         iPerformanceOptions
                            , bool                      iCallInvalidCB )
{
    if( iRadius == 0 )
        return;
    
    //Clipping -----
    int x = 0;
    int y = iRadius; //We start from the top of the circle for the first octant

    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int shift45 = std::sin( FMath::DegToRad( 45 ) ) * iRadius;
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iRadius );
    FPoint point45 = FPoint( iCenter.x + shift45, iCenter.y - shift45 );
    FPoint point90 = FPoint( iCenter.x + iRadius, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45, iCenter.y + shift45 );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iRadius );
    FPoint point225 = FPoint( iCenter.x - shift45, iCenter.y + shift45 );
    FPoint point270 = FPoint( iCenter.x - iRadius, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45, iCenter.y - shift45 );
    
    FRect rectOctant1 = FRect( point0.x, point0.y, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;

    //Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );

    int sizeAngleToDraw = (iEndDegree - iStartDegree + 360) % 360; //Positive modulo
    int currentAngle = iStartDegree;

    int octantsToDraw[8] = {0, 0, 0, 0, 0 ,0 ,0 ,0 }; // 0: Don't draw the octant. 1: draw fully the octant. 2: draw part of the octant
    int directionToDraw[8][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} ,{0, 0} ,{0, 0} ,{0, 0} }; // 1 clockwise, -1 anti-clockwise, 2 both 0 irrelevant, second entry is angle to draw on octant
    
    if( sizeAngleToDraw < 45 )
    {
        octantsToDraw[ currentAngle / 45 ] = 2;
        directionToDraw[ currentAngle / 45 ][0] = 2;
        directionToDraw[ currentAngle / 45 ][1] = sizeAngleToDraw;
    }
    else if( currentAngle % 45 == 0 )
        octantsToDraw[ currentAngle / 45 ] = 1;
    else
    {
        octantsToDraw[ currentAngle / 45 ] = 2;
        directionToDraw[ currentAngle / 45 ][0] = -1;
        directionToDraw[ currentAngle / 45 ][1] = 45 - (currentAngle % 45);
    }

    sizeAngleToDraw -= ( 45 - (iStartDegree % 45) );

    while (sizeAngleToDraw >= 45 )
    {
        currentAngle = ( currentAngle + 45 ) % 360;
        octantsToDraw[ currentAngle / 45] = 1;
        sizeAngleToDraw -= 45;
    }

    if( sizeAngleToDraw > 0 )
    {
        currentAngle = ( currentAngle + 45 ) % 360;
        octantsToDraw[ currentAngle / 45] = 2;
        directionToDraw[ currentAngle / 45 ][0] = 1;
        directionToDraw[ currentAngle / 45 ][1] = sizeAngleToDraw;
    }
    
    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            }
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( diff > 0 )
            {
                y--; yy++;
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
        
        while (xx <= limitX && yy <= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            }
            
            if( diff > 0 )
            {
                y--; yy++;
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 2 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant2 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
            }
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( diff > 0 )
            {
                y--; xx--;
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
        
        while (xx >= limitX && yy >= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
            }
            
            if( diff > 0 )
            {
                y--; xx--;
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 3 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant3 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
            }
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
        
        int diff = 5 - 4 * iRadius;

        //Right and top clip
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( diff > 0 )
            {
                y--;xx--;
                diff = diff - 8 * y;
            }
            x++;yy++;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
            }
            
            if( diff > 0 )
            {
                y--;xx--;
                diff = diff - 8 * y;
            }
            x++;yy++;
            diff = diff + 8 * x + 4;
        }
    }

    //Octant 4 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant4 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            }
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( diff > 0 )
            {
                y--;yy--;
                diff = diff - 8 * y;
            }
            x++;xx++;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            }
            
            if( diff > 0 )
            {
                y--;yy--;
                diff = diff - 8 * y;
            }
            x++;xx++;
            diff = diff + 8 * x + 4;
        }
    }

    //Octant 5 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant5 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            }
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( diff > 0 )
            {
                y--;yy--;
                diff = diff - 8 * y;
            }
            x++;xx--;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy >= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            }
            
            if( diff > 0 )
            {
                y--;yy--;
                diff = diff - 8 * y;
            }
            x++;xx--;
            diff = diff + 8 * x + 4;
        }
    }
    
   //Octant 6 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant6 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
            }
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( diff > 0 )
            {
                y--;xx++;
                diff = diff - 8 * y;
            }
            x++;yy++;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy <= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
            }
            
            if( diff > 0 )
            {
                y--;xx++;
                diff = diff - 8 * y;
            }
            x++;yy++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 7 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant7 == 1 )
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
            }
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( diff > 0 )
            {
                y--;xx++;
                diff = diff - 8 * y;
            }
            x++;yy--;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
            }
            
            if( diff > 0 )
            {
                y--;xx++;
                diff = diff - 8 * y;
            }
            x++;yy--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 8 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant8 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            }
            
            if( diff > 0 )
            {
                y--;
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( diff > 0 )
            {
                y--;yy++;
                diff = diff - 8 * y;
            }
            x++;xx--;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            }
            
            if( diff > 0 )
            {
                y--;yy++;
                diff = diff - 8 * y;
            }
            x++;xx--;
            diff = diff + 8 * x + 4;
        }
    }
}

//Don't draw concentric arcs with this one. But 1 pixel outline all around the arc -> Cleaner result
static void DrawArcBresenhamAA( TBlock< _SH >*            iBlock
                            , const FPoint              iCenter
                            , const int                 iRadius
                            , const int                 iStartDegree
                            , const int                 iEndDegree
                            , const CColor&             iColor
                            , const FRect&                      iClippingRect
                            , const FPerformanceOptions&         iPerformanceOptions
                            , bool                      iCallInvalidCB )
{
    if( iRadius == 0 )
        return;
    
    //Clipping -----
    int x = 0;
    int y = iRadius; //We start from the top of the circle for the first octant
    int errMax = 2 * ( 4 * iRadius - 5 );
    int errMin = 0;
    
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    int shift45 = std::sin( FMath::DegToRad( 45 ) ) * iRadius;
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iRadius );
    FPoint point45 = FPoint( iCenter.x + shift45, iCenter.y - shift45 );
    FPoint point90 = FPoint( iCenter.x + iRadius, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45, iCenter.y + shift45 );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iRadius );
    FPoint point225 = FPoint( iCenter.x - shift45, iCenter.y + shift45 );
    FPoint point270 = FPoint( iCenter.x - iRadius, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45, iCenter.y - shift45 );
    
    FRect rectOctant1 = FRect( point0.x, point0.y, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;

    //Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );
    
    auto MaxAlpha = val.GetAlpha();

    int sizeAngleToDraw = (iEndDegree - iStartDegree + 360) % 360; //Positive modulo
    int currentAngle = iStartDegree;

    int octantsToDraw[8] = {0, 0, 0, 0, 0 ,0 ,0 ,0 }; // 0: Don't draw the octant. 1: draw fully the octant. 2: draw part of the octant
    int directionToDraw[8][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} ,{0, 0} ,{0, 0} ,{0, 0} }; // 1 clockwise, -1 anti-clockwise, 2 both 0 irrelevant, second entry is angle to draw on octant
    
    if( sizeAngleToDraw < 45 )
    {
        octantsToDraw[ currentAngle / 45 ] = 2;
        directionToDraw[ currentAngle / 45 ][0] = 2;
        directionToDraw[ currentAngle / 45 ][1] = sizeAngleToDraw;
    }
    else if( currentAngle % 45 == 0 )
        octantsToDraw[ currentAngle / 45 ] = 1;
    else
    {
        octantsToDraw[ currentAngle / 45 ] = 2;
        directionToDraw[ currentAngle / 45 ][0] = -1;
        directionToDraw[ currentAngle / 45 ][1] = 45 - (currentAngle % 45);
    }

    sizeAngleToDraw -= ( 45 - (iStartDegree % 45) );

    while (sizeAngleToDraw >= 45 )
    {
        currentAngle = ( currentAngle + 45 ) % 360;
        octantsToDraw[ currentAngle / 45] = 1;
        sizeAngleToDraw -= 45;
    }

    if( sizeAngleToDraw > 0 )
    {
        currentAngle = ( currentAngle + 45 ) % 360;
        octantsToDraw[ currentAngle / 45] = 2;
        directionToDraw[ currentAngle / 45 ][0] = 1;
        directionToDraw[ currentAngle / 45 ][1] = sizeAngleToDraw;
    }
    
    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        int diff = 5 - 4 * iRadius;
        
        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val ); // 0° to 45°

            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( diff > 0 )
            {
                y--; yy++;
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
        
        while (xx <= limitX && yy <= limitY )
        {
            if( diff > 0 )
            {
                y--; yy++;
            }
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0° to 45°
            
            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[0] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val ); // 0° to 45°

            // Complex cases
            if( octantsToDraw[0] == 2)
            {
                if ( directionToDraw[0][0] == 1 && currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
                else if ( directionToDraw[0][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[0][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
                else if ( directionToDraw[0][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - 1, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 2 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant2 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( diff > 0 )
            {
                y--; xx--;
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
        
        while (xx >= limitX && yy >= limitY )
        {
            if( diff > 0 )
            {
                y--; xx--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y - x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[1] == 1 ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val ); // 90° to 45°
            
            if( octantsToDraw[1] == 2)
            {
                if ( directionToDraw[1][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == -1 && currentAngleOnFirstOctant < directionToDraw[1][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
                else if ( directionToDraw[1][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y - x, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 3 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant3 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
            }

            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
        
        int diff = 5 - 4 * iRadius;

        //Right and top clip
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( diff > 0 )
            {
                y--; xx--;
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            if( diff > 0 )
            {
                y--; xx--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );
            
            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y, iCenter.y + x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[2] == 1 ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val ); // 90° to 135°
            
            if( octantsToDraw[2] == 2)
            {
                if ( directionToDraw[2][0] == 1 && currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[2][1] ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
                else if ( directionToDraw[2][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + y + 1, iCenter.y + x, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 4 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant4 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
            }
        
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( diff > 0 )
            {
                y--; yy--;
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            if( diff > 0 )
            {
                y--; yy--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[3] == 1 ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val ); // 180° to 135°
            
            if( octantsToDraw[3] == 2)
            {
                if ( directionToDraw[3][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
                else if ( directionToDraw[3][0] == -1 && currentAngleOnFirstOctant < directionToDraw[3][1] ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
                else if ( directionToDraw[3][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + 1, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; xx++;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 5 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant5 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            }

            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
            }

            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( diff > 0 )
            {
                y--; yy--;
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy >= limitY )
        {
            if( diff > 0 )
            {
                y--; yy--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            }

            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[4] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val ); // 180° to 225°
            
            if( octantsToDraw[4] == 2)
            {
                if ( directionToDraw[4][0] == 1 && currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
                else if ( directionToDraw[4][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[4][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
                else if ( directionToDraw[4][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + 1, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
    }
    
    //Octant 6 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant6 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
           if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( diff > 0 )
            {
                y--; xx++;
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy <= limitY )
        {
            if( diff > 0 )
            {
                y--; xx++;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y + x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );
            
            if( octantsToDraw[5] == 1 ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );  // 270° to 225°
            
            if( octantsToDraw[5] == 2)
            {
                if ( directionToDraw[5][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == -1 && currentAngleOnFirstOctant < directionToDraw[5][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
                else if ( directionToDraw[5][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y + x, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; yy++;
            diff = diff + 8 * x + 4;
        }
    }
    
   //Octant 7 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant7 == 1 )
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( diff > 0 )
            {
                y--; xx++;
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx <= limitX && yy >= limitY )
        {
            if( diff > 0 )
            {
                y--; xx++;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y, iCenter.y - x, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[6] == 1 ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val ); // 270° to 315°
            
            if( octantsToDraw[6] == 2)
            {
                if ( directionToDraw[6][0] == 1 && currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == -1 && 45 - currentAngleOnFirstOctant < directionToDraw[6][1] ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
                else if ( directionToDraw[6][0] == 2 && currentAngleOnFirstOctant > (iStartDegree % 45) && currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - y - 1, iCenter.y - x, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; yy--;
            diff = diff + 8 * x + 4;
        }
    }

   //Octant 8 ------
    x = 0;
    y = iRadius;
    if( drawRectOctant8 == 1)
    {
        int diff = 5 - 4 * iRadius;

        while (y >= x)
        {
            if( diff > 0 )
            {
                y--;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++;
            diff = diff + 8 * x + 4;
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
        
        int diff = 5 - 4 * iRadius;

        //Left and bottom clip
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( diff > 0 )
            {
                y--; yy++;
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
        
        //Bottom and left clip
        while (xx >= limitX && yy <= limitY )
        {
            if( diff > 0 )
            {
                y--; yy++;
            }
            
            float alphaTop = 1 - FMath::Abs( ( float( diff - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            double currentAngleOnFirstOctant = -::ULIS::FMath::RadToDeg( std::acos( double(x) / double(iRadius) ) - (FMath::kPId / 2) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            }
            
            val.SetAlpha( MaxAlpha * (1 - alphaTop ) );

            if( octantsToDraw[7] == 1 ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val ); // 0° to 315°
            
            if( octantsToDraw[7] == 2)
            {
                if ( directionToDraw[7][0] == 1 && 45 - currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
                else if ( directionToDraw[7][0] == -1 && currentAngleOnFirstOctant < directionToDraw[7][1] ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
                else if ( directionToDraw[7][0] == 2 && 45 - currentAngleOnFirstOctant > (iStartDegree % 45) && 45 - currentAngleOnFirstOctant < ( iEndDegree % 45 ) ) iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - 1, val );
            }
            
            if( diff > 0 )
            {
                diff = diff - 8 * y;
            }
            x++; xx--;
            diff = diff + 8 * x + 4;
        }
    }
}


// ---

static void DrawEllipse(  TBlock< _SH >*           iBlock
                        , const FPoint             iCenter
                        , const int                iA
                        , const int                iB
                        , const CColor&            iColor
                        , const bool               iFilled
                        , const FRect&                      iClippingRect
                        , const FPerformanceOptions&        iPerformanceOptions
                        , bool                     iCallInvalidCB )
{
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }

    float angleTo45 = std::atan( - float(iB) / float(iA) );
    int shift45x = FMath::Abs( std::cos( angleTo45 ) * iA ) + 1;
    int shift45y = FMath::Abs( std::sin( angleTo45 ) * iB ) + 1;
    
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iB );
    FPoint point45 = FPoint( iCenter.x + shift45x, iCenter.y - shift45y );
    FPoint point90 = FPoint( iCenter.x + iA, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45x, iCenter.y + shift45y );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iB );
    FPoint point225 = FPoint( iCenter.x - shift45x, iCenter.y + shift45y );
    FPoint point270 = FPoint( iCenter.x - iA, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45x, iCenter.y - shift45y );
    
    FRect rectOctant1 = FRect( point0.x, point0.y, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;

    //Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );
    
    int a2 = iA * iA;
    int b2 = iB * iB;
    int fa2 = 4 * a2;
    int fb2 = 4 * b2;
    int x = 0;
    int y = iB;
    int sigma = 2*b2+a2*(1-2*iB);
    
    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        while ( b2 * x <= a2 * y )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );

            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
        

    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy++;
            }
            sigma += b2*(4 * x + 6);
            x++; xx++;
        }
            
        while (xx <= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
  
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy++;
            }
            sigma += b2*(4 * x + 6);
            x++; xx++;
        }
    }
    
    //Octant 2 ------
    x = iA;
    y = 0;
    sigma = 2*a2+b2*(1-2*iA);
    if( drawRectOctant2 == 1)
    {
        while ( a2 * y <= b2 * x )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
    
        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx--;
            }
            sigma += a2*(4 * y + 6);
            y++; yy--;
        }
        
        while (xx >= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx--;
            }
            sigma += a2*(4 * y + 6);
            y++; yy--;
        }
    }
    
    //Octant 3 ------
    x = iA;
    y = 0;
    sigma = 2*a2+b2*(1-2*iA);
    if( drawRectOctant3 == 1)
    {
        while ( a2 * y <= b2 * x )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
    
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx--;
            }
            sigma += a2*(4 * y + 6);
            y++; yy++;
        }
        
        while (xx >= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx--;
            }
            sigma += a2*(4 * y + 6);
            y++; yy++;
        }
    }
    
    //Octant 4 ------
    x = 0;
    y = iB;
    sigma = 2*b2+a2*(1-2*iB);
    if( drawRectOctant4 == 1)
    {
        while ( b2 * x <= a2 * y )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
    
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy--;
            }
            sigma += b2*(4 * x + 6);
            x++; xx++;
        }
        
        while (xx <= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy--;
            }
            sigma += b2*(4 * x + 6);
            x++; xx++;
        }
    }
    
    //Octant 5 ------
    x = 0;
    y = iB;
    sigma = 2*b2+a2*(1-2*iB);
    if( drawRectOctant5 == 1)
    {
        while ( b2 * x <= a2 * y )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
    
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy--;
            }
            sigma += b2*(4 * x + 6);
            x++; xx--;
        }
        
        while (xx >= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy--;
            }
            sigma += b2*(4 * x + 6);
            x++; xx--;
        }
    }
    
    //Octant 6 ------
    x = iA;
    y = 0;
    sigma = 2*a2+b2*(1-2*iA);
    if( drawRectOctant6 == 1)
    {
        while ( a2 * y <= b2 * x )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
    
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx++;
            }
            sigma += a2*(4 * y + 6);
            y++; yy++;
        }
        
        while (xx <= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx++;
            }
            sigma += a2*(4 * y + 6);
            y++; yy++;
        }
    }
    
    //Octant 7 ------
    x = iA;
    y = 0;
    sigma = 2*a2+b2*(1-2*iA);
    if( drawRectOctant7 == 1)
    {
        while ( a2 * y <= b2 * x )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
    
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx++;
            }
            sigma += a2*(4 * y + 6);
            y++; yy--;
        }
        
        while (xx <= limitX && yy >= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx++;
            }
            sigma += a2*(4 * y + 6);
            y++; yy--;
        }
    }
    
    
    //Octant 8 ------
    x = 0;
    y = iB;
    sigma = 2*b2+a2*(1-2*iB);
    if( drawRectOctant8 == 1)
    {
        while ( b2 * x <= a2 * y )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
  
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
    
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy++;
            }
            sigma += b2*(4 * x + 6);
            x++; xx--;
        }
        
        while (xx >= limitX && yy <= limitY )
        {
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
  
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy++;
            }
            sigma += b2*(4 * x + 6);
            x++; xx--;
        }
    }
    
    //Fill
    
    if( iFilled )
    {
        x = 0;
        y = iB;
        sigma = 2*b2+a2*(1-2*iB);
        
        while ( b2 * x <= a2 * y )
        {
            DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
        
        x = iA;
        y = 0;
        sigma = 2*a2+b2*(1-2*iA);
        
        while ( a2 * y <= b2 * x )
        {
            DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
}


static void DrawEllipseAA(  TBlock< _SH >*           iBlock
                        , const FPoint             iCenter
                        , const int                iA
                        , const int                iB
                        , const CColor&            iColor
                        , const bool               iFilled
                        , const FRect&                      iClippingRect
                        , const FPerformanceOptions&        iPerformanceOptions
                        , bool                     iCallInvalidCB )
{
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }

    float angleTo45 = std::atan( - float(iB) / float(iA) );
    int shift45x = FMath::Abs( std::cos( angleTo45 ) * iA );
    int shift45y = FMath::Abs( std::sin( angleTo45 ) * iB );
    
    FPoint point0 = FPoint( iCenter.x, iCenter.y - iB );
    FPoint point45 = FPoint( iCenter.x + shift45x, iCenter.y - shift45y );
    FPoint point90 = FPoint( iCenter.x + iA, iCenter.y );
    FPoint point135 = FPoint( iCenter.x + shift45x, iCenter.y + shift45y );
    FPoint point180 = FPoint( iCenter.x, iCenter.y + iB );
    FPoint point225 = FPoint( iCenter.x - shift45x, iCenter.y + shift45y );
    FPoint point270 = FPoint( iCenter.x - iA, iCenter.y );
    FPoint point315 = FPoint( iCenter.x - shift45x, iCenter.y - shift45y );
    
    FRect rectOctant1 = FRect( point0.x, point0.y - 1, point45.x - point0.x, point45.y - point0.y );
    FRect rectOctant2 = FRect( point45.x + 1, point45.y, point90.x - point45.x, point90.y - point45.y );
    FRect rectOctant3 = FRect( point135.x + 1, point90.y, point90.x - point135.x, point135.y - point90.y );
    FRect rectOctant4 = FRect( point180.x, point135.y + 1, point135.x - point180.x, point180.y - point135.y );
    FRect rectOctant5 = FRect( point225.x, point225.y + 1, point180.x - point225.x, point180.y - point225.y );
    FRect rectOctant6 = FRect( point270.x - 1, point270.y, point225.x - point270.x, point225.y - point270.y );
    FRect rectOctant7 = FRect( point270.x - 1, point315.y, point315.x - point270.x, point270.y - point315.y );
    FRect rectOctant8 = FRect( point315.x, point0.y - 1, point0.x - point315.x, point315.y - point0.y );
    
    FRect rectOctant1Clipped = rectOctant1 & clippingRect;
    FRect rectOctant2Clipped = rectOctant2 & clippingRect;
    FRect rectOctant3Clipped = rectOctant3 & clippingRect;
    FRect rectOctant4Clipped = rectOctant4 & clippingRect;
    FRect rectOctant5Clipped = rectOctant5 & clippingRect;
    FRect rectOctant6Clipped = rectOctant6 & clippingRect;
    FRect rectOctant7Clipped = rectOctant7 & clippingRect;
    FRect rectOctant8Clipped = rectOctant8 & clippingRect;

    // 0 : Don't draw the octant. 1: Draw fully the octant. 2: Partial draw of the octant (complex case)
    int drawRectOctant1 = rectOctant1Clipped == rectOctant1 ? 1 : rectOctant1Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant2 = rectOctant2Clipped == rectOctant2 ? 1 : rectOctant2Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant3 = rectOctant3Clipped == rectOctant3 ? 1 : rectOctant3Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant4 = rectOctant4Clipped == rectOctant4 ? 1 : rectOctant4Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant5 = rectOctant5Clipped == rectOctant5 ? 1 : rectOctant5Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant6 = rectOctant6Clipped == rectOctant6 ? 1 : rectOctant6Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant7 = rectOctant7Clipped == rectOctant7 ? 1 : rectOctant7Clipped.Area() != 0 ? 2 : 0;
    int drawRectOctant8 = rectOctant8Clipped == rectOctant8 ? 1 : rectOctant8Clipped.Area() != 0 ? 2 : 0;

    //Drawing ------
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );
    
    auto MaxAlpha = val.GetAlpha();

    int a2 = iA * iA;
    int b2 = iB * iB;
    int fa2 = 4 * a2;
    int fb2 = 4 * b2;
    int x = 0;
    int y = iB;
    int sigma = 2*b2+a2*(1-2*iB);

    int errMax = 0;
    int errMin =  2 * (2*b2+a2*(1-2*iB));

    //0° is on top and we turn clockwise
    //Octant 1 ------
    if( drawRectOctant1 == 1)
    {
        while ( b2 * x <= a2 * y )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            int step = sigma <= 0 ? 1 : -1;

            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0 to 45 degrees

            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );

            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - step, val ); // 0 to 45 degrees
            
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
    }
    else if( drawRectOctant1 == 2 )
    {
        int xx = rectOctant1.x;
        int yy = rectOctant1.y;
        int limitX = rectOctant1Clipped.w + rectOctant1Clipped.x;
        int limitY = rectOctant1Clipped.h + rectOctant1Clipped.y;
        
        while ( xx < rectOctant1Clipped.x || yy < rectOctant1Clipped.y )
        {
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy++;
            }
            sigma += b2*(4 * x + 6);
            x++; xx++;
        }
        
        while (xx <= limitX && yy <= limitY )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            int step = sigma <= 0 ? 1 : -1;

            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val ); // 0 to 45 degrees

            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );

            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y - step, val ); // 0 to 45 degrees
            
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy++;
            }
            sigma += b2*(4 * x + 6);
            x++; xx++;
        }
    }
    
    //Octant 2 ------
    x = iA;
    y = 0;
    sigma = 2*a2+b2*(1-2*iA);
    errMin =  2 * (2*a2+b2*(1-2*iA));
    if( drawRectOctant2 == 1)
    {
        while ( a2 * y <= b2 * x )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            int step = sigma < 0 ? 1 : -1;

            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x + step, iCenter.y - y, val ); // 90 to 45 degrees
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
    else if( drawRectOctant2 == 2 )
    {
        int xx = rectOctant2.x + rectOctant2.w;
        int yy = rectOctant2.y + rectOctant2.h;
        int limitX = rectOctant2Clipped.x;
        int limitY = rectOctant2Clipped.y;
    
        while ( xx > rectOctant2Clipped.x + rectOctant2Clipped.w || yy > rectOctant2Clipped.y + rectOctant2Clipped.h )
        {
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx--;
            }
            sigma += a2*(4 * y + 6);
            y++; yy--;
        }
        
        while (xx >= limitX && yy >= limitY )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            int step = sigma < 0 ? 1 : -1;

            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y - y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x + step, iCenter.y - y, val ); // 90 to 45 degrees
            
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx--;
            }
            sigma += a2*(4 * y + 6);
            y++; yy--;
        }
    }
    
    
    //Octant 3 ------
    x = iA;
    y = 0;
    sigma = 2*a2+b2*(1-2*iA);
    errMin =  2 * (2*a2+b2*(1-2*iA));
    if( drawRectOctant3 == 1)
    {
        while ( a2 * y <= b2 * x )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            int step = sigma < 0 ? 1 : -1;

            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x + step, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
    else if( drawRectOctant3 == 2 )
    {
        int xx = rectOctant3.x + rectOctant3.w;
        int yy = rectOctant3.y;
        int limitX = rectOctant3Clipped.x;
        int limitY = rectOctant3Clipped.y + rectOctant3Clipped.h;
    
        while ( xx > rectOctant3Clipped.x + rectOctant3Clipped.w || yy < rectOctant3Clipped.y )
        {
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx--;
            }
            sigma += a2*(4 * y + 6);
            y++; yy++;
        }
        
        while (xx >= limitX && yy <= limitY )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            int step = sigma < 0 ? 1 : -1;

            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x + step, iCenter.y + y, val );
            
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx--;
            }
            sigma += a2*(4 * y + 6);
            y++; yy++;
        }
    }
    
    
    //Octant 4 ------
    x = 0;
    y = iB;
    sigma = 2*b2+a2*(1-2*iB);
    errMin =  2 * (2*b2+a2*(1-2*iB));
    if( drawRectOctant4 == 1)
    {
        while ( b2 * x <= a2 * y )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            int step = sigma <= 0 ? 1 : -1;
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + step, val ); // 180 to 135 degrees
  
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
    }
    else if( drawRectOctant4 == 2 )
    {
        int xx = rectOctant4.x;
        int yy = rectOctant4.y + rectOctant4.h;
        int limitX = rectOctant4Clipped.x + rectOctant4Clipped.w;
        int limitY = rectOctant4Clipped.y;
    
        while ( xx < rectOctant4Clipped.x || yy > rectOctant4Clipped.y + rectOctant4Clipped.h )
        {
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy--;
            }
            sigma += b2*(4 * x + 6);
            x++; xx++;
        }
        
        while (xx <= limitX && yy >= limitY )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            int step = sigma <= 0 ? 1 : -1;
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x + x, iCenter.y + y + step, val ); // 180 to 135 degrees
            
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy--;
            }
            sigma += b2*(4 * x + 6);
            x++; xx++;
        }
    }
    
    //Octant 5 ------
    x = 0;
    y = iB;
    sigma = 2*b2+a2*(1-2*iB);
    errMin =  2 * (2*b2+a2*(1-2*iB));
    if( drawRectOctant5 == 1)
    {
        while ( b2 * x <= a2 * y )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            int step = sigma <= 0 ? 1 : -1;
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );

            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + step, val );

            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
    }
    else if( drawRectOctant5 == 2 )
    {
        int xx = rectOctant5.x + rectOctant5.w;
        int yy = rectOctant5.y + rectOctant5.h;
        int limitX = rectOctant5Clipped.x;
        int limitY = rectOctant5Clipped.y;
    
        while ( xx > rectOctant5Clipped.x + rectOctant5Clipped.w || yy > rectOctant5Clipped.y + rectOctant5Clipped.h )
        {
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy--;
            }
            sigma += b2*(4 * x + 6);
            x++; xx--;
        }
        
        while (xx >= limitX && yy >= limitY )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            int step = sigma <= 0 ? 1 : -1;
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );

            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y + step, val );
            
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy--;
            }
            sigma += b2*(4 * x + 6);
            x++; xx--;
        }
    }
    
    //Octant 6 ------
    x = iA;
    y = 0;
    sigma = 2*a2+b2*(1-2*iA);
    errMin =  2 * (2*a2+b2*(1-2*iA));
    if( drawRectOctant6 == 1)
    {
        while ( a2 * y <= b2 * x )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            int step = sigma < 0 ? 1 : -1;

            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x - step, iCenter.y + y, val );
  
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
    else if( drawRectOctant6 == 2 )
    {
        int xx = rectOctant6.x;
        int yy = rectOctant6.y;
        int limitX = rectOctant6Clipped.x + rectOctant6Clipped.w;
        int limitY = rectOctant6Clipped.y + rectOctant6Clipped.h;
    
        while ( xx < rectOctant6Clipped.x || yy < rectOctant6Clipped.y )
        {
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx++;
            }
            sigma += a2*(4 * y + 6);
            y++; yy++;
        }
        
        while (xx <= limitX && yy <= limitY )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            int step = sigma < 0 ? 1 : -1;

            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y + y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x - step, iCenter.y + y, val );
            
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx++;
            }
            sigma += a2*(4 * y + 6);
            y++; yy++;
        }
    }
    
    //Octant 7 ------
    x = iA;
    y = 0;
    sigma = 2*a2+b2*(1-2*iA);
    errMin =  2 * (2*a2+b2*(1-2*iA));
    if( drawRectOctant7 == 1)
    {
        while ( a2 * y <= b2 * x )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            int step = sigma < 0 ? 1 : -1;

            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x - step, iCenter.y - y, val );
            
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
    else if( drawRectOctant7 == 2 )
    {
        int xx = rectOctant7.x;
        int yy = rectOctant7.y + rectOctant7.h;
        int limitX = rectOctant7Clipped.x + rectOctant7Clipped.w;
        int limitY = rectOctant7Clipped.y;
    
        while ( xx < rectOctant7Clipped.x || yy > rectOctant7Clipped.y + rectOctant7Clipped.h )
        {
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx++;
            }
            sigma += a2*(4 * y + 6);
            y++; yy--;
        }
        
        while (xx <= limitX && yy >= limitY )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            int step = sigma < 0 ? 1 : -1;

            val.SetAlpha( MaxAlpha * alphaTop );
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );
            
            iBlock->SetPixelValue( iCenter.x - x - step, iCenter.y - y, val );
            
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--; xx++;
            }
            sigma += a2*(4 * y + 6);
            y++; yy--;
        }
    }
    
    //Octant 8 ------
    x = 0;
    y = iB;
    sigma = 2*b2+a2*(1-2*iB);
    errMin =  2 * (2*b2+a2*(1-2*iB));
    if( drawRectOctant8 == 1)
    {
        while ( b2 * x <= a2 * y )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            int step = sigma <= 0 ? 1 : -1;
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );

            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - step, val );
            
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
    }
    else if( drawRectOctant8 == 2 )
    {
        int xx = rectOctant8.x + rectOctant8.w;
        int yy = rectOctant8.y;
        int limitX = rectOctant8Clipped.x;
        int limitY = rectOctant8Clipped.y + rectOctant8Clipped.h;
    
        while ( xx > rectOctant8Clipped.x + rectOctant8Clipped.w || yy < rectOctant8Clipped.y )
        {
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy++;
            }
            sigma += b2*(4 * x + 6);
            x++; xx--;
        }
        
        while (xx >= limitX && yy <= limitY )
        {
            float alphaTop = 1 - FMath::Abs( ( float( sigma - errMax ) / float( errMin - errMax ) ) ); //Interpolation of slopedifferential between errMin and errMax

            val.SetAlpha( MaxAlpha * alphaTop );

            int step = sigma <= 0 ? 1 : -1;
            
            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y, val );
            
            val.SetAlpha( MaxAlpha * ( 1 - alphaTop ) );

            iBlock->SetPixelValue( iCenter.x - x, iCenter.y - y - step, val );
            
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--; yy++;
            }
            sigma += b2*(4 * x + 6);
            x++; xx--;
        }
    }
    
    //Fill
    
    if( iFilled )
    {
        x = 0;
        y = iB;
        sigma = 2*b2+a2*(1-2*iB);
        
        while ( b2 * x <= a2 * y )
        {
            int step = sigma <= 0 ? 1 : -1;

            if( step == 1 )
            {
                DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            }
            else //step = -1, we draw the aliasing on the inside of the ellipse, so we colorize one pixel less
            {
                DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y + 1 ), FPoint( iCenter.x + x, iCenter.y + y - 1 ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y + 1 ), FPoint( iCenter.x - x, iCenter.y + y - 1 ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            }
            
            if( sigma >= 0)
            {
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2*(4 * x + 6);
            x++;
        }
        
        x = iA;
        y = 0;
        sigma = 2*a2+b2*(1-2*iA);
        
        while ( a2 * y <= b2 * x )
        {
            int step = sigma < 0 ? 1 : -1;
            
            if( step == 1 )
            {
                DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y ), FPoint( iCenter.x + x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y ), FPoint( iCenter.x - x, iCenter.y + y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            }
            else //step = -1, we draw the aliasing on the inside of the ellipse, so we colorize one pixel less
            {
                DrawLine( iBlock, FPoint( iCenter.x + x, iCenter.y - y + 1 ), FPoint( iCenter.x + x, iCenter.y + y - 1 ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                DrawLine( iBlock, FPoint( iCenter.x - x, iCenter.y - y + 1 ), FPoint( iCenter.x - x, iCenter.y + y - 1 ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            }
            
            if( sigma >= 0)
            {
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2*(4 * y + 6);
            y++;
        }
    }
}


static void DrawRotatedEllipse(  TBlock< _SH >*           iBlock
                                , const FPoint             iCenter
                                , const int                iA
                                , const int                iB
                                , const int                iRotationDegrees
                                , const CColor&            iColor
                                , const bool               iFilled
                                , const FRect&                      iClippingRect
                                , const FPerformanceOptions&        iPerformanceOptions
                                , bool                     iCallInvalidCB )
{
    if( iRotationDegrees % 180 == 0 )
    {
        DrawEllipse( iBlock, iCenter, iA, iB, iColor, iFilled, iClippingRect, iPerformanceOptions, iCallInvalidCB ); //Don't bother to use the rotated ellipse algorithm if the ellipse is not rotated
        return;
    }
    if( iRotationDegrees % 90 == 0 )
    {
        DrawEllipse( iBlock, iCenter, iB, iA, iColor, iFilled, iClippingRect, iPerformanceOptions, iCallInvalidCB ); //Don't bother to use the rotated ellipse algorithm if the ellipse is not rotated
        return;
    }

                                                        //               x  y
    std::map< int, std::vector< int > > storagePoints; // storagePoints[x][0]  We have two points for each x on the ellipse: p1(x, y0), p2(x, y1)
                                                        //                 [1]
    storagePoints[0].push_back( iCenter.x );           // In order to only pass storagePoints in parameter to InternalDrawQuadRationalBezierSeg
    storagePoints[0].push_back( iCenter.y );           // we store the center (useful in this function) at index 0 (we'll know it's there)


    int a = iA;
    int b = iB;

    float dx = (long)iA*iA;
    float dy = (long)iB*iB;
    float s = std::sin( FMath::DegToRad( iRotationDegrees ) );
    float dz = (dx - dy) * s;
    dx = std::sqrt( dx - dz * s );
    dy = std::sqrt( dy + dz * s );
    a = dx + 0.5;
    b = dy + 0.5;
    dz = dz * a * b / (dx * dy );

    int x0 = iCenter.x - a;
    int y0 = iCenter.y - b;
    int x1 = iCenter.x + a;
    int y1 = iCenter.y + b;
    dz = (4 * dz * std::cos( FMath::DegToRad( iRotationDegrees ) ) );


    dx = x1 - x0;
    dy = y1 - y0;
    float w = dx * dy;
    if( w != 0.0 )
        w = ( w - dz ) / (w + w);

    if( w > 1 || w < 0 )
        return;

    dx = std::floor( dx * w + 0.5 );
    dy = std::floor( dy * w + 0.5 );

    InternalDrawQuadRationalBezierSeg( iBlock, x0, y0 + dy, x0, y0, x0 + dx, y0, 1 - w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, &storagePoints );
    InternalDrawQuadRationalBezierSeg( iBlock, x0, y0 + dy, x0, y1, x1 - dx, y1, w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, &storagePoints );
    InternalDrawQuadRationalBezierSeg( iBlock, x1, y1 - dy, x1, y1, x1 - dx, y1, 1 - w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, &storagePoints );
    InternalDrawQuadRationalBezierSeg( iBlock, x1, y1 - dy, x1, y0, x0 + dx, y0, w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, &storagePoints );

    if( iFilled ) //We fill the ellipse by drawing vertical lines
    {
        //We delete the values we stored for the center position (two times pop_front)
        storagePoints[0].erase( storagePoints[0].begin() );
        storagePoints[0].erase( storagePoints[0].begin() );

        for (std::map< int, std::vector< int > >::iterator it=storagePoints.begin(); it!=storagePoints.end(); ++it)
        {
            if( it->second.size() == 2 )
            {
                DrawLine( iBlock, FPoint( iCenter.x + it->first, iCenter.y + it->second[0] ), FPoint( iCenter.x + it->first, iCenter.y + it->second[1] ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            }
            if( it->second.size() > 2 ) // where we draw more than 2 pixels for a said y
            {
                int minY = it->second[0];
                int maxY = it->second[0];
                for( int i = 1; i < it->second.size(); i++)
                {
                    if( minY > it->second[i] )
                        minY = it->second[i];

                    if( maxY < it->second[i] )
                        maxY = it->second[i];
                }
                DrawLine( iBlock, FPoint( iCenter.x + it->first, iCenter.y + minY ), FPoint( iCenter.x + it->first, iCenter.y + maxY ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            }
        }
    }
}


static void DrawRotatedEllipseAA(  TBlock< _SH >*           iBlock
                                    , const FPoint             iCenter
                                    , const int                iA
                                    , const int                iB
                                    , const int                iRotationDegrees
                                    , const CColor&            iColor
                                    , const bool               iFilled
                                    , const FRect&                      iClippingRect
                                    , const FPerformanceOptions&        iPerformanceOptions
                                    , bool                     iCallInvalidCB )
{
    if( iRotationDegrees % 180 == 0 )
    {
        DrawEllipseAA( iBlock, iCenter, iA, iB, iColor, iFilled, iClippingRect, iPerformanceOptions, iCallInvalidCB ); //Don't bother to use the rotated ellipse algorithm if the ellipse is not rotated
        return;
    }
    if( iRotationDegrees % 90 == 0 )
    {
        DrawEllipseAA( iBlock, iCenter, iB, iA, iColor, iFilled, iClippingRect, iPerformanceOptions, iCallInvalidCB ); //Don't bother to use the rotated ellipse algorithm if the ellipse is not rotated
        return;
    }

    int a = iA;
    int b = iB;
    int rotation = ( ( iRotationDegrees % 180 ) + 180 ) % 180; //Positive modulo

    float dx = (long)a*a;
    float dy = (long)b*b;
    float s = std::sin( FMath::DegToRad( rotation + 90 ) );
    float dz = (dx - dy) * s;
    dx = std::sqrt( dx - dz * s );
    dy = std::sqrt( dy + dz * s );
    a = dx + 0.5;
    b = dy + 0.5;
    dz = dz * a * b / (dx * dy );

    int x0 = iCenter.x - a;
    int y0 = iCenter.y - b;
    int x1 = iCenter.x + a;
    int y1 = iCenter.y + b;
    dz = (4 * dz * std::cos( FMath::DegToRad( rotation + 90 ) ) );


    dx = x1 - x0;
    dy = y1 - y0;
    float w = dx * dy;
    if( w != 0.0 )
        w = ( w - dz ) / (w + w);

    if( w > 1 || w < 0 )
        return;

    dx = std::floor( dx * w + 0.5 );
    dy = std::floor( dy * w + 0.5 );



    if( !iFilled )
    {
        InternalDrawQuadRationalBezierSegAA( iBlock, x0, y0 + dy, x0, y0, x0 + dx, y0, 1 - w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB ); //top left
        InternalDrawQuadRationalBezierSegAA( iBlock, x0, y0 + dy, x0, y1, x1 - dx, y1, w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB ); //bottom left
        InternalDrawQuadRationalBezierSegAA( iBlock, x1, y1 - dy, x1, y1, x1 - dx, y1, 1 - w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB ); //bottom right
        InternalDrawQuadRationalBezierSegAA( iBlock, x1, y1 - dy, x1, y0, x0 + dx, y0, w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB ); //top right
    }
    else //Filled Ellipse
    {
                                                            //               x  y
        std::map< int, std::vector< int > > storagePoints; // storagePoints[x][0]  We have n points for each x on the ellipse: p1(x, y0), p2(x, y1) ... pn(x, yn)
                                                            //                 [1]
                                                            //                 [...]
                                                            //                 [n]
        storagePoints[0].push_back( iCenter.x );           // In order to only pass storagePoints in parameter to InternalDrawQuadRationalBezierSeg
        storagePoints[0].push_back( iCenter.y );           // we store the center (useful in this function) at index 0 (we'll know it's there)

        std::map< int, std::vector< int > > pointsForFill; // same structure, but with only the top and bottom points to draw a line on each x to fill the ellipse


        // Lambda ---
        auto fillPointsForFill = [&] ( bool isTop, int shift )
        {
            //Removing the coordinates of the center we stored
            storagePoints[0].erase(storagePoints[0].begin());
            storagePoints[0].erase(storagePoints[0].begin());

            for (std::map< int, std::vector< int > >::iterator it=storagePoints.begin(); it!=storagePoints.end(); ++it)
            {
                if( it->second.size() > 0 )
                {
                    int extremum = it->second[0];
                    for (int i = 1; i < it->second.size() ; i++ )
                    {
                        if( (isTop && shift == 0) || ( shift == 1 && !isTop ) ) //We get the minimum of it->second
                        {
                            if( extremum > it->second[i] )
                                extremum = it->second[i];
                        }
                        else //We get the maximum of it->second
                        {
                            if( extremum < it->second[i] )
                                extremum = it->second[i];
                        }
                    }
                    pointsForFill[ it->first ].push_back( extremum + shift * (isTop ? 1 : -1 ) );
                }
            }

            storagePoints.clear();
            storagePoints[0].push_back( iCenter.x );
            storagePoints[0].push_back( iCenter.y );
        };
        //Lambda end ----


        // Depending of the angle of the ellipse, we either get the outer antialiased pixels or the inner antialiased pixels of the ellipse. If we get the outer pixels, we need to draw from the less outer one on top to the less outer one on bottom, and shift them one pixel inwards so we don't draw on outer pixels at all
        // If we get the inner pixels, we draw from the less inner one on top to the less inner one on bottom, no need to shift the pixels, we're already inside the ellipse, so we can fill from there.

        int shift = ( ( ( ( iRotationDegrees + 45 ) % 180 ) + 180 ) % 180 ) < 90 ? 0 : 1;

        InternalDrawQuadRationalBezierSegAA( iBlock, x0, y0 + dy, x0, y0, x0 + dx, y0, 1 - w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, &storagePoints ); //top left
        fillPointsForFill( true, shift );

        InternalDrawQuadRationalBezierSegAA( iBlock, x0, y0 + dy, x0, y1, x1 - dx, y1, w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, &storagePoints ); //bottom left
        fillPointsForFill( false, shift );

        InternalDrawQuadRationalBezierSegAA( iBlock, x1, y1 - dy, x1, y1, x1 - dx, y1, 1 - w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, &storagePoints ); //bottom right
        fillPointsForFill( false, shift );

        InternalDrawQuadRationalBezierSegAA( iBlock, x1, y1 - dy, x1, y0, x0 + dx, y0, w, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, &storagePoints ); //top right
        fillPointsForFill( true, shift );

        for (std::map< int, std::vector< int > >::iterator it=pointsForFill.begin(); it!=pointsForFill.end(); ++it)
        {
            if( it->second.size() == 2 )
            {
                DrawLine( iBlock, FPoint( iCenter.x + it->first, iCenter.y + it->second[0] ), FPoint( iCenter.x + it->first, iCenter.y + it->second[1] ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            }
            if( it->second.size() > 2 ) // where we draw more than 2 pixels for a said y (it happens at the junctions between beziers)
            {
                int minY = it->second[0];
                int maxY = it->second[0];
                for( int i = 1; i < it->second.size(); i++)
                {
                    if( minY > it->second[i] )
                        minY = it->second[i];

                    if( maxY < it->second[i] )
                        maxY = it->second[i];
                }
                DrawLine( iBlock, FPoint( iCenter.x + it->first, iCenter.y + minY ), FPoint( iCenter.x + it->first, iCenter.y + maxY ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
            }
        }
    }
}


// ---


static void DrawRectangle( TBlock< _SH >*                   iBlock
                            , const FPoint                  iTopLeft
                            , const FPoint                  iBottomRight
                            , const CColor&                 iColor
                            , const bool                    iFilled
                            , const FRect&                  iClippingRect
                            , const FPerformanceOptions&    iPerformanceOptions
                            , bool                          iCallInvalidCB )
{
    FRect clippingRect = iClippingRect;
    
    if( clippingRect.Area() == 0 )
    {
        clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
    }
    
    const int xmin = FMath::Min3( iTopLeft.x, iBottomRight.x, clippingRect.x + clippingRect.w );
    const int ymin = FMath::Min3( iTopLeft.y, iBottomRight.y, clippingRect.y + clippingRect.h );
    const int xmax = FMath::Max3( iTopLeft.x, iBottomRight.x, clippingRect.x );
    const int ymax = FMath::Max3( iTopLeft.y, iBottomRight.y, clippingRect.y );
    const int width = FMath::Max( xmax - xmin, 0 );
    const int height = FMath::Max( ymax - ymin, 0 );
    if( iFilled )
    {
        const FRect rect = FRect( xmin, ymin, width, height );
        TClearFillContext< _SH >::FillRect( iBlock, iColor, rect, iPerformanceOptions );
    }
    else
    {
        DrawLine( iBlock, FPoint( xmin, ymin ), FPoint( xmax, ymin ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB ); // Top
        DrawLine( iBlock, FPoint( xmax, ymin ), FPoint( xmax, ymax ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB ); // Right
        DrawLine( iBlock, FPoint( xmin, ymax ), FPoint( xmax, ymax ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB ); // Bot
        DrawLine( iBlock, FPoint( xmin, ymin ), FPoint( xmin, ymax ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB ); // Left
    }
}


static void DrawPolygon( TBlock< _SH >*                    iBlock
                        , std::vector< FPoint >&       iPoints
                        , const CColor&                iColor
                        , const bool                   iFilled
                        , const FRect&                  iClippingRect
                        , const FPerformanceOptions&            iPerformanceOptions
                        , bool                         iCallInvalidCB )
{
    if( iPoints.size() < 3 )
        return;

    int j = iPoints.size() - 1;
    for( int i = 0; i < iPoints.size(); i++ )
    {
        DrawLine( iBlock, iPoints.at( i ), iPoints.at( j ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
        j = i;
    }


    if( iFilled )
    {
        int maxX = 0;
        int maxY = 0;
        int minX = INT_MAX;
        int minY = INT_MAX;

        //Initialization of useful variables
        for( int i = 0; i < iPoints.size(); i++ )
        {
            if( maxX < iPoints[i].x )
                maxX = iPoints[i].x;
            if( maxY < iPoints[i].y )
                maxY = iPoints[i].y;
            if( minX > iPoints[i].x )
                minX = iPoints[i].x;
            if( minY > iPoints[i].y )
                minY = iPoints[i].y;
        }

        //We go through the polygon by scanning it top to bottom
        for (int y = minY; y <= maxY; y++)
        {
            std::vector< int > nodesX;
            int j = iPoints.size() - 1;

            for( int i = 0; i < iPoints.size(); i++ )
            {
                if( ( iPoints[i].y < y && iPoints[j].y >= y ) || ( iPoints[j].y < y && iPoints[i].y >= y ) )
                {
                    nodesX.push_back( iPoints[i].x  + double( y - iPoints[i].y ) / double( iPoints[j].y - iPoints[i].y ) * (iPoints[j].x - iPoints[i].x ));
                }
                j = i;
            }

            //Sorting the nodes on X
            int i = 0;
            int size = nodesX.size() - 1;
            while( i < size )
            {
                if( nodesX[i] > nodesX[i+1] )
                {
                    int temp = nodesX[i];
                    nodesX[i]=nodesX[i+1];
                    nodesX[i+1] = temp;
                    if( i > 0 )
                        i--;
                }
                else
                {
                    i++;
                }
            }

            //Filling the polygon on line y
            for( i = 0; i < nodesX.size(); i+= 2)
            {
                if( nodesX[i] > maxX ) break;
                if( nodesX[i+1] > minX )
                {
                    if( nodesX[i] < minX )
                        nodesX[i] = minX;
                    if( nodesX[i+1] > maxX )
                        nodesX[i+1] = maxX;

                    DrawLine( iBlock, FPoint( nodesX[i], y), FPoint( nodesX[i+1], y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                }
            }
        }
    }
}


static void DrawPolygonAA( TBlock< _SH >*               iBlock
                            , std::vector< FPoint >&       iPoints
                            , const CColor&                iColor
                            , const bool                   iFilled
                            , const FRect&                  iClippingRect
                            , const FPerformanceOptions&   iPerformanceOptions
                            , bool                         iCallInvalidCB )
{
    if( iPoints.size() < 3 )
        return;

    int j = iPoints.size() - 1;
    for( int i = 0; i < iPoints.size(); i++ )
    {
        DrawLineAA( iBlock, iPoints.at( i ), iPoints.at( j ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
        j = i;
    }

    if( iFilled )
    {
        int maxX = 0;
        int maxY = 0;
        int minX = INT_MAX;
        int minY = INT_MAX;

        //Initialization of useful variables
        for( int i = 0; i < iPoints.size(); i++ )
        {
            if( maxX < iPoints[i].x )
                maxX = iPoints[i].x;
            if( maxY < iPoints[i].y )
                maxY = iPoints[i].y;
            if( minX > iPoints[i].x )
                minX = iPoints[i].x;
            if( minY > iPoints[i].y )
                minY = iPoints[i].y;
        }

        //We go through the polygon by scanning it top to bottom
        for (int y = minY; y <= maxY; y++)
        {
            std::vector< int > nodesX;
            int j = iPoints.size() - 1;

            for( int i = 0; i < iPoints.size(); i++ )
            {
                if( ( iPoints[i].y < y && iPoints[j].y >= y ) || ( iPoints[j].y < y && iPoints[i].y >= y ) )
                {
                    nodesX.push_back( iPoints[i].x  + double( y - iPoints[i].y ) / double( iPoints[j].y - iPoints[i].y ) * (iPoints[j].x - iPoints[i].x ));
                }
                j = i;
            }

            //Sorting the nodes on X
            int i = 0;
            int size = nodesX.size() - 1;
            while( i < size )
            {
                if( nodesX[i] > nodesX[i+1] )
                {
                    int temp = nodesX[i];
                    nodesX[i]=nodesX[i+1];
                    nodesX[i+1] = temp;
                    if( i > 0 )
                        i--;
                }
                else
                {
                    i++;
                }
            }

            //Filling the polygon on line y
            for( i = 0; i < nodesX.size(); i+= 2)
            {
                if( nodesX[i] > maxX ) break;
                if( nodesX[i+1] > minX )
                {
                    if( nodesX[i] < minX )
                        nodesX[i] = minX;
                    if( nodesX[i+1] > maxX )
                        nodesX[i+1] = maxX;

                    DrawLine( iBlock, FPoint( nodesX[i] + 1, y), FPoint( nodesX[i+1], y ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );
                }
            }
        }
    }
}


static void InternalDrawQuadRationalBezierSeg( TBlock< _SH>* iBlock
                                                , int x0
                                                , int y0
                                                , int x1
                                                , int y1
                                                , int x2
                                                , int y2
                                                , float w
                                                , const CColor&                 iColor
                                                , const FRect&                  iClippingRect
                                                , const FPerformanceOptions&    iPerformanceOptions
                                                , bool iCallInvalidCB
                                                , std::map< int, std::vector< int > >* iStoragePoints = NULL)
{
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );

    int sx = x2-x1, sy = y2-y1;
    double dx = x0-x2, dy = y0-y2, xx = x0-x1, yy = y0-y1;
    double xy = xx*sy+yy*sx, cur = xx*sy-yy*sx, err;

    if( xx*sx > 0.0 || yy*sy > 0.0 )
    {
        return;
    }

    if (cur != 0.0 && w > 0.0) {
        if (sx*(long)sx+sy*(long)sy > xx*xx+yy*yy) {
            x2 = x0; x0 -= dx; y2 = y0; y0 -= dy; cur = -cur;
        }
        xx = 2.0*(4.0*w*sx*xx+dx*dx);
        yy = 2.0*(4.0*w*sy*yy+dy*dy);
        sx = x0 < x2 ? 1 : -1;
        sy = y0 < y2 ? 1 : -1;
        xy = -2.0*sx*sy*(2.0*w*xy+dx*dy);
        if (cur*sx*sy < 0.0)
        {
            xx = -xx; yy = -yy; xy = -xy; cur = -cur;
        }
        dx = 4.0*w*(x1-x0)*sy*cur+xx/2.0+xy;
        dy = 4.0*w*(y0-y1)*sx*cur+yy/2.0+xy;
        if (w < 0.5 && dy > dx)
        {
            cur = -(w+1.0)/2.0;
            w = sqrt(w);
            xy = 1.0/(w+1.0);
            sx = floor((x0+2.0*w*x1+x2)*xy/2.0+0.5);
            sy = floor((y0+2.0*w*y1+y2)*xy/2.0+0.5);
            dx = floor((w*x1+x0)*xy+0.5);
            dy = floor((y1*w+y0)*xy+0.5);
            InternalDrawQuadRationalBezierSeg( iBlock, x0, y0, dx, dy, sx, sy, cur, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, iStoragePoints );
            dx = floor((w*x1+x2)*xy+0.5);
            dy = floor((y1*w+y2)*xy+0.5);
            InternalDrawQuadRationalBezierSeg( iBlock, sx, sy, dx, dy, x2, y2, cur, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, iStoragePoints );
            return;
        }
        err = dx+dy-xy;
        
        // Clipping
        FRect clippingRect = iClippingRect;
        
        if( clippingRect.Area() == 0 )
        {
            clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
        }
        
        FRect bezierRect = FRect::FromMinMax( FMath::Min3( x0, x1, x2 ), FMath::Min3( y0, y1, y2 ), FMath::Max3( x0, x1, x2 ), FMath::Max3( y0, y1, y2 ) );
        
        clippingRect = clippingRect & bezierRect;
        
        //Out
        while( ( x0 < clippingRect.x || x0 > (clippingRect.x + clippingRect.w ) || y0 < clippingRect.y || y0 > (clippingRect.y + clippingRect.h ) ) && ( dy <= xy && dx >= xy ) )
        {
            if( iStoragePoints )
            {
                (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
            }
            if (x0 == x2 && y0 == y2)
                return;
            x1 = 2 * err > dy; y1 = 2*( err + yy ) < -dy;
            if (2 * err < dx || y1) { y0 += sy; dy += xy; err += dx += xx; }
            if (2 * err > dx || x1) { x0 += sx; dx += xy; err += dy += yy; }
        }
        
        //In
        while ( x0 >= clippingRect.x && x0 <= (clippingRect.x + clippingRect.w ) && y0 >= clippingRect.y && y0 <= (clippingRect.y + clippingRect.h ) && ( dy <= xy && dx >= xy ) )
        {
            iBlock->SetPixelValue( x0, y0, val );
            if( iStoragePoints )
            {
                (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
            }
            if (x0 == x2 && y0 == y2)
                return;
            x1 = 2 * err > dy; y1 = 2*( err + yy ) < -dy;
            if (2 * err < dx || y1) { y0 += sy; dy += xy; err += dx += xx; }
            if (2 * err > dx || x1) { x0 += sx; dx += xy; err += dy += yy; }
        }
    }
    DrawLine( iBlock, FPoint( x0, y0 ), FPoint( x2, y2 ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );

    if( iStoragePoints ) //We go here only when we draw an ellipse with beziers
    {
        if( x0 == x2 && y0 == y2 ) //Corners where we draw a single pixel
        {
            (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
        }
        else if( y0 == y2 ) //Horizontal lines
        {
            for( int i = x0; i < x2; i++ )
                (*iStoragePoints)[ i - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
        }
        //We don't need to take care of vertical lines, since storagePoints is used to fill an ellipse using the exact same type of vertical lines
    }
}

static void DrawQuadraticBezier( TBlock< _SH>*                   iBlock
                                , const FPoint&                   iCtrlPt0
                                , const FPoint&                   iCtrlPt1
                                , const FPoint&                   iCtrlPt2
                                , const float                     iWeight
                                , const CColor&                   iColor
                                , const FRect&                    iClippingRect
                                , const FPerformanceOptions&      iPerformanceOptions
                                , bool                            iCallInvalidCB )
{
    int x = iCtrlPt0.x - 2 * iCtrlPt1.x + iCtrlPt2.x;
    int y = iCtrlPt0.y - 2 * iCtrlPt1.y + iCtrlPt2.y;
    double dx = iCtrlPt0.x - iCtrlPt1.x;
    double dy = iCtrlPt0.y - iCtrlPt1.y;
    double dWeight;
    double dt;
    double dq;

    FPoint pt0 = iCtrlPt0;
    FPoint pt1 = iCtrlPt1;
    FPoint pt2 = iCtrlPt2;

    float weight = iWeight;

    if( weight < 0) //Can't draw a bezier curve with a weight < 0
        return;

    if( dx * ( pt2.x - pt1.x ) > 0 )
    {
        if( dy * ( pt2.y - pt1.y ) > 0 )
        {
            if( FMath::Abs( dx * y ) > FMath::Abs( dy * x ) )
            {
                pt0.x = pt2.x;
                pt2.x = dx + pt1.x;
                pt0.y = pt2.y;
                pt2.y = dy + pt1.y;
            }
        }
        if( pt0.x == pt2.x || weight == 1.0 )
        {
            dt = (pt0.x - pt1.x) / (double)x;
        }
        else
        {
            dq = std::sqrt( 4.0 * weight * weight * ( pt0.x - pt1.x ) * ( pt2.x - pt1.x ) + ( pt2.x - pt0.x ) * (long)( pt2.x - pt0.x ) );

            if( pt1.x < pt0.x )
                dq = -dq;

            dt = ( 2.0 * weight * ( pt0.x - pt1.x ) - pt0.x + pt2.x + dq ) / (2.0 * ( 1.0 - weight ) * ( pt2.x - pt0.x ) );
        }
        dq = 1.0 / ( 2.0 * dt * ( 1.0 - dt ) * ( weight - 1.0 ) + 1.0);
        dx = ( dt * dt * ( pt0.x - 2.0 * weight * pt1.x + pt2.x ) + 2.0 * dt * ( weight * pt1.x - pt0.x ) + pt0.x) * dq;
        dy = ( dt * dt * ( pt0.y - 2.0 * weight * pt1.y + pt2.y ) + 2.0 * dt * ( weight * pt1.y - pt0.y ) + pt0.y) * dq;
        dWeight = dt * (weight - 1.0) + 1.0;
        dWeight *= (dWeight * dq);
        weight = ( ( 1.0 - dt ) * ( weight - 1.0 ) + 1.0 ) * std::sqrt(dq);
        x = std::floor( dx + 0.5 );
        y = std::floor( dy + 0.5 );
        dy = ( dx - pt0.x ) * ( pt1.y - pt0.y ) / ( pt1.x - pt0.x ) + pt0.y;
        InternalDrawQuadRationalBezierSeg( iBlock, pt0.x, pt0.y, x, std::floor( dy + 0.5 ), x, y, dWeight, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB  );
        dy = ( dx - pt2.x ) * ( pt1.y - pt2.y ) / ( pt1.x - pt2.x ) + pt2.y;
        pt1.y = std::floor( dy + 0.5 );
        pt0.x = pt1.x = x;
        pt0.y = y;

    }

    if( ( pt0.y - pt1.y ) * (long)( pt2.y - pt1.y ) > 0 )
    {
        if( pt0.y == pt2.y || iWeight == 1.0 )
        {
            dt = ( pt0.y - pt1.y ) / ( pt0.y - 2.0 * pt1.y + pt2.y );
        }
        else
        {
            dq = std::sqrt( 4.0 * weight * weight * ( pt0.y - pt1.y ) * ( pt2.y - pt1.y ) + ( pt2.y - pt0.y ) * (long)( pt2.y - pt0.y ) );

            if( pt1.y < pt0.y )
                dq = -dq;

            dt = ( 2.0 * weight * ( pt0.y - pt1.y ) - pt0.y + pt2.y + dq ) / (2.0 * ( 1.0 - weight ) * ( pt2.y - pt0.y ) );
        }
        dq = 1.0 / ( 2.0 * dt * ( 1.0 - dt ) * ( weight - 1.0 ) + 1.0);
        dx = ( dt * dt * ( pt0.x - 2.0 * weight * pt1.x + pt2.x ) + 2.0 * dt * ( weight * pt1.x - pt0.x ) + pt0.x) * dq;
        dy = ( dt * dt * ( pt0.y - 2.0 * weight * pt1.y + pt2.y ) + 2.0 * dt * ( weight * pt1.y - pt0.y ) + pt0.y) * dq;
        dWeight = dt * (weight - 1.0) + 1.0;
        dWeight *= (dWeight * dq);
        weight = ( ( 1.0 - dt ) * ( weight - 1.0 ) + 1.0 ) * std::sqrt(dq);
        x = std::floor( dx + 0.5 );
        y = std::floor( dy + 0.5 );
        dx = ( pt1.x - pt0.x ) * ( dy - pt0.y ) / ( pt1.y - pt0.y ) + pt0.x;
        InternalDrawQuadRationalBezierSeg( iBlock, pt0.x, pt0.y, std::floor( dx + 0.5 ), y, x, y, dWeight, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB  );

        dx = ( pt1.x - pt2.x ) * ( dy - pt2.y ) / ( pt1.y - pt2.y ) + pt2.x;
        pt1.x = std::floor( dx + 0.5 );
        pt0.x = x;
        pt0.y = pt1.y = y;

    }
    InternalDrawQuadRationalBezierSeg( iBlock, pt0.x, pt0.y, pt1.x, pt1.y, pt2.x, pt2.y, weight * weight, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB  );
}



static void InternalDrawQuadRationalBezierSegAA( TBlock< _SH>* iBlock
                                                , int x0
                                                , int y0
                                                , int x1
                                                , int y1
                                                , int x2
                                                , int y2
                                                , float w
                                                , const CColor& iColor
                                                , const FRect&                  iClippingRect
                                                , const FPerformanceOptions& iPerformanceOptions
                                                , bool iCallInvalidCB
                                                , std::map< int, std::vector< int > >* iStoragePoints = NULL )
{
    TPixelValue< _SH > val = iBlock->PixelValueForColor( iColor );
    auto MaxAlpha = val.GetAlpha();

    int sx = x2-x1, sy = y2-y1;
    double dx = x0-x2, dy = y0-y2, xx = x0-x1, yy = y0-y1;
    double xy = xx*sy+yy*sx, cur = xx*sy-yy*sx, err, ed;
    bool f;

    if( xx*sx > 0.0 || yy*sy > 0.0 )
    {
        return;
    }

    if (cur != 0.0 && w > 0.0)
    {
        if (sx*(long)sx+sy*(long)sy > xx*xx+yy*yy)
        {
            x2 = x0; x0 -= dx; y2 = y0; y0 -= dy; cur = -cur;
        }
        xx = 2.0*(4.0*w*sx*xx+dx*dx);
        yy = 2.0*(4.0*w*sy*yy+dy*dy);
        sx = x0 < x2 ? 1 : -1;
        sy = y0 < y2 ? 1 : -1;
        xy = -2.0*sx*sy*(2.0*w*xy+dx*dy);

        if (cur*sx*sy < 0.0)
        {
            xx = -xx; yy = -yy; xy = -xy; cur = -cur;
        }

        dx = 4.0*w*(x1-x0)*sy*cur+xx/2.0+xy;
        dy = 4.0*w*(y0-y1)*sx*cur+yy/2.0+xy;

        if (w < 0.5 && dy > dx)
        {
            cur = -(w+1.0)/2.0;
            w = sqrt(w);
            xy = 1.0/(w+1.0);
            sx = floor((x0+2.0*w*x1+x2)*xy/2.0+0.5);
            sy = floor((y0+2.0*w*y1+y2)*xy/2.0+0.5);
            dx = floor((w*x1+x0)*xy+0.5);
            dy = floor((y1*w+y0)*xy+0.5);
            InternalDrawQuadRationalBezierSegAA( iBlock, x0, y0, dx, dy, sx, sy, cur, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, iStoragePoints );
            dx = floor((w*x1+x2)*xy+0.5);
            dy = floor((y1*w+y2)*xy+0.5);
            InternalDrawQuadRationalBezierSegAA( iBlock, sx, sy, dx, dy, x2, y2, cur, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB, iStoragePoints );
            return;
        }
        err = dx+dy-xy;
        
        // Clipping
        FRect clippingRect = iClippingRect;
        
        if( clippingRect.Area() == 0 )
        {
            clippingRect = FRect::FromXYWH(0, 0, iBlock->Width() - 1, iBlock->Height() - 1);
        }
        
        FRect bezierRect = FRect::FromMinMax( FMath::Min3( x0, x1, x2 ), FMath::Min3( y0, y1, y2 ), FMath::Max3( x0, x1, x2 ), FMath::Max3( y0, y1, y2 ) );
        
        clippingRect = clippingRect & bezierRect;
        
        if( clippingRect.Area() == 0 )
            return;
        
        //Out
        while( ( x0 < clippingRect.x || x0 > (clippingRect.x + clippingRect.w ) || y0 < clippingRect.y || y0 > (clippingRect.y + clippingRect.h ) ) && ( dy < dx ) )
        {
            cur = FMath::Min( dx - xy, xy - dy );
            ed = FMath::Max( dx - xy, xy - dy );
            ed += ( 2 * ed * cur * cur / (4.0 * ed * ed + cur * cur ) );

            float errorRatio = ( err - dx - dy + xy ) / ed;

            x1 = MaxAlpha * ( 1 - FMath::Abs( errorRatio ) );
            f = (2 * err + dy) < 0;

            if( x1 <= MaxAlpha )
            {
                if( iStoragePoints && errorRatio >= 0 )
                {
                    (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
                }
            }

            if( f )
            {
                if( y0 == y2 )
                    return;
                if( ( dx - err ) < ed )
                {
                    if( iStoragePoints && errorRatio <= 0 )
                    {
                        (*iStoragePoints)[ x0 + sx - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
                    }
                }
            }

            if( 2 * err + dx > 0 )
            {
                if( x0 == x2 )
                    return;
                if( ( err - dy ) < ed )
                {
                    if( iStoragePoints && errorRatio >= 0 )
                    {
                        (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 + sy - (*iStoragePoints)[0][1] );
                    }
                }
                x0 += sx;
                dx += xy;
                err += dy += yy;
            }

            if( f )
            {
                y0 += sy;
                dy += xy;
                err += dx += xx;
            }
        }

        //Border Case
        while ( ( x0 == clippingRect.x || x0 == (clippingRect.x + clippingRect.w ) || y0 == clippingRect.y || y0 == (clippingRect.y + clippingRect.h ) ) && ( dy < dx ) )
        {
            cur = FMath::Min( dx - xy, xy - dy );
            ed = FMath::Max( dx - xy, xy - dy );
            ed += ( 2 * ed * cur * cur / (4.0 * ed * ed + cur * cur ) );

            float errorRatio = ( err - dx - dy + xy ) / ed;

            x1 = MaxAlpha * ( 1 - FMath::Abs( errorRatio ) );
            f = (2 * err + dy) < 0;

            if( x1 <= MaxAlpha )
            {
                val.SetAlpha( x1 );
                iBlock->SetPixelValue( x0, y0, val );

                if( iStoragePoints && errorRatio >= 0 )
                {
                    (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
                }
            }

            if( f )
            {
                if( y0 == y2 )
                    return;
                if( ( dx - err ) < ed )
                {
                    //float errorRatio = 1 - ( dx - err ) / ed ;
                    //float alpha = FMath::Abs( errorRatio );

                    //val.SetAlpha( MaxAlpha * alpha );
                    //iBlock->SetPixelValue( x0 + sx, y0, val );

                    if( iStoragePoints && errorRatio <= 0 )
                    {
                        (*iStoragePoints)[ x0 + sx - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
                    }
                }
            }

            if( 2 * err + dx > 0 )
            {
                if( x0 == x2 )
                    return;
                if( ( err - dy ) < ed )
                {
                    //float errorRatio = 1 - ( err - dy ) / ed;
                    //float alpha = FMath::Abs( errorRatio );

                    //val.SetAlpha( MaxAlpha * alpha );
                    //iBlock->SetPixelValue( x0, y0 + sy, val );

                    if( iStoragePoints && errorRatio >= 0 )
                    {
                        (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 + sy - (*iStoragePoints)[0][1] );
                    }
                }
                x0 += sx;
                dx += xy;
                err += dy += yy;
            }

            if( f )
            {
                y0 += sy;
                dy += xy;
                err += dx += xx;
            }
        }

        //In
        while ( x0 > clippingRect.x && x0 < (clippingRect.x + clippingRect.w ) && y0 > clippingRect.y && y0 < (clippingRect.y + clippingRect.h ) && ( dy < dx ) )
        {
            cur = FMath::Min( dx - xy, xy - dy );
            ed = FMath::Max( dx - xy, xy - dy );
            ed += ( 2 * ed * cur * cur / (4.0 * ed * ed + cur * cur ) );

            float errorRatio = ( err - dx - dy + xy ) / ed;

            x1 = MaxAlpha * ( 1 - FMath::Abs( errorRatio ) );
            f = (2 * err + dy) < 0;

            if( x1 <= MaxAlpha )
            {
                val.SetAlpha( x1 );
                iBlock->SetPixelValue( x0, y0, val );

                if( iStoragePoints && errorRatio >= 0 )
                {
                    (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
                }
            }

            if( f )
            {
                if( y0 == y2 )
                    return;
                if( ( dx - err ) < ed )
                {
                    float errorRatio = 1 - ( dx - err ) / ed ;
                    float alpha = FMath::Abs( errorRatio );

                    val.SetAlpha( MaxAlpha * alpha );
                    iBlock->SetPixelValue( x0 + sx, y0, val );

                    if( iStoragePoints && errorRatio <= 0 )
                    {
                        (*iStoragePoints)[ x0 + sx - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
                    }
                }
            }

            if( 2 * err + dx > 0 )
            {
                if( x0 == x2 )
                    return;
                if( ( err - dy ) < ed )
                {
                    float errorRatio = 1 - ( err - dy ) / ed;
                    float alpha = FMath::Abs( errorRatio );

                    val.SetAlpha( MaxAlpha * alpha );
                    iBlock->SetPixelValue( x0, y0 + sy, val );

                    if( iStoragePoints && errorRatio >= 0 )
                    {
                        (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 + sy - (*iStoragePoints)[0][1] );
                    }
                }
                x0 += sx;
                dx += xy;
                err += dy += yy;
            }

            if( f )
            {
                y0 += sy;
                dy += xy;
                err += dx += xx;
            }
        }
        
        //Border Case
        while ( ( x0 == clippingRect.x || x0 == (clippingRect.x + clippingRect.w ) || y0 == clippingRect.y || y0 == (clippingRect.y + clippingRect.h ) ) && ( dy < dx ) )
        {
            cur = FMath::Min( dx - xy, xy - dy );
            ed = FMath::Max( dx - xy, xy - dy );
            ed += ( 2 * ed * cur * cur / (4.0 * ed * ed + cur * cur ) );

            float errorRatio = ( err - dx - dy + xy ) / ed;

            x1 = MaxAlpha * ( 1 - FMath::Abs( errorRatio ) );
            f = (2 * err + dy) < 0;

            if( x1 <= MaxAlpha )
            {
                val.SetAlpha( x1 );
                iBlock->SetPixelValue( x0, y0, val );

                if( iStoragePoints && errorRatio >= 0 )
                {
                    (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
                }
            }

            if( f )
            {
                if( y0 == y2 )
                    return;
                if( ( dx - err ) < ed )
                {
                    //float errorRatio = 1 - ( dx - err ) / ed ;
                    //float alpha = FMath::Abs( errorRatio );

                    //val.SetAlpha( MaxAlpha * alpha );
                    //iBlock->SetPixelValue( x0 + sx, y0, val );

                    if( iStoragePoints && errorRatio <= 0 )
                    {
                        (*iStoragePoints)[ x0 + sx - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
                    }
                }
            }

            if( 2 * err + dx > 0 )
            {
                if( x0 == x2 )
                    return;
                if( ( err - dy ) < ed )
                {
                    //float errorRatio = 1 - ( err - dy ) / ed;
                    //float alpha = FMath::Abs( errorRatio );

                    //val.SetAlpha( MaxAlpha * alpha );
                    //iBlock->SetPixelValue( x0, y0 + sy, val );

                    if( iStoragePoints && errorRatio >= 0 )
                    {
                        (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 + sy - (*iStoragePoints)[0][1] );
                    }
                }
                x0 += sx;
                dx += xy;
                err += dy += yy;
            }

            if( f )
            {
                y0 += sy;
                dy += xy;
                err += dx += xx;
            }
        }
    }
    DrawLineAA( iBlock, FPoint( x0, y0 ), FPoint( x2, y2 ), iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB );

    if( iStoragePoints ) //We go here only when we draw an ellipse with beziers
    {
        if( x0 == x2 && y0 == y2 ) //Corners where we draw a single pixel
        {
            (*iStoragePoints)[ x0 - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
        }
        else if( y0 == y2 ) //Horizontal lines
        {
            for( int i = x0; i < x2; i++ )
                (*iStoragePoints)[ i - (*iStoragePoints)[0][0] ].push_back(y0 - (*iStoragePoints)[0][1] );
        }
        //We don't need to take care of vertical lines, since storagePoints is used to fill an ellipse using the exact same type of vertical lines
    }
}

static void DrawQuadraticBezierAA( TBlock< _SH>*                   iBlock
                                    , const FPoint&                   iCtrlPt0
                                    , const FPoint&                   iCtrlPt1
                                    , const FPoint&                   iCtrlPt2
                                    , const float                     iWeight
                                    , const CColor&                   iColor
                                    , const FRect&                    iClippingRect
                                    , const FPerformanceOptions&      iPerformanceOptions
                                    , bool                            iCallInvalidCB )
{
    int x = iCtrlPt0.x - 2 * iCtrlPt1.x + iCtrlPt2.x;
    int y = iCtrlPt0.y - 2 * iCtrlPt1.y + iCtrlPt2.y;
    double dx = iCtrlPt0.x - iCtrlPt1.x;
    double dy = iCtrlPt0.y - iCtrlPt1.y;
    double dWeight;
    double dt;
    double dq;

    FPoint pt0 = iCtrlPt0;
    FPoint pt1 = iCtrlPt1;
    FPoint pt2 = iCtrlPt2;

    float weight = iWeight;

    if( weight < 0) //Can't draw a bezier curve with a weight < 0
        return;

    if( dx * ( pt2.x - pt1.x ) > 0 )
    {
        if( dy * ( pt2.y - pt1.y ) > 0 )
        {
            if( FMath::Abs( dx * y ) > FMath::Abs( dy * x ) )
            {
                pt0.x = pt2.x;
                pt2.x = dx + pt1.x;
                pt0.y = pt2.y;
                pt2.y = dy + pt1.y;
            }
        }
        if( pt0.x == pt2.x || weight == 1.0 )
        {
            dt = (pt0.x - pt1.x) / (double)x;
        }
        else
        {
            dq = std::sqrt( 4.0 * weight * weight * ( pt0.x - pt1.x ) * ( pt2.x - pt1.x ) + ( pt2.x - pt0.x ) * (long)( pt2.x - pt0.x ) );

            if( pt1.x < pt0.x )
                dq = -dq;

            dt = ( 2.0 * weight * ( pt0.x - pt1.x ) - pt0.x + pt2.x + dq ) / (2.0 * ( 1.0 - weight ) * ( pt2.x - pt0.x ) );
        }
        dq = 1.0 / ( 2.0 * dt * ( 1.0 - dt ) * ( weight - 1.0 ) + 1.0);
        dx = ( dt * dt * ( pt0.x - 2.0 * weight * pt1.x + pt2.x ) + 2.0 * dt * ( weight * pt1.x - pt0.x ) + pt0.x) * dq;
        dy = ( dt * dt * ( pt0.y - 2.0 * weight * pt1.y + pt2.y ) + 2.0 * dt * ( weight * pt1.y - pt0.y ) + pt0.y) * dq;
        dWeight = dt * (weight - 1.0) + 1.0;
        dWeight *= (dWeight * dq);
        weight = ( ( 1.0 - dt ) * ( weight - 1.0 ) + 1.0 ) * std::sqrt(dq);
        x = std::floor( dx + 0.5 );
        y = std::floor( dy + 0.5 );
        dy = ( dx - pt0.x ) * ( pt1.y - pt0.y ) / ( pt1.x - pt0.x ) + pt0.y;
        InternalDrawQuadRationalBezierSegAA( iBlock, pt0.x, pt0.y, x, std::floor( dy + 0.5 ), x, y, dWeight, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB  );
        dy = ( dx - pt2.x ) * ( pt1.y - pt2.y ) / ( pt1.x - pt2.x ) + pt2.y;
        pt1.y = std::floor( dy + 0.5 );
        pt0.x = pt1.x = x;
        pt0.y = y;
    }

    if( ( pt0.y - pt1.y ) * (long)( pt2.y - pt1.y ) > 0 )
    {
        if( pt0.y == pt2.y || iWeight == 1.0 )
        {
            dt = ( pt0.y - pt1.y ) / ( pt0.y - 2.0 * pt1.y + pt2.y );
        }
        else
        {
            dq = std::sqrt( 4.0 * weight * weight * ( pt0.y - pt1.y ) * ( pt2.y - pt1.y ) + ( pt2.y - pt0.y ) * (long)( pt2.y - pt0.y ) );

            if( pt1.y < pt0.y )
                dq = -dq;

            dt = ( 2.0 * weight * ( pt0.y - pt1.y ) - pt0.y + pt2.y + dq ) / (2.0 * ( 1.0 - weight ) * ( pt2.y - pt0.y ) );
        }
        dq = 1.0 / ( 2.0 * dt * ( 1.0 - dt ) * ( weight - 1.0 ) + 1.0);
        dx = ( dt * dt * ( pt0.x - 2.0 * weight * pt1.x + pt2.x ) + 2.0 * dt * ( weight * pt1.x - pt0.x ) + pt0.x) * dq;
        dy = ( dt * dt * ( pt0.y - 2.0 * weight * pt1.y + pt2.y ) + 2.0 * dt * ( weight * pt1.y - pt0.y ) + pt0.y) * dq;
        dWeight = dt * (weight - 1.0) + 1.0;
        dWeight *= (dWeight * dq);
        weight = ( ( 1.0 - dt ) * ( weight - 1.0 ) + 1.0 ) * std::sqrt(dq);
        x = std::floor( dx + 0.5 );
        y = std::floor( dy + 0.5 );
        dx = ( pt1.x - pt0.x ) * ( dy - pt0.y ) / ( pt1.y - pt0.y ) + pt0.x;
        InternalDrawQuadRationalBezierSegAA( iBlock, pt0.x, pt0.y, std::floor( dx + 0.5 ), y, x, y, dWeight, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB  );

        dx = ( pt1.x - pt2.x ) * ( dy - pt2.y ) / ( pt1.y - pt2.y ) + pt2.x;
        pt1.x = std::floor( dx + 0.5 );
        pt0.x = x;
        pt0.y = pt1.y = y;
    }
    InternalDrawQuadRationalBezierSegAA( iBlock, pt0.x, pt0.y, pt1.x, pt1.y, pt2.x, pt2.y, weight * weight, iColor, iClippingRect, iPerformanceOptions, iCallInvalidCB  );
}



};

/////////////////////////////////////////////////////
// Undefines
#undef tSpec

} // namespace ULIS
