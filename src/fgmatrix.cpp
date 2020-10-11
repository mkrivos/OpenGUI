/****************************************************************************
** $Id: fgmatrix.cpp 4310 2006-02-21 15:23:52Z majo $
**
** Implementation of FGMatrix class
**
** Created : 941020
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <math.h>
#include <limits.h>

#include "fgbase.h"
#include "fgmatrix.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif


static inline int FGround(double value)
{
	if (value < 0)
		return int(value - 0.5);
	return int(value + 0.5);
}

/*!
	@class FGMatrix fgmatrix.h
    @brief The FGMatrix class specifies 2D transformations of a
    coordinate system.

    The standard coordinate system of a has the origin located at the top-left
	position. X values increase to the right; Y values increase downward.

    This coordinate system is the default for the FGDrawBuffer, which
    renders graphics in a paint device. A user-defined coordinate
    system can be specified by setting a FGMatrix for the FGDrawBuffer.

    Example:
    @code
	MyWindow::OnPaint(void)
	{
            FGMatrix m;                      // our transformation matrix
			FGPoint position(30,20);
            m.translate( 100, 100 );                // rotated coordinate system
			m.scale( 3, 2);
			position = m.map(position);
            WindowText( position.x, position.y, "detator" );  // draw rotated text at 100+30*3, 100+20*2
	}
    @endcode

    A matrix specifies how to translate, scale, shear or rotate the
    postitions.

    The FGMatrix class contains a 3x3 matrix of the form:
    <table align=center border=1 cellpadding=1 cellspacing=0>
    <tr align=center><td>m11</td><td>m12</td><td>&nbsp;0 </td></tr>
    <tr align=center><td>m21</td><td>m22</td><td>&nbsp;0 </td></tr>
    <tr align=center><td>dx</td> <td>dy</td> <td>&nbsp;1 </td></tr>
    </table>

    A matrix transforms a point in the plane to another point:
    @code
	x' = m11*x + m21*y + dx
	y' = m22*y + m12*x + dy
    @endcode

    The point @e (x, y) is the original point, and @e (x', y') is the
	transformed point. @e (x', y') can be transformed back to @e (x, y)
	by performing the same operation on the FGMatrix::invert() inverted matrix.

    The elements @e dx and @e dy specify horizontal and vertical
    translation. The elements @e m11 and @e m22 specify horizontal and
    vertical scaling. The elements @e m12 and @e m21 specify
    horizontal and vertical shearing.

    The identity matrix has @e m11 and @e m22 set to 1; all others are
    set to 0. This matrix maps a point to itself.

    Translation is the simplest transformation. Setting @e dx and @e
    dy will move the coordinate system @e dx units along the X axis
    and @e dy units along the Y axis.

    Scaling can be done by setting @e m11 and @e m22. For example,
    setting @e m11 to 2 and @e m22 to 1.5 will double the height and
    increase the width by 50%.

    Shearing is controlled by @e m12 and @e m21. Setting these
    elements to values different from zero will twist the coordinate
    system.

    Rotation is achieved by carefully setting both the shearing
    factors and the scaling factors. The FGMatrix also has a function
    that sets rotate() directly.

    FGMatrix lets you combine transformations like this:
    @code
	FGMatrix m;           // identity matrix
	m.translate(10, -20); // first translate (10,-20)
	m.rotate(25);         // then rotate 25 degrees
	m.scale(1.2, 0.7);    // finally scale it
    @endcode

    Here's the same example using basic matrix operations:
    @code
	double a    = pi/180 * 25;         // convert 25 to radians
	double sina = sin(a);
	double cosa = cos(a);
	FGMatrix m1(1, 0, 0, 1, 10, -20);  // translation matrix
	FGMatrix m2( cosa, sina,           // rotation matrix
		    -sina, cosa, 0, 0 );
	FGMatrix m3(1.2, 0, 0, 0.7, 0, 0); // scaling matrix
	FGMatrix m;
	m = m3 * m2 * m1;                  // combine all transformations
    @endcode

    QPainter has functions to translate, scale, shear and rotate the
    coordinate system without using a FGMatrix. Although these
    functions are very convenient, it can be more efficient to build a
    FGMatrix and call QPainter::setWorldMatrix() if you want to perform
    more than a single transform operation.

    @sa QPainter::setWorldMatrix(), QPixmap::xForm()
*/

bool qt_old_transformations = true;

/*!
    @enum FGMatrix::TransformationMode

    FGMatrix offers two transformation modes. Calculations can either
    be done in terms of points (Points mode, the default), or in
    terms of area (Area mode).

    In Points mode the transformation is applied to the points that
    mark out the shape's bounding line. In Areas mode the
    transformation is applied in such a way that the area of the
    contained region is correctly transformed under the matrix.

    * Points transformations are applied to the shape's points.
    * Areas transformations are applied (e.g. to the width and
    height) so that the area is transformed.

    Example:

    Suppose we have a rectangle,
    @c {FGRect( 10, 20, 30, 40 )} and a transformation matrix
    @c {FGMatrix( 2, 0, 0, 2, 0, 0 )} to double the rectangle's size.

    In Points mode, the matrix will transform the top-left (10,20) and
    the bottom-right (39,59) points producing a rectangle with its
    top-left point at (20,40) and its bottom-right point at (78,118),
    i.e. with a width of 59 and a height of 79.

    In Areas mode, the matrix will transform the top-left point in
    the same way as in Points mode to (20/40), and double the width
    and height, so the bottom-right will become (69,99), i.e. a width
    of 60 and a height of 80.

    Because integer arithmetic is used (for speed), FGrounding
    differences mean that the modes will produce slightly different
    results given the same shape and the same transformation,
    especially when scaling up. This also means that some operations
    are not commutative.

    Under Points mode, @c {matrix * ( region1 | region2 )} is not equal to
    @c {matrix * region1 | matrix * region2}. Under Area mode, @c {matrix *
    (pointarray[i])} is not neccesarily equal to
    @c {(matrix * pointarry)[i]}.

    @image html xform.png Comparison of Points and Areas TransformationModes
*/

/*!
    Sets the transformation mode that FGMatrix and painter
    transformations use to @a m.

    @sa FGMatrix::TransformationMode
*/
void FGMatrix::setTransformationMode( FGMatrix::TransformationMode m )
{
    if ( m == FGMatrix::Points )
		qt_old_transformations = true;
	else
	    qt_old_transformations = false;
}


/*!
    Returns the current transformation mode.

    @sa FGMatrix::TransformationMode
*/
FGMatrix::TransformationMode FGMatrix::transformationMode()
{
    return (qt_old_transformations ? FGMatrix::Points : FGMatrix::Areas );
}


// some defines to inline some code
#define MAPDOUBLE( x, y, nx, ny ) \
{ \
    double fx = x; \
    double fy = y; \
    nx = _m11*fx + _m21*fy + _dx; \
    ny = _m12*fx + _m22*fy + _dy; \
}

#define MAPINT( x, y, nx, ny ) \
{ \
    double fx = x; \
    double fy = y; \
    nx = (int)FGround(_m11*fx + _m21*fy + _dx); \
    ny = (int)FGround(_m12*fx + _m22*fy + _dy); \
}

/*****************************************************************************
  FGMatrix member functions
 *****************************************************************************/

/*!
    Constructs an identity matrix. All elements are set to zero except
    @e m11 and @e m22 (scaling), which are set to 1.
*/

FGMatrix::FGMatrix()
{
	_mscale = 1.0;
    _m11 = _m22 = 1.0;
    _m12 = _m21 = _dx = _dy = 0.0;
}

/*!
    Constructs a matrix with the elements, @a m11, @a m12, @a m21, @a
    m22, @a dx and @a dy.
*/

FGMatrix::FGMatrix( double m11, double m12, double m21, double m22,
		    double dx, double dy )
{
	_mscale = m11;
    _m11 = m11;	 _m12 = m12;
    _m21 = m21;	 _m22 = m22;
    _dx	 = dx;	 _dy  = dy;
}


/*!
    Sets the matrix elements to the specified values, @a m11, @a m12,
    @a m21, @a m22, @a dx and @a dy.
*/

void FGMatrix::setMatrix( double m11, double m12, double m21, double m22,
			  double dx, double dy )
{
	_mscale = m11;
    _m11 = m11;	 _m12 = m12;
    _m21 = m21;	 _m22 = m22;
    _dx	 = dx;	 _dy  = dy;
}


/*!
    @fn double FGMatrix::m11() const

    Returns the X scaling factor.
*/

/*!
    @fn double FGMatrix::m12() const

    Returns the vertical shearing factor.
*/

/*!
    @fn double FGMatrix::m21() const

    Returns the horizontal shearing factor.
*/

/*!
    @fn double FGMatrix::m22() const

    Returns the Y scaling factor.
*/

/*!
    @fn double FGMatrix::dx() const

    Returns the horizontal translation.
*/

/*!
    @fn double FGMatrix::dy() const

    Returns the vertical translation.
*/


/*!
    Transforms ( @a x, @a y ) to ( @a *tx, @a *ty ) using the
    following formulae:

    @code
	*tx = m11*x + m21*y + dx
	*ty = m22*y + m12*x + dy
    @endcode
*/

void FGMatrix::map( double x, double y, double *tx, double *ty ) const
{
    MAPDOUBLE( x, y, *tx, *ty );
}

/*!
    Transforms ( @a x, @a y ) to ( @a *tx, @a *ty ) using the formulae:

    @code
	*tx = m11*x + m21*y + dx  (FGrounded to the nearest integer)
	*ty = m22*y + m12*x + dy  (FGrounded to the nearest integer)
    @endcode
*/

void FGMatrix::map( int x, int y, int *tx, int *ty ) const
{
    MAPINT( x, y, *tx, *ty );
}

/*!
    @fn FGPoint FGMatrix::map( const FGPoint &p ) const

    Transforms @a p to using the formulae:

    @code
	retx = m11*px + m21*py + dx  (FGrounded to the nearest integer)
	rety = m22*py + m12*px + dy  (FGrounded to the nearest integer)
    @endcode
*/

/*!
  @fn FGRect FGMatrix::map( const FGRect &r ) const

  @deprecated

  Please use FGMatrix::mapRect() instead.

  Note that this method does return the bounding rectangle of the @a r, when
  shearing or rotations are used.
*/

/*!
    @fn FGPointArray FGMatrix::map( const FGPointArray &a ) const

    Returns the point array @a a transformed by calling map for each point.
*/


/*!
    Returns the transformed rectangle @a rect.

    The bounding rectangle is returned if rotation or shearing has
    been specified.

    If you need to know the exact region @a rect maps to use operator*().

    @sa operator*()
*/
FGRect FGMatrix::mapRect( const FGRect &rect ) const
{
    FGRect result;

    if( qt_old_transformations )
	{
		if ( _m12 == 0.0F && _m21 == 0.0F )
		{
		    result = FGRect( map(rect.topLeft()), map(rect.bottomRight()) ).normalize();
	    }
		else
		{
            // rectangle to polygon
		    FGPointArray a( rect ), b;
            // map src to dst
	        map( a, b );
            // polygon to bounding rect
		    result = b.BoundingRect();
	    }
    }
	else
	{
		if ( _m12 == 0.0F && _m21 == 0.0F )
		{
	        int x = (int)FGround( _m11*rect.x + _dx );
			int y = (int)FGround( _m22*rect.y + _dy );
	        int w = (int)FGround( _m11*rect.w );
	        int h = (int)FGround( _m22*rect.h );

		    if ( w < 0 )
			{
		    	w = -w;
	    	    x -= w-1;
		    }
		    if ( h < 0 )
	    	{
			    h = -h;
	    		y -= h-1;
	        }
		    result = FGRect( x, y, w, h );
	    }
		else
		{
		    // see mapToPolygon for explanations of the algorithm.
	        double x0, y0;
	        double x, y;
		    MAPDOUBLE( rect.left(), rect.top(), x0, y0 );
	        double xmin = x0;
		    double ymin = y0;
	        double xmax = x0;
		    double ymax = y0;
	        MAPDOUBLE( rect.right() + 1, rect.top(), x, y );
		    xmin = MIN( xmin, x );
		    ymin = MIN( ymin, y );
		    xmax = MAX( xmax, x );
		    ymax = MAX( ymax, y );
		    MAPDOUBLE( rect.right() + 1, rect.bottom() + 1, x, y );
		    xmin = MIN( xmin, x );
		    ymin = MIN( ymin, y );
			xmax = MAX( xmax, x );
	        ymax = MAX( ymax, y );
	        MAPDOUBLE( rect.left(), rect.bottom() + 1, x, y );
		    xmin = MIN( xmin, x );
	        ymin = MIN( ymin, y );
	        xmax = MAX( xmax, x );
		    ymax = MAX( ymax, y );
	        double w = xmax - xmin;
	        double h = ymax - ymin;
		    xmin -= ( xmin - x0 ) / w;
		    ymin -= ( ymin - y0 ) / h;
	        xmax -= ( xmax - x0 ) / w;
	        ymax -= ( ymax - y0 ) / h;
		    result = FGRect( (int)FGround(xmin), (int)FGround(ymin), (int)FGround(xmax)-(int)FGround(xmin)+1, (int)FGround(ymax)-(int)FGround(ymin)+1 );
	    }
    }
    return result;
}


/*!
  @internal
*/
FGPoint FGMatrix::operator *( const FGPoint &p ) const
{
    double fx = p.x;
    double fy = p.y;
    return FGPoint( FGround(_m11*fx + _m21*fy + _dx),
		   FGround(_m12*fx + _m22*fy + _dy) );
}

/*!
  @internal
*/
FGSize FGMatrix::operator *( const FGSize &sz ) const
{
	FGRect r(sz);
	r = mapRect(r);
	return FGSize(r.w, r.h);
}

/*!
  @internal
*/
FGCircle FGMatrix::operator *( const FGCircle &p ) const
{
	double fx = p.x;
	double fy = p.y;

	double rx = p.x+p.r;
	double ry = p.y;

	int x = FGround(_m11*fx + _m21*fy + _dx);
	int y = FGround(_m12*fx + _m22*fy + _dy);
	int r1 = FGround(_m11*rx + _m21*ry + _dx);
	int r2 = FGround(_m12*rx + _m22*ry + _dy);

	return FGCircle(x,y, hypot((x-r1),(y-r2)));
}


void FGMatrix::map(const FGPointArray& src, FGPointArray& dst) const
{
	struct QWMDoublePoint
	{
		double x,y;
	};

	if( qt_old_transformations )
	{
		for ( int i=0; i<(int)dst.vertices; i++ )
		{
			MAPINT( src.array[i].x, src.array[i].y, dst.array[i].x, dst.array[i].y );
		}
	}
	else
	{
		int i;
		int size = src.vertices;

		FGPoint *da = src.array;
		QWMDoublePoint *dp = new QWMDoublePoint[size];

		double xmin = INT_MAX;
		double ymin = xmin;
		double xmax = INT_MIN;
		double ymax = xmax;
		int xminp = 0;
		int yminp = 0;

		for( i = 0; i < size; i++ )
		{
			dp[i].x = da[i].x;
			dp[i].y = da[i].y;

			if ( dp[i].x < xmin )
			{
				xmin = dp[i].x;
				xminp = i;
			}
			if ( dp[i].y < ymin )
			{
				ymin = dp[i].y;
				yminp = i;
			}
			xmax = MAX( xmax, dp[i].x );
			ymax = MAX( ymax, dp[i].y );
		}

		double w = MAX( xmax - xmin, 1 );
		double h = MAX( ymax - ymin, 1 );

		for( i = 0; i < size; i++ )
		{
			dp[i].x += (dp[i].x - xmin)/w;
			dp[i].y += (dp[i].y - ymin)/h;
			MAPDOUBLE( dp[i].x, dp[i].y, dp[i].x, dp[i].y );
		}

		// now apply correction back for transformed values...
		xmin = INT_MAX;
		ymin = xmin;
		xmax = INT_MIN;
		ymax = xmax;

		for( i = 0; i < size; i++ )
		{
			xmin = MIN( xmin, dp[i].x );
			ymin = MIN( ymin, dp[i].y );
			xmax = MAX( xmax, dp[i].x );
			ymax = MAX( ymax, dp[i].y );
		}
		w = MAX( xmax - xmin, 1 );
		h = MAX( ymax - ymin, 1 );

		FGPoint *dr = dst.array;

		for( i = 0; i < size; i++ )
		{
			dr[i].x = ( FGround( dp[i].x - (dp[i].x - dp[xminp].x)/w ) );
			dr[i].y = ( FGround( dp[i].y - (dp[i].y - dp[yminp].y)/h ) );
		}
		delete dp;
	}
}

/*!
  @internal
*/
FGPointArray FGMatrix::operator *( const FGPointArray &a ) const
{
	if( qt_old_transformations )
	{
		FGPointArray result(a);
		for ( int i=0; i<(int)result.vertices; i++ )
		{
			MAPINT( result.array[i].x, result.array[i].y, result.array[i].x, result.array[i].y );
		}
		return result;
	}
	else
	{
		int i;
		int size = a.vertices;
		FGPoint *da = a.array;
		FGDoublePoint *dp = new FGDoublePoint[size];

		double xmin = INT_MAX;
		double ymin = xmin;
		double xmax = INT_MIN;
		double ymax = xmax;
		int xminp = 0;
		int yminp = 0;
	for( i = 0; i < size; i++ ) {
		dp[i].x = da[i].x;
		dp[i].y = da[i].y;
		if ( dp[i].x < xmin ) {
		xmin = dp[i].x;
		xminp = i;
		}
		if ( dp[i].y < ymin ) {
		ymin = dp[i].y;
		yminp = i;
		}
		xmax = MAX( xmax, dp[i].x );
		ymax = MAX( ymax, dp[i].y );
	}
	double w = MAX( xmax - xmin, 1 );
	double h = MAX( ymax - ymin, 1 );
	for( i = 0; i < size; i++ ) {
		dp[i].x += (dp[i].x - xmin)/w;
		dp[i].y += (dp[i].y - ymin)/h;
		MAPDOUBLE( dp[i].x, dp[i].y, dp[i].x, dp[i].y );
	}

	// now apply correction back for transformed values...
	xmin = INT_MAX;
	ymin = xmin;
	xmax = INT_MIN;
	ymax = xmax;
	for( i = 0; i < size; i++ ) {
		xmin = MIN( xmin, dp[i].x );
		ymin = MIN( ymin, dp[i].y );
		xmax = MAX( xmax, dp[i].x );
		ymax = MAX( ymax, dp[i].y );
	}
	w = MAX( xmax - xmin, 1 );
	h = MAX( ymax - ymin, 1 );

	FGPointArray result( size );
	FGPoint *dr = result.array;
	for( i = 0; i < size; i++ ) {
		dr[i].x = ( FGround( dp[i].x - (dp[i].x - dp[xminp].x)/w ) );
		dr[i].y = ( FGround( dp[i].y - (dp[i].y - dp[yminp].y)/h ) );
	}
	delete [] dp;
	return result;
	}
}

/*!
	Resets the matrix to an identity matrix.

    All elements are set to zero, except @e m11 and @e m22 (scaling)
    which are set to 1.

    @sa isIdentity()
*/

void FGMatrix::reset()
{
	_mscale = 1.0;
    _m11 = _m22 = 1.0;
    _m12 = _m21 = _dx = _dy = 0.0;
}

/*!
    Returns TRUE if the matrix is the identity matrix; otherwise returns FALSE.

    @sa reset()
*/
bool FGMatrix::isIdentity() const
{
    return _m11 == 1.0 && _m22 == 1.0 && _m12 == 0.0 && _m21 == 0.0
	&& _dx == 0.0 && _dy == 0.0;
}

/*!
    Moves the coordinate system @a dx along the X-axis and @a dy along
    the Y-axis.

    Returns a reference to the matrix.

    @sa scale(), shear(), rotate()
*/

FGMatrix &FGMatrix::translate( double dx, double dy )
{
	_dx += dx*_m11 + dy*_m21;
	_dy += dy*_m22 + dx*_m12;
	return *this;
}

/*!
	Scales the coordinate system unit by @a sx horizontally and @a sy
	vertically.

	Returns a reference to the matrix.

	@sa translate(), shear(), rotate()
*/

FGMatrix &FGMatrix::scale( double sx, double sy )
{
	_mscale *= sx;

	_m11 *= sx;
	_m12 *= sx;
	_m21 *= sy;
	_m22 *= sy;
	return *this;
}

/*!
	Shears the coordinate system by @a sh horizontally and @a sv
	vertically.

    Returns a reference to the matrix.

    @sa translate(), scale(), rotate()
*/

FGMatrix &FGMatrix::shear( double sh, double sv )
{
    double tm11 = sv*_m21;
    double tm12 = sv*_m22;
    double tm21 = sh*_m11;
    double tm22 = sh*_m12;
    _m11 += tm11;
    _m12 += tm12;
    _m21 += tm21;
    _m22 += tm22;
    return *this;
}

const double deg2rad = 0.017453292519943295769;	// pi/180

/*!
    Rotates the coordinate system @a a degrees counterclockwise.

    Returns a reference to the matrix.

    @sa translate(), scale(), shear()
*/

FGMatrix &FGMatrix::rotate( double a )
{
    double b = deg2rad*a;			// convert to radians
    double sina = sin(b);
    double cosa = cos(b);
    double tm11 = cosa*_m11 + sina*_m21;
    double tm12 = cosa*_m12 + sina*_m22;
    double tm21 = -sina*_m11 + cosa*_m21;
    double tm22 = -sina*_m12 + cosa*_m22;
    _m11 = tm11; _m12 = tm12;
    _m21 = tm21; _m22 = tm22;
    return *this;
}

/*!
    @fn bool FGMatrix::isInvertible() const

    Returns TRUE if the matrix is invertible; otherwise returns FALSE.

    @sa invert()
*/

/*!
    @fn double FGMatrix::det() const

    Returns the matrix's determinant.
*/


/*!
    Returns the inverted matrix.

    If the matrix is singular (not invertible), the identity matrix is
    returned.

    If @a invertible is not 0: the value of @a *invertible is set
    to TRUE if the matrix is invertible; otherwise @a *invertible is
    set to FALSE.

    @sa isInvertible()
*/

FGMatrix FGMatrix::invert( bool *invertible ) const
{
    double determinant = det();
    if ( determinant == 0.0 ) {
	if ( invertible )
		*invertible = false;		// singular matrix
	FGMatrix defaultMatrix;
	return defaultMatrix;
	}
	else {					// invertible matrix
	if ( invertible )
	    *invertible = true;
	double dinv = 1.0/determinant;
	FGMatrix imatrix( (_m22*dinv),	(-_m12*dinv),
			  (-_m21*dinv), ( _m11*dinv),
			  ((_m21*_dy - _m22*_dx)*dinv),
			  ((_m12*_dx - _m11*_dy)*dinv) );
	return imatrix;
    }
}


/*!
    Returns TRUE if this matrix is equal to @a m; otherwise returns FALSE.
*/

bool FGMatrix::operator==( const FGMatrix &m ) const
{
    return _m11 == m._m11 &&
	   _m12 == m._m12 &&
	   _m21 == m._m21 &&
	   _m22 == m._m22 &&
	   _dx == m._dx &&
	   _dy == m._dy;
}

/*!
    Returns TRUE if this matrix is not equal to @a m; otherwise returns FALSE.
*/

bool FGMatrix::operator!=( const FGMatrix &m ) const
{
    return _m11 != m._m11 ||
	   _m12 != m._m12 ||
	   _m21 != m._m21 ||
	   _m22 != m._m22 ||
	   _dx != m._dx ||
	   _dy != m._dy;
}

/*!
    Returns the result of multiplying this matrix by matrix @a m.
*/

FGMatrix &FGMatrix::operator*=( const FGMatrix &m )
{
    double tm11 = _m11*m._m11 + _m12*m._m21;
    double tm12 = _m11*m._m12 + _m12*m._m22;
    double tm21 = _m21*m._m11 + _m22*m._m21;
    double tm22 = _m21*m._m12 + _m22*m._m22;

    double tdx  = _dx*m._m11  + _dy*m._m21 + m._dx;
    double tdy =  _dx*m._m12  + _dy*m._m22 + m._dy;

    _mscale *= m._mscale;

    _m11 = tm11; _m12 = tm12;
    _m21 = tm21; _m22 = tm22;
    _dx = tdx; _dy = tdy;
    return *this;
}

/*!
    @relates FGMatrix
    Returns the product of @a m1 * @a m2.

    Note that matrix multiplication is not commutative, i.e. a*b !=
    b*a.
*/

FGMatrix operator*( const FGMatrix &m1, const FGMatrix &m2 )
{
    FGMatrix result = m1;
    result *= m2;
    return result;
}


#ifdef FG_NAMESPACE
}
#endif

