/*
   OpenGUI - Drawing & Windowing library

   Copyright (C) 1996,2005  Marian Krivos

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   nezmar@atlas.sk

   base.cc - base graphics support routines
*/

#include <string.h>
#include <math.h>
#include "fgbase.h"

#ifdef FG_NAMESPACE
namespace fgl
{
#endif

FGRect::FGRect(const FGDrawBuffer& sprite)
{
	x = y = 0;
	w = sprite.GetW();
	h = sprite.GetH();
}

/*!
	Returns TRUE if the point \a p is inside or on the edge of the
	rectangle; otherwise returns FALSE.

	If \a proper is TRUE, this function returns TRUE only if \a p is
	inside (not on the edge).
*/
bool FGRect::contains( const FGPoint &p, bool proper ) const
{
	if ( proper )
	return p.x > x && p.x < x+w &&
		   p.y > y && p.y < y+h;
	else
	return p.x >= x && p.x <= x+w &&
		   p.y >= y && p.y <= y+h;
}

bool FGRect::contains( int px, int py, bool proper ) const
{
	if ( proper )
		return px > x && px < x+w &&
		   py > y && py < y+h;
	else
		return px >= x && px <= x+w &&
		   py >= y && py <= y+h;
}

bool FGRect::contains( const FGRect &r, bool proper ) const
{
	if ( proper )
		return r.x > x && r.x+w < x+w && r.y > y && r.y+h < y+h;
	else
	    return r.x >= x && r.x+w <= x+w && r.y >= y && r.y+h <= y+h;
}

FGRect& FGRect::operator|=(const FGRect &r)
{
	*this = *this | r;
	return *this;
}

FGRect& FGRect::operator&=(const FGRect &r)
{
	*this = *this & r;
	return *this;
}

FGRect FGRect::operator|(const FGRect &r) const
{
	if ( isValid() ) {
	if ( r.isValid() )
	{
		FGRect tmp;
		tmp.x = MIN( x, r.x );
		tmp.w = MAX( w, r.w );
		tmp.y = MIN( y, r.y );
		tmp.h = MAX( h, r.h );
		return tmp;
	} else {
		return *this;
	}
	} else {
	return r;
	}
}

FGRect FGRect::unite( const FGRect &r ) const
{
	return *this | r;
}

FGRect FGRect::operator&( const FGRect &r ) const
{
	FGRect tmp;
	tmp.x = MAX( x, r.x );
	tmp.w = MIN( w, r.w );
	tmp.y = MAX( y, r.y );
	tmp.h = MIN( h, r.h );
	return tmp;
}

FGRect FGRect::intersect( const FGRect &r ) const
{
	return *this & r;
}

bool operator==( const FGRect &r1, const FGRect &r2 )
{
	return r1.x==r2.x && r1.w==r2.w && r1.y==r2.y && r1.h==r2.h;
}

bool operator!=( const FGRect &r1, const FGRect &r2 )
{
	return r1.x!=r2.x || r1.w!=r2.w || r1.y!=r2.y || r1.h!=r2.h;
}

/**
	Returns the bounding rectangle of the points in the array, or
	FGRect(0,0,0,0) if the array is empty.
*/
FGRect FGPointArray::BoundingRect() const
{
	if ( isEmpty() )
		return FGRect( 0, 0, 0, 0 );		// null rectangle

	register FGPoint *pd = array;
	int minx, maxx, miny, maxy;
	minx = maxx = pd->x;
	miny = maxy = pd->y;
	pd++;
	for ( int i=1; i<(int)vertices; i++ )
	{	// find min+max x and y
	    if ( pd->x < minx )
		    minx = pd->x;
		else if ( pd->x > maxx )
	    	maxx = pd->x;
		if ( pd->y < miny )
	    	miny = pd->y;
		else if ( pd->y > maxy )
	    	maxy = pd->y;
		pd++;
	}
	return FGRect( FGPoint(minx,miny), FGPoint(maxx, maxy) );
}

bool   FGPointArray::contains(const FGPoint &p) const
{
    return contains(p.x, p.y);
}

bool   FGPointArray::contains(int x, int y) const
{
	int c = 0;

    if (vertices < 3) return false;

	for (unsigned i = 0, j = vertices-1; i < vertices; j = i++)
	{
		if ((((array[i].y <= y) && (y<array[j].y)) ||
			((array[j].y <= y) && (y<array[i].y))) &&
			(x < (array[j].x - array[i].x) * (y - array[i].y) / (array[j].y - array[i].y) + array[i].x))
			c = !c;
	}
	return c;
}



/*****************************************************************************
  FGSize member functions
 *****************************************************************************/

/*!
  \fn FGSize::FGSize()
  Constructs a size with invalid (negative) width and height.
*/

/*!
  \fn FGSize::FGSize( int w, int h )
  Constructs a size with width \a w and height \a h.
*/

/*!
  \fn bool FGSize::isNull() const
  Returns TRUE if the width is 0 and the height is 0; otherwise
  returns FALSE.
*/

/*!
  \fn bool FGSize::isEmpty() const
  Returns TRUE if the width is less than or equal to 0, or the height is
  less than or equal to 0; otherwise returns FALSE.
*/

/*!
  \fn bool FGSize::isValid() const
  Returns TRUE if the width is equal to or greater than 0 and the height is
  equal to or greater than 0; otherwise returns FALSE.
*/

/*!
  \fn int FGSize::width() const
  Returns the width.
  \sa height()
*/

/*!
  \fn int FGSize::height() const
  Returns the height.
  \sa width()
*/

/*!
  \fn void FGSize::setWidth( int w )
  Sets the width to \a w.
  \sa width(), setHeight()
*/

/*!
  \fn void FGSize::setHeight( int h )
  Sets the height to \a h.
  \sa height(), setWidth()
*/

/*!
  Swaps the values of width and height.
*/
void FGSize::transpose()
{
	FGCOORD tmp = wd;
	wd = ht;
	ht = tmp;
}

/*! \enum FGSize::ScaleMode

	This enum type defines the different ways of scaling a size.

	\img scaling.png

	\value ScaleFree  The size is scaled freely. The ratio is not preserved.
	\value ScaleMin  The size is scaled to a rectangle as large as possible
					 inside a given rectangle, preserving the aspect ratio.
	\value ScaleMax  The size is scaled to a rectangle as small as possible
					 outside a given rectangle, preserving the aspect ratio.

	\sa FGSize::scale(), QImage::scale(), QImage::smoothScale()
*/

/*!
	Scales the size to a rectangle of width \a w and height \a h according
	to the ScaleMode \a mode.

	- If \a mode is \c ScaleFree, the size is set to (\a w, \a h).
	- If \a mode is \c ScaleMin, the current size is scaled to a rectangle
	   as large as possible inside (\a w, \a h), preserving the aspect ratio.
	- If \a mode is \c ScaleMax, the current size is scaled to a rectangle
	   as small as possible outside (\a w, \a h), preserving the aspect ratio.

	Example:
	\code
	FGSize t1( 10, 12 );
	t1.scale( 60, 60, FGSize::ScaleFree );
	// t1 is (60, 60)

	FGSize t2( 10, 12 );
	t2.scale( 60, 60, FGSize::ScaleMin );
	// t2 is (50, 60)

	FGSize t3( 10, 12 );
	t3.scale( 60, 60, FGSize::ScaleMax );
	// t3 is (60, 72)
	\endcode
*/
void FGSize::scale( int w, int h, ScaleMode mode )
{
	if ( mode == ScaleFree ) {
	wd = (FGCOORD)w;
	ht = (FGCOORD)h;
	} else {
	bool useHeight = true;
	int w0 = width();
	int h0 = height();
	int rw = h * w0 / h0;

	if ( mode == ScaleMin ) {
		useHeight = ( rw <= w );
	} else { // mode == ScaleMax
		useHeight = ( rw >= w );
	}

	if ( useHeight ) {
		wd = (FGCOORD)rw;
		ht = (FGCOORD)h;
	} else {
		wd = (FGCOORD)w;
		ht = (FGCOORD)( w * h0 / w0 );
	}
	}
}

/*!
	Equivalent to scale( \a {s}.width(), \a {s}.height(), \a mode).
*/
void FGSize::scale( const FGSize &s, ScaleMode mode )
{
	scale( s.width(), s.height(), mode );
}

/*!
  \fn FGCOORD& FGSize::rwidth()
  Returns a reference to the width.

  Using a reference makes it possible to directly manipulate the width.

  Example:
  \code
	FGSize s( 100, 10 );
	s.rwidth() += 20;		// s becomes (120,10)
  \endcode

  \sa rheight()
*/

/*!
  \fn FGCOORD &FGSize::rheight()
  Returns a reference to the height.

  Using a reference makes it possible to directly manipulate the height.

  Example:
  \code
	FGSize s( 100, 10 );
	s.rheight() += 5;		// s becomes (100,15)
  \endcode

  \sa rwidth()
*/

/*!
  \fn FGSize &FGSize::operator+=( const FGSize &s )

  Adds \a s to the size and returns a reference to this size.

  Example:
  \code
	FGSize s(  3, 7 );
	FGSize r( -1, 4 );
	s += r;			// s becomes (2,11)
\endcode
*/

/*!
  \fn FGSize &FGSize::operator-=( const FGSize &s )

  Subtracts \a s from the size and returns a reference to this size.

  Example:
  \code
	FGSize s(  3, 7 );
	FGSize r( -1, 4 );
	s -= r;			// s becomes (4,3)
  \endcode
*/

/*!
  \fn FGSize &FGSize::operator*=( int c )
  Multiplies both the width and height by \a c and returns a reference to
  the size.
*/

/*!
  Multiplies both the width and height by \a c and returns a reference to
  the size.

  Note that the result is truncated.
*/

/*!
  \fn bool operator==( const FGSize &s1, const FGSize &s2 )
  \relates FGSize
  Returns TRUE if \a s1 and \a s2 are equal; otherwise returns FALSE.
*/

/*!
  \fn bool operator!=( const FGSize &s1, const FGSize &s2 )
  \relates FGSize
  Returns TRUE if \a s1 and \a s2 are different; otherwise returns FALSE.
*/

/*!
  \fn const FGSize operator+( const FGSize &s1, const FGSize &s2 )
  \relates FGSize
  Returns the sum of \a s1 and \a s2; each component is added separately.
*/

/*!
  \fn const FGSize operator-( const FGSize &s1, const FGSize &s2 )
  \relates FGSize
  Returns \a s2 subtracted from \a s1; each component is
  subtracted separately.
*/

/*!
  \fn const FGSize operator*( const FGSize &s, int c )
  \relates FGSize
  Multiplies \a s by \a c and returns the result.
*/

/*!
  \relates FGSize
  Multiplies \a s by \a c and returns the result.
*/

/*!
  \overload const FGSize operator*( const FGSize &s, double c )
  \relates FGSize
  Multiplies \a s by \a c and returns the result.
*/

/*!
  \relates FGSize
  Multiplies \a s by \a c and returns the result.
*/

/*!
  \fn FGSize &FGSize::operator/=( int c )
  Divides both the width and height by \a c and returns a reference to the
  size.
*/

/*!
  \fn FGSize &FGSize::operator/=( double c )
  Divides both the width and height by \a c and returns a reference to the
  size.

  Note that the result is truncated.
*/

/*!
  \fn const FGSize operator/( const FGSize &s, int c )
  \relates FGSize
  Divides \a s by \a c and returns the result.
*/

/*!
  \fn const FGSize operator/( const FGSize &s, double c )
  \relates FGSize
  \overload
  Divides \a s by \a c and returns the result.

  Note that the result is truncated.
*/

/*!
  \fn FGSize FGSize::expandedTo( const FGSize & otherSize ) const

  Returns a size with the maximum width and height of this size and
  \a otherSize.
*/

/*!
  \fn FGSize FGSize::boundedTo( const FGSize & otherSize ) const

  Returns a size with the minimum width and height of this size and
  \a otherSize.
*/

#ifdef FG_NAMESPACE
}
#endif

