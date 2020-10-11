/****************************************************************************
** $Id: fgmatrix.h 4292 2006-02-21 10:00:48Z majo $
**
** Definition of FGMatrix class
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

#ifndef FGMATRIX_H
#define FGMATRIX_H


#ifdef FG_NAMESPACE
namespace fgl {
#endif


#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

class FGMatrix					// 2D transform matrix
{
  public:
    FGMatrix();
	FGMatrix(const FGMatrix& old) { memcpy(this, &old, sizeof(*this)); }
	FGMatrix( double m11, double m12, double m21, double m22,
	      double dx, double dy );

    void	setMatrix( double m11, double m12, double m21, double m22,
			   double dx,  double dy );

    double	m11() const { return _m11; }
    double	m12() const { return _m12; }
    double	m21() const { return _m21; }
    double	m22() const { return _m22; }
    double	dx()  const { return _dx; }
    double	dy()  const { return _dy; }

    void	map( int x, int y, int *tx, int *ty )	      const;
    void	map( double x, double y, double *tx, double *ty ) const;
    FGRect	mapRect( const FGRect & ) const;

	FGPoint	map( const FGPoint &p )	const { return operator *( p ); }
	FGCircle map( const FGCircle &c )	const { return operator *( c ); }
	FGSize  map( const FGSize &sz )	const { return operator *( sz ); }
	FGRect	map( const FGRect &r )	const { return mapRect ( r ); }
	FGPointArray map( const FGPointArray &a ) const { return operator * ( a ); }
	void	map(const FGPointArray& src, FGPointArray& dst) const;
	void	reset();
	bool	isIdentity() const;

    double	get_scale() { return _mscale; }
	FGMatrix   &translate( double dx, double dy );
	FGMatrix   &scale( double sx, double sy );
	FGMatrix   &scale( double s) { return scale(s,s); }
	FGMatrix   &shear( double sh, double sv );
	FGMatrix   &rotate( double a );

	void 	shift_x(double x) { _dx += x; }
	void 	shift_y(double y) { _dy += y; }
	void	SetDxDy(double dx, double dy) { _dx = dx; _dy = dy; }
	void 	zoom(double val) { _m11 *= val; _m22 *= val; }

	FGMatrix &scale_with_center( double sx, double sy, double center_x, double center_y)
	{
		scale(sx, sy);
		_dx = center_x + ((_dx - center_x) * sx);
		_dy = center_y + ((_dy - center_y) * sy);

		return *this;

	}

	bool isInvertible() const { return (_m11*_m22 - _m12*_m21) != 0; }
	double det() const { return _m11*_m22 - _m12*_m21; }

	FGMatrix	invert( bool * = 0 ) const;

	bool	operator==( const FGMatrix & ) const;
	bool	operator!=( const FGMatrix & ) const;
	FGMatrix   &operator*=( const FGMatrix & );

	/* we use matrix multiplication semantics here */
	FGCircle operator * (const FGCircle & ) const;
	FGPoint operator * (const FGPoint & ) const;
	FGSize operator * (const FGSize & ) const;
	FGPointArray operator *  ( const FGPointArray &a ) const;

	enum TransformationMode {	Points, Areas  };

	static void setTransformationMode( FGMatrix::TransformationMode m );
	static TransformationMode transformationMode();

  private:
	double	_m11, _m12;
	double	_m21, _m22;
	double	_dx,  _dy;
	double  _mscale;
	struct FGDoublePoint
	{
		double x,y;
	};
};

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#ifdef FG_NAMESPACE
}
#endif

#endif // FGMATRIX_H
