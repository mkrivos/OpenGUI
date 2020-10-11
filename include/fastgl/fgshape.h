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

   fgshape.h - base graphics support
*/

#ifndef __FGSHAPE_H
#define __FGSHAPE_H

#ifdef FG_NAMESPACE
namespace fgl
{
#endif

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#define	MIN(a,b)	( (a) < (b) ? (a) : (b) )
#define MAX(a,b)	( (a) > (b) ? (a) : (b) )

/**
* @ingroup Types
* Basic type for coordinates storage.
*/
typedef int FGCOORD;

struct XYPoint { FGCOORD x; FGCOORD y; };

/**
	@brief The FGPoint class defines a point in the plane.

	@ingroup images
	@ingroup graphics

	A point is specified by an x coordinate and a y coordinate.

	The coordinate type is @c FGCOORD (a 32-bit integer). The minimum
	value of @c FGCOORD is @c FGCOORD_MIN (-2147483648) and the maximum
	value is  @c FGCOORD_MAX (2147483647).

	The coordinates are accessed by the functions x() and y(); they
	can be set by setX() and setY() or by the reference functions rx()
	and ry().

	Given a point @e p, the following statements are all equivalent:
	@code
	p.setX( p.x() + 1 );
	p += FGPoint( 1, 0 );
	p.rx()++;
	@endcode

	A FGPoint can also be used as a vector. Addition and subtraction
	of FGPoints are defined as for vectors (each component is added
	separately). You can divide or multiply a FGPoint by an @c int or a
	@c double. The function manhattanLength() gives an inexpensive
	approximation of the length of the FGPoint interpreted as a vector.

	Example:
	@code
	//FGPoint oldPos is defined somewhere else
	MyWidget::mouseMoveEvent( QMouseEvent *e )
	{
		FGPoint vector = e->pos() - oldPos;
		if ( vector.manhattanLength() > 3 )
		... //mouse has moved more than 3 pixels since oldPos
	}
	@endcode

	FGPoints can be compared for equality or inequality, and they can
	be written to and read from a QStream.

	@sa FGPointArray FGRect
*/
struct FGPoint
 : public XYPoint
{
	/*!
		Constructs a point with coordinates (0, 0) (isNull() returns TRUE).
	*/
	FGPoint()
	{
		x=y=0;
	}
	/*!
		Constructs a point with x value @a xpos and y value @a ypos.
	*/
	FGPoint(int xpos, int ypos)
	{
		x = xpos;
		y = ypos;
	}

	/*!
		Constructs a point with x value @a xpos and y value @a ypos.
	*/
	FGPoint(double xpos, double ypos)
	{
		x = (int)xpos;
		y = (int)ypos;
	}

	FGPoint(const XYPoint& old)
	{
		x = old.x;
		y = old.y;
	}

	/*!
		Returns TRUE if both the x value and the y value are 0; otherwise
		returns FALSE.
	*/
	inline bool isNull() const
	{ return x == 0 && y == 0; }

	/*!
		Adds point @a p to this point and returns a reference to this
		point.

		Example:
		@code
		FGPoint p(  3, 7 );
		FGPoint q( -1, 4 );
		p += q;            // p becomes (2,11)
		@endcode
	*/
	inline class FGPoint &operator+=( const class FGPoint &p )
	{ x+=p.x; y+=p.y; return *this; }

	/*!
		Subtracts point @a p from this point and returns a reference to
		this point.

		Example:
		@code
		FGPoint p(  3, 7 );
		FGPoint q( -1, 4 );
		p -= q;            // p becomes (4,3)
		@endcode
	*/
	inline class FGPoint &operator-=( const class FGPoint &p )
	{ x-=p.x; y-=p.y; return *this; }

	/*!
		Multiplies this point's x and y by @a c, and returns a reference
		to this point.

		Example:
		@code
		FGPoint p( -1, 4 );
		p *= 2;            // p becomes (-2,8)
		@endcode
	*/
	inline class FGPoint &operator*=( int c )
	{ x*=(FGCOORD)c; y*=(FGCOORD)c; return *this; }

	/*!
		Multiplies this point's x and y by @a c, and returns a reference
		to this point.

		Example:
		@code
		FGPoint p( -1, 4 );
		p *= 2.5;          // p becomes (-3,10)
		@endcode

		Note that the result is truncated because points are held as
		integers.
	*/
	inline class FGPoint &operator*=( double c )
	{ x=(FGCOORD)(x*c); y=(FGCOORD)(y*c); return *this; }
	/*!
		Divides both x and y by @a c, and returns a reference to this
		point.

		Example:
		@code
		FGPoint p( -2, 8 );
		p /= 2;            // p becomes (-1,4)
		@endcode
	*/
	inline class FGPoint &operator/=( int c )
	{
		x/=(FGCOORD)c;
		y/=(FGCOORD)c;
		return *this;
	}

	/*!
		Divides both x and y by @a c, and returns a reference to this
		point.

		Example:
		@code
		FGPoint p( -3, 10 );
		p /= 2.5;           // p becomes (-1,4)
		@endcode

		Note that the result is truncated because points are held as
		integers.
	*/
	inline class FGPoint &operator/=( double c )
	{
		x=(FGCOORD)(x/c);
		y=(FGCOORD)(y/c);
		return *this;
	}

	/**
		Returns the sum of the absolute values of x() and y(),
		traditionally known as the "Manhattan length" of the vector from
		the origin to the point. The tradition arises because such
		distances apply to travelers who can only travel on a rectangular
		grid, like the streets of Manhattan.

		This is a useful, and quick to calculate, approximation to the
		true length: sqrt(pow(x(),2)+pow(y(),2)).
	*/
	int ManhattanLength(void) const
	{
		return labs(x)+labs(y);
	}
	friend inline bool	 operator==( const class FGPoint &, const class FGPoint & );
	friend inline bool	 operator!=( const class FGPoint &, const class FGPoint & );
	friend inline const class FGPoint operator+( const class FGPoint &, const class FGPoint & );
	friend inline const class FGPoint operator-( const class FGPoint &, const class FGPoint & );
	friend inline const class FGPoint operator*( const class FGPoint &, int );
	friend inline const class FGPoint operator*( int, const class FGPoint & );
	friend inline const class FGPoint operator*( const class FGPoint &, double );
	friend inline const class FGPoint operator*( double, const class FGPoint & );
	friend inline const class FGPoint operator-( const class FGPoint & );
	friend inline const class FGPoint operator/( const class FGPoint &, int );
	friend inline const class FGPoint operator/( const class FGPoint &, double );
};

/*!
	@relates FGPoint
	Returns TRUE if @a p1 and @a p2 are equal; otherwise returns FALSE.
*/
inline bool operator==( const class FGPoint &p1, const class FGPoint &p2 )
{ return p1.x == p2.x && p1.y == p2.y; }

/*!
	@relates FGPoint

	Returns TRUE if @a p1 and @a p2 are not equal; otherwise returns FALSE.
*/
inline bool operator!=( const class FGPoint &p1, const class FGPoint &p2 )
{ return p1.x != p2.x || p1.y != p2.y; }


/*!
	@relates FGPoint

	Returns the sum of @a p1 and @a p2; each component is added separately.
*/
inline const class FGPoint operator+( const class FGPoint &p1, const class FGPoint &p2 )
{ return FGPoint(p1.x+p2.x, p1.y+p2.y); }


/*!
	@relates FGPoint

	Returns @a p2 subtracted from @a p1; each component is subtracted
	separately.
*/

inline const class FGPoint operator-( const class FGPoint &p1, const class FGPoint &p2 )
{ return FGPoint(p1.x-p2.x, p1.y-p2.y); }

/*!
	@relates FGPoint

	Returns the FGPoint formed by multiplying both components of @a p
	by @a c.
*/
inline const class FGPoint operator*( const class FGPoint &p, int c )
{ return FGPoint(p.x*c, p.y*c); }

/*!
	@relates FGPoint

	Returns the FGPoint formed by multiplying both components of @a p
	by @a c.
*/
inline const class FGPoint operator*( int c, const class FGPoint &p )
{ return FGPoint(p.x*c, p.y*c); }

/*!
	@relates FGPoint

	Returns the FGPoint formed by multiplying both components of @a p
	by @a c.

	Note that the result is truncated because points are held as
	integers.
*/
inline const class FGPoint operator*( const class FGPoint &p, double c )
{ return FGPoint(p.x*c, p.y*c); }

/*!
	@relates FGPoint

	Returns the FGPoint formed by multiplying both components of @a p
	by @a c.

	Note that the result is truncated because points are held as
	integers.
*/
inline const class FGPoint operator*( double c, const class FGPoint &p )
{ return FGPoint((FGCOORD)(p.x*c), (FGCOORD)(p.y*c)); }

/*!
	@relates FGPoint

	Returns the FGPoint formed by changing the sign of both components
	of @a p, equivalent to @c {FGPoint(0,0) - p}.
*/
inline const class FGPoint operator-( const class FGPoint &p )
{ return FGPoint(-p.x, -p.y); }

/*!
	@relates FGPoint

	Returns the FGPoint formed by dividing both components of @a p by
	@a c.
*/
inline const class FGPoint operator/( const class FGPoint &p, int c )
{
	return FGPoint(p.x/c, p.y/c);
}
/*!
	@relates FGPoint

	Returns the FGPoint formed by dividing both components of @a p
	by @a c.

	Note that the result is truncated because points are held as
	integers.
*/
inline const class FGPoint operator/( const class FGPoint &p, double c )
{
	return FGPoint((FGCOORD)(p.x/c), (FGCOORD)(p.y/c));
}

/**
  \brief The FGSize class defines the size of a two-dimensional object.

  \ingroup images
  \ingroup graphics

  A size is specified by a width and a height.

  The coordinate type is FGCOORD (defined as \c int).
  The minimum value of FGCOORD is FGCOORD_MIN (-2147483648) and the maximum
  value is FGCOORD_MAX (2147483647).

  The size can be set in the constructor and changed with setWidth()
  and setHeight(), or using operator+=(), operator-=(), operator*=()
  and operator/=(), etc. You can swap the width and height with
  transpose(). You can get a size which holds the maximum height and
  width of two sizes using expandedTo(), and the minimum height and
  width of two sizes using boundedTo().

  \sa FGPoint, FGRect
*/
class FGSize
{
	enum ScaleMode
	{
		ScaleFree,
		ScaleMin,
		ScaleMax
	};
public:
	FGCOORD wd;
	FGCOORD ht;

	FGSize();
	FGSize( int w, int h );

	bool   isNull()	const;
	bool   isEmpty()	const;
	bool   isValid()	const;

	int	   width()	const;
	int	   height()	const;
	void   setWidth( int w );
	void   setHeight( int h );
	void   transpose();

	class FGSize expandedTo( const class FGSize & ) const;
	class FGSize boundedTo( const class FGSize & ) const;

	void scale( int w, int h, ScaleMode mode );
	void scale( const class FGSize &s, ScaleMode mode );

	FGCOORD &rwidth();
	FGCOORD &rheight();

	class FGSize &operator+=( const class FGSize & );
	class FGSize &operator-=( const class FGSize & );
	class FGSize &operator*=( int c );
	class FGSize &operator*=( double c );
	class FGSize &operator/=( int c );
	class FGSize &operator/=( double c );

	friend inline bool	operator==( const class FGSize &, const class FGSize & );
	friend inline bool	operator!=( const class FGSize &, const class FGSize & );
	friend inline const class FGSize operator+( const class FGSize &, const class FGSize & );
	friend inline const class FGSize operator-( const class FGSize &, const class FGSize & );
	friend inline const class FGSize operator*( const class FGSize &, int );
	friend inline const class FGSize operator*( int, const class FGSize & );
	friend inline const class FGSize operator*( const class FGSize &, double );
	friend inline const class FGSize operator*( double, const class FGSize & );
	friend inline const class FGSize operator/( const class FGSize &, int );
	friend inline const class FGSize operator/( const class FGSize &, double );
};

/*****************************************************************************
  FGSize inline functions
 *****************************************************************************/

inline FGSize::FGSize()
{ wd = ht = -1; }

inline FGSize::FGSize( int w, int h )
{ wd=(FGCOORD)w; ht=(FGCOORD)h; }

inline bool FGSize::isNull() const
{ return wd==0 && ht==0; }

inline bool FGSize::isEmpty() const
{ return wd<1 || ht<1; }

inline bool FGSize::isValid() const
{ return wd>=0 && ht>=0; }

inline int FGSize::width() const
{ return wd; }

inline int FGSize::height() const
{ return ht; }

inline void FGSize::setWidth( int w )
{ wd=(FGCOORD)w; }

inline void FGSize::setHeight( int h )
{ ht=(FGCOORD)h; }

inline FGCOORD &FGSize::rwidth()
{ return wd; }

inline FGCOORD &FGSize::rheight()
{ return ht; }

inline FGSize &FGSize::operator+=( const FGSize &s )
{ wd+=s.wd; ht+=s.ht; return *this; }

inline FGSize &FGSize::operator-=( const FGSize &s )
{ wd-=s.wd; ht-=s.ht; return *this; }

inline FGSize &FGSize::operator*=( int c )
{ wd*=(FGCOORD)c; ht*=(FGCOORD)c; return *this; }

inline FGSize &FGSize::operator*=( double c )
{ wd=(FGCOORD)(wd*c); ht=(FGCOORD)(ht*c); return *this; }


inline bool operator==( const FGSize &s1, const FGSize &s2 )
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

inline bool operator!=( const FGSize &s1, const FGSize &s2 )
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

inline const FGSize operator+( const FGSize & s1, const FGSize & s2 )
{ return FGSize(s1.wd+s2.wd, s1.ht+s2.ht); }

inline const FGSize operator-( const FGSize &s1, const FGSize &s2 )
{ return FGSize(s1.wd-s2.wd, s1.ht-s2.ht); }

inline const FGSize operator*( const FGSize &s, int c )
{ return FGSize(s.wd*c, s.ht*c); }

inline const FGSize operator*( int c, const FGSize &s )
{  return FGSize(s.wd*c, s.ht*c); }

inline const FGSize operator*( const FGSize &s, double c )
{ return FGSize((FGCOORD)(s.wd*c), (FGCOORD)(s.ht*c)); }

inline const FGSize operator*( double c, const FGSize &s )
{ return FGSize((FGCOORD)(s.wd*c), (FGCOORD)(s.ht*c)); }

inline const FGSize operator/( const FGSize &s, int c )
{
	return FGSize(s.wd/c, s.ht/c);
}

inline const FGSize operator/( const FGSize &s, double c )
{
	return FGSize((FGCOORD)(s.wd/c), (FGCOORD)(s.ht/c));
}


inline FGSize &FGSize::operator/=( int c )
{
	wd/=(FGCOORD)c; ht/=(FGCOORD)c;
	return *this;
}

inline FGSize &FGSize::operator/=( double c )
{
	wd=(FGCOORD)(wd/c); ht=(FGCOORD)(ht/c);
	return *this;
}

inline FGSize FGSize::expandedTo( const FGSize & otherSize ) const
{
	return FGSize( MAX(wd,otherSize.wd), MAX(ht,otherSize.ht) );
}

inline FGSize FGSize::boundedTo( const FGSize & otherSize ) const
{
	return FGSize( MIN(wd,otherSize.wd), MIN(ht,otherSize.ht) );
}

/**
	Circle object.
*/
struct FGCircle : public FGPoint
{
	FGCOORD     r;

	/*!
		Constructs an invalid rectangle.
	*/
	FGCircle()
	 : FGPoint()
	{
		r=0;
	}

	/*!
		Constructs a circle with the \a top, \a left corner and \a
		width and \a height.
	*/
	FGCircle(int xpos, int ypos, int radius)
	: FGPoint(xpos,ypos)
	{
		r = radius;
	}

	/*!
	*/
	FGCircle(const FGPoint point, int radius)
	: FGPoint(point)
	{
		r = radius;
	}
	/*!
		Returns TRUE if the circle is valid; otherwise returns FALSE.

		A valid circle has a left() \<= right() and top() \<= bottom().

		Note that non-trivial operations like intersections are not defined
		for invalid circles.

		\c {isValid() == !isEmpty()}

		\sa isNull(), isEmpty(), normalize()
	*/
	inline bool isValid(void) const
	{
		return r >= 0;
	}

	/*!
		Returns TRUE if the circle is a null circle; otherwise
		returns FALSE.

		A null rectangle has both the width and the height set to 0, that
		is right() == left() - 1 and bottom() == top() - 1.

		Note that if right() == left() and bottom() == top(), then the
		circle has width 1 and height 1.

		A null circle is also empty.

		A null circle is not valid.

		\sa isEmpty(), isValid()
	*/
	inline bool isNull(void) const
	{ return r == 0; }

	/*!
		Returns TRUE if the circle is empty; otherwise returns FALSE.

		An empty circle has a left() \> right() or top() \> bottom().

		An empty circle is not valid. \c {isEmpty() == !isValid()}

		\sa isNull(), isValid(), normalize()
	*/
	inline bool isEmpty(void) const
	{
		return r < 0;
	}
	/*!
		Returns TRUE if the point \a x, \a y is inside this circle,
		otherwise returns FALSE.

		If \a proper is TRUE, this function returns TRUE only if the point
		is entirely inside (not on the edge).
	*/
	bool   contains( double xc, double yc) const
	{
		if ( ((x-xc)*(x-xc) + (y-yc)*(y-yc)) <= ((double)r*r) )
			return true;
		return 0;
	}
};

class FGDrawBuffer;

/*!
	@brief The FGRect class defines a rectangle in the plane.

	@ingroup images
	@ingroup graphics

	A rectangle is internally represented as an upper-left corner and
	a bottom-right corner, but it is normally expressed as an
	upper-left corner and a size.

	The coordinate type is FGCOORD (defined in @c qwindowdefs.h as @c
	int). The minimum value of FGCOORD is FGCOORD_MIN (-2147483648) and
	the maximum value is  FGCOORD_MAX (2147483647).

	Note that the size (width and height) of a rectangle might be
	different from what you are used to. If the top-left corner and
	the bottom-right corner are the same, the height and the width of
	the rectangle will both be 1.

	Generally, @ e{width = right - left + 1} and @e {height = bottom -
	top + 1}. We designed it this way to make it correspond to
	rectangular spaces used by drawing functions in which the width
	and height denote a number of pixels. For example, drawing a
	rectangle with width and height 1 draws a single pixel.

	The default coordinate system has origin (0, 0) in the top-left
	corner. The positive direction of the y axis is down, and the
	positive x axis is from left to right.

	A FGRect can be constructed with a set of left, top, width and
	height integers, from two FGPoints or from a FGPoint and a FGSize.
	After creation the dimensions can be changed, e.g. with setLeft(),
	setRight(), setTop() and setBottom(), or by setting sizes, e.g.
	setWidth(), setHeight() and setSize(). The dimensions can also be
	changed with the move functions, e.g. moveBy(), moveCenter(),
	moveBottomRight(), etc. You can also add coordinates to a
	rectangle with addCoords().

	You can test to see if a FGRect contains a specific point with
	contains(). You can also test to see if two FGRects intersect with
	intersects() (see also intersect()). To get the bounding rectangle
	of two FGRects use unite().

	@sa FGPoint
*/
struct FGRect : public FGPoint
{
	FGCOORD     w;
	FGCOORD     h;
	/*!
		Constructs an invalid rectangle.
	*/
	FGRect()
	 : FGPoint()
	{
		w=h=0;
	}

	//! copy
	FGRect(const FGRect& old) : FGPoint(old) { memcpy(this, &old, sizeof(*this)); }

	/*!
		Constructs a rectangle with the \a top, \a left corner and \a
		width and \a height.

		Example (creates two identical rectangles):
		\code
		FGRect r1( FGPoint(100,200), FGPoint(110,215) );
		FGRect r2( 100, 200, 11, 16 );
		\endcode
	*/
	FGRect(int xpos, int ypos, int width, int height)
	: FGPoint(xpos,ypos)
	{
		w = width;
		h = height;
	}

	/*!
		Constructs a rectangle with the top left corner at (0,0) and \a
		width and \a height.
	*/
	FGRect(int width, int height)
	: FGPoint(0, 0), w(width), h(height)
	{}

	/*!
		Constructs a rectangle with \a topLeft as the top-left corner and
		\a bottomRight as the bottom-right corner.
	*/
	FGRect(const FGPoint r0, const FGPoint r1)
	: FGPoint(r0)
	{
		w = r1.x - x;
		h = r1.y - y;
	}

	/*!
		Constructs a rectangle with \a topLeft as the top-left corner and
		\a bottomRight as the bottom-right corner.
	*/
	FGRect(const FGSize& sz)
	: FGPoint()
	{
		w = sz.width();
		h = sz.height();
	}

	/*!
		Constructs a rectangle with \a topLeft as the top-left corner and
		\a bottomRight as the bottom-right corner.
	*/
	FGRect(const FGCircle& c)
	: FGPoint(c.x-c.r, c.y-c.r)
	{
		w = h = c.r*2;
	}

	FGRect(const FGDrawBuffer& sprite);
	/*!
		Returns TRUE if the rectangle is valid; otherwise returns FALSE.

		A valid rectangle has a left() \<= right() and top() \<= bottom().

		Note that non-trivial operations like intersections are not defined
		for invalid rectangles.

		\c {isValid() == !isEmpty()}

		\sa isNull(), isEmpty(), normalize()
	*/
	inline bool isValid(void) const
	{
		return w >= 0 && h >= 0;
	}

	/*!
		Returns TRUE if the rectangle is a null rectangle; otherwise
		returns FALSE.

		A null rectangle has both the width and the height set to 0, that
		is right() == left() - 1 and bottom() == top() - 1.

		Note that if right() == left() and bottom() == top(), then the
		rectangle has width 1 and height 1.

		A null rectangle is also empty.

		A null rectangle is not valid.

		\sa isEmpty(), isValid()
	*/
	inline bool isNull(void) const
	{ return w == 0 && h == 0; }

	/*!
		Returns TRUE if the rectangle is empty; otherwise returns FALSE.

		An empty rectangle has a left() \> right() or top() \> bottom().

		An empty rectangle is not valid. \c {isEmpty() == !isValid()}

		\sa isNull(), isValid(), normalize()
	*/
	inline bool isEmpty(void) const
	{
		return w < 0 || h > 0;
	}

	/*!
		Returns a normalized rectangle, i.e. a rectangle that has a
		non-negative width and height.

		It swaps left and right if left() \> right(), and swaps top and
		bottom if top() \> bottom().

		\sa isValid()
	*/
	class FGRect  normalize(void)	const
	{
		return *this;
	}

	/*!
		Returns the left coordinate of the rectangle. Identical to x().

		\sa setLeft(), right(), topLeft(), bottomLeft()
	*/
	FGCOORD left(void) const { return x; }

	/*!
		Returns the right coordinate of the rectangle.

		\sa setRight(), left(), topRight(), bottomRight()
	*/
	FGCOORD right(void) const { return x+w; }
	/*!
		Returns the top coordinate of the rectangle. Identical to y().

		\sa setTop(), bottom(), topLeft(), topRight()
	*/
	FGCOORD top(void) const { return y; }
	/*!
		Returns the bottom coordinate of the rectangle.

		\sa setBottom(), top(), bottomLeft(), bottomRight()
	*/
	FGCOORD bottom(void) const { return y+h; }
	/*!
		Returns the top-left position of the rectangle.

		\sa setTopLeft(), moveTopLeft(), bottomRight(), left(), top()
	*/
	FGPoint topLeft(void) const { return FGPoint(x,y); }
	/*!
		Returns the bottom-right position of the rectangle.

		\sa setBottomRight(), moveBottomRight(), topLeft(), right(), bottom()
	*/
	FGPoint bottomRight(void) const { return FGPoint(int(x+w), int(y+h)); }
	/*!
		\sa setTopRight(), moveTopRight(), bottomLeft(), top(), right()
	*/
	FGPoint topRight(void) const { return FGPoint(x+w,y); }
	/*!
		Returns the bottom-left position of the rectangle.

		\sa setBottomLeft(), moveBottomLeft(), topRight(), bottom(), left()
	*/
	FGPoint bottomLeft(void) const { return FGPoint(x,y+h); }
	/*!
		Returns the center point of the rectangle.

		\sa moveCenter(), topLeft(), bottomRight(), topRight(), bottomLeft()
	*/
	FGPoint center(void) const { return FGPoint( int(x+w/2), int(y+h/2)); }
	/*!
		Returns the bounding rectangle of this rectangle and rectangle \a
		r.

		The bounding rectangle of a nonempty rectangle and an empty or
		invalid rectangle is defined to be the nonempty rectangle.

		\sa operator|=(), operator&(), intersects(), contains()
	*/
	class FGRect  operator|(const FGRect &r) const;
	/*!
		Returns the intersection of this rectangle and rectangle \a r.

		Returns an empty rectangle if there is no intersection.

		\sa operator&=(), operator|(), isEmpty(), intersects(), contains()
	*/
	class FGRect  operator&(const FGRect &r) const;
	/*!
		Unites this rectangle with rectangle \a r.
	*/
	class FGRect&  operator|=(const FGRect &r);
	/*!
		Intersects this rectangle with rectangle \a r.
	*/
	class FGRect&  operator&=(const FGRect &r);
	/*!
		Returns TRUE if the point \a p is inside or on the edge of the
		rectangle; otherwise returns FALSE.

		If \a proper is TRUE, this function returns TRUE only if \a p is
		inside (not on the edge).
	*/
	bool   contains( const FGPoint &p, bool proper=false ) const;
	/*!
		Returns TRUE if the point \a x, \a y is inside this rectangle;
		otherwise returns FALSE.

		If \a proper is TRUE, this function returns TRUE only if the point
		is entirely inside (not on the edge).
	*/
	bool   contains( int x, int y, bool proper=false ) const;
	/*!
		\overload

		Returns TRUE if the rectangle \a r is inside this rectangle;
		otherwise returns FALSE.

		If \a proper is TRUE, this function returns TRUE only if \a r is
		entirely inside (not on the edge).

		\sa unite(), intersect(), intersects()
	*/
	bool   contains( const FGRect &r, bool proper=false ) const;
	/*!
		Returns the bounding rectangle of this rectangle and rectangle \a
		r. \c {r.unite(s)} is equivalent to \c {r|s}.
	*/
	class FGRect unite( const FGRect &r ) const;
	/*!
		Returns the intersection of this rectangle and rectangle \a r.
		\c {r.intersect(s)} is equivalent to \c {r&s}.
	*/
	class FGRect intersect( const FGRect &r ) const;
	/*!
		Returns TRUE if this rectangle intersects with rectangle \a r
		(there is at least one pixel that is within both rectangles);
		otherwise returns FALSE.

		\sa intersect(), contains()
	*/
	bool   intersects( const FGRect &r ) const
	{
		return ( MAX( x, r.x ) <= MIN( w, r.w ) &&
			 MAX( y, r.y ) <= MIN( h, r.h ) );
	}
	friend bool operator==( const FGRect &, const FGRect & );
	friend bool operator!=( const FGRect &, const FGRect & );
};

/*!
	@relates FGRect
	@returns TRUE if \a r1 and \a r2 are equal; otherwise returns FALSE.
*/
bool operator==( const FGRect &r1, const FGRect &r2 );
/*!
	@relates FGRect
	@returns TRUE if \a r1 and \a r2 are different; otherwise returns FALSE.
*/
bool operator!=( const FGRect &r1, const FGRect &r2 );

/*!
	@brief The FGPointArray class provides an array of points.

	@ingroup images
	@ingroup graphics

	A FGPointArray is an array of FGPoint objects. In addition to the
	functions provided by QMemArray, FGPointArray provides some
	point-specific functions.

	For convenient reading and writing of the point data use
	setPoints(), putPoints(), point(), and setPoint().

	For geometry operations use boundingRect() and translate(). There
	is also the FGMatrix::map() function for more general
	transformations of FGPointArrays. You can also create arcs and
	ellipses with makeArc() and makeEllipse().

	Among others, FGPointArray is used by QPainter::drawLineSegments(),
	QPainter::drawPolyline(), QPainter::drawPolygon() and
	QPainter::drawCubicBezier().

	Note that because this class is a QMemArray, copying an array and
	modifying the copy modifies the original as well, i.e. a shallow
	copy. If you need a deep copy use copy() or detach(), for example:

	@code
	void drawGiraffe( const FGPointArray & r, QPainter * p )
	{
		FGPointArray tmp = r;
		tmp.detach();
		// some code that modifies tmp
		p->drawPoints( tmp );
	}
	@endcode

	If you forget the tmp.detach(), the const array will be modified.

	@sa FGMatrix
*/
struct FGPointArray
{
	unsigned int    vertices;
	FGPoint*        array;
	bool			allocated;

	/**
	*  Resizes the array of the FGPoints.
	*/
	void resize(unsigned new_size)
	{
		if (new_size != vertices)
		{
			if (allocated)
				delete [] array;

			array = new FGPoint[vertices = new_size];
			allocated = true;
		}
	}
	/*!
		Returns TRUE if the array is empty; otherwise returns FALSE.

		An empty rectangle has a zero number of vertices.

	*/
	inline bool isEmpty(void) const
	{
		return vertices <= 0;
	}
	/*!
		Constructs a null point array.
		\sa isNull()
	*/
	FGPointArray()
	{
		vertices=0;
		array = 0;
		allocated = false;
	}
	/*!
		Destroys the point array.
	*/
	~FGPointArray()
	{
		if (allocated)
        {
			if (array) delete [] array;
            allocated = 0;
        }
		array = 0;
	}
	/*!
		Constructs a point array with room for \a size points. Makes a
		null array if \a size == 0.

		\sa resize(), isNull()
	*/
	FGPointArray(unsigned int nPoints)
	: vertices(nPoints)
	{
		if (vertices > 0)
		{
			array = new FGPoint[vertices];
			allocated = true;
		}
	}
	/*!
		Constructs a deep copy of the point array \a a.
	*/
	FGPointArray(const class FGPointArray& old)
	{
		vertices = old.vertices;
		array = new FGPoint[vertices];;
		allocated = true;

		for(unsigned int i=0; i<vertices; i++)
			array[i] = old.array[i];
	}
	/*!
	  Constructs a point array with \a nPoints points, taken from the
	  \a FGPoint array.

	  Equivalent to setPoints(nPoints, points).
	*/
	FGPointArray(unsigned int nPoints, class FGPoint* points)
	: vertices(nPoints)
	{
		array = new FGPoint[vertices];
		allocated = true;
		for(unsigned int i=0; i<vertices; i++)
			array[i] = points[i];
	}
	/*!
	  Constructs a point array with \a nPoints points, taken from the
	  \a points array.

	  Equivalent to SetPoints(nPoints, points).
	*/
	FGPointArray(unsigned int nPoints, FGCOORD points[][2])
	: vertices(nPoints)
	{
		array = new FGPoint[vertices];
		allocated = true;

		SetPoints(vertices, (FGCOORD *)points);
	}
	/*!
		Constructs a point array from the rectangle \a r.

		If \a closed is FALSE, then the point array just contains the
		following four points in the listed order: r.topLeft(),
		r.topRight(), r.bottomRight() and r.bottomLeft().

		If \a closed is TRUE, then a fifth point is set to r.topLeft().
	*/
	FGPointArray( const FGRect &r, bool closed=false )
	{
		vertices = closed ? 5 : 4;
		array = new FGPoint[vertices];
		allocated = true;

		array[0] = r.topLeft();
		array[1] = r.bottomRight();
		array[2] = r.topRight();
		array[3] = r.bottomLeft();
		if (vertices == 5)
			array[4] = r.topLeft();
	}
	/*!
	  Constructs a point array with \a nPoints points, taken from the
	  \a points array.

	  Equivalent to SetPoints(nPoints, points).
	*/
	FGPointArray( unsigned int nPoints, const FGCOORD *points )
	 : vertices(nPoints)
	{
		array = new FGPoint[vertices];
		allocated = true;

		SetPoints(vertices, points);
	}
	/*!
	  \internal
	  Resizes the array to \a nPoints and sets the points in the array to
	  the values taken from \a points.

	  Returns TRUE if successful, or FALSE if the array could not be
	  resized (normally due to lack of memory).

	  The example code creates an array with two points (1,2) and (3,4):
	  \code
		static FGCOORD points[] = { 1,2, 3,4 };
		FGPointArray a;
		a.SetPoints( 2, points );
	  \endcode

	  \sa resize(), putPoints()
	*/
	void SetPoints( unsigned int nPoints, const FGCOORD *points )
	{
		resize(nPoints);
		for(unsigned int i=0; i<vertices; i++)
		{
			array[i].x = *points++;
			array[i].y = *points++;
		}
	}
	/*!
		\overload

		Resizes the array to \a nPoints and sets the points in the array
		to the values taken from the variable argument list.

		Returns TRUE if successful, or FALSE if the array could not be
		resized (typically due to lack of memory).

		The example code creates an array with two points (1,2) and (3,4):

		\code
		FGPointArray a;
		a.SetPoints( 2, 1,2, 3,4 );
		\endcode

		The points are given as a sequence of integers, starting with \a
		firstx then \a firsty, and so on.

		\sa resize(), putPoints()
	*/
	void SetPoints( unsigned nPoints, int firstx, int firsty, ... )
	{
		SetPoints(nPoints, (FGCOORD *) &firstx);
	}
	/*!
	  \internal
	  Resizes the array to \a nPoints and sets the points in the array to
	  the values taken from \a points.

	  Returns TRUE if successful, or FALSE if the array could not be
	  resized (normally due to lack of memory).

	  The example code creates an array with two points (1,2) and (3,4):
	  \code
		static FGCOORD points[] = { 1,2, 3,4 };
		FGPointArray a;
		a.SetPoints( 2, points );
	  \endcode

	  \sa resize(), putPoints()
	*/
	void SetPoints( unsigned int nPoints, const double *points )
	{
		resize(nPoints);
		for(unsigned int i=0; i<vertices; i++)
		{
			array[i].x = FGCOORD(*points++);
			array[i].y = FGCOORD(*points++);
		}
	}

	void SetPoints( unsigned int nPoints, const class FGPoint *points )
	{
		resize(nPoints);
		memcpy(array, points, sizeof(FGPoint)*nPoints);
	}

	/*!
		Translates all points in the array by \a (dx, dy).
	*/
	void translate( int dx, int dy )
	{
		register FGPoint *p = array;
		register int i = vertices;
		FGPoint pt( dx, dy );
		while ( i-- )
		{
			*p += pt;
			p++;
		}
	}
	/**
		Compute the bounding rect of polygon vertices.
	*/
	FGRect BoundingRect() const;

	/*!
		@return TRUE if the point \a p is inside this object;
		otherwise returns FALSE.
	*/
	bool   contains(const class FGPoint &p) const;
	/*!
		Returns TRUE if the point \a x, \a y is inside this rectangle;
		otherwise returns FALSE.
	*/
	bool   contains(int x, int y) const;
};

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#endif // shape_h
