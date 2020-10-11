#ifndef fggradientH
#define fggradientH

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/* HJH modification: protect compiler options for structure alignment and enum
 * size if the compiler is Borland C++ */
#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

/**
	Creates the gradient object.
	@image html gradient.png
*/
class FGGradient
{
	public:
		/**
		*/
		enum FGGradientMode { Wrap, Reflect, Repeat };
		/**
		*/
		enum FGGradientFunction { Linear, LinearXY, Radial, Diamond, Conic, Sqrt };

	private:
		FGGradientMode mode;
		FGGradientFunction fnc;

		void* array;                // The gradient colors

		void init();

	public:
		FGGradient();
		~FGGradient();

		/**
			Build bicolor gradient.
		*/
		FGGradient(FGColor& _start_color, FGColor& _end_color);
		/**
			Build tricolor gradient.
		*/
		FGGradient(FGColor& _start_color, FGColor& _middle_color, FGColor& _end_color);

		/**
			Set drawing function (Linear, LinearXY, Radial, Diamond, Conic or Sqrt).
		*/
		void SetFunction(FGGradientFunction f) { fnc = f; }
		/**
			Set wrapping mode ( Wrap, Reflect or Repeat ).
		*/
		void SetMode(FGGradientMode m) { mode = m; }

		/**
			Draw the rectangle with color gradient to the buffer.
			You can set the start and the end point for gradient.
			@param _start_point the start point for gradient
			@param _end_point the end point for gradient
			@param image desired object
			@param rectangle the rectangle that describes painted area (0 = flat)
		*/
		void Draw(FGDrawBuffer& image, FGPoint& _start_point, FGPoint& _end_point, FGRect* rectangle=0);

		/**
			Draw the rectangle with color gradient to the buffer.
			You can't set the start and the end point for gradient, it is the
			top left for start point and the bottom right for the end point.
			@param image desired object
			@param rectangle the rectangle that describes painted area (0 = flat)
		*/
		void Draw(FGDrawBuffer& image, FGRect* rectangle=0);
};

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

//---------------------------------------------------------------------------
#endif
