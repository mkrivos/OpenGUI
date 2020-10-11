//---------------------------------------------------------------------------
//
//
//
//
//---------------------------------------------------------------------------

#include "fgbase.h"
#include "fgcolor.h"

#if defined(BPP32)

#include "agg_pixfmt_rgba32.h"
#define pix_format agg::pix_format_bgra32
typedef agg::pixfmt_bgra32 pixfmt;
typedef agg::rgba8 color_type;
typedef agg::order_bgra32 component_order;

#elif defined(BPP16)

#include "agg_pixfmt_rgb565.h"
#define pix_format agg::pix_format_rgb565
typedef agg::pixfmt_rgb565 pixfmt;
typedef agg::rgba8 color_type;

#elif defined(BPP15)

#include "agg_pixfmt_rgb555.h"
#define pix_format agg::pix_format_rgb555
typedef agg::pixfmt_rgb555 pixfmt;
typedef agg::rgba8 color_type;

#else

#include "agg_pixfmt_gray8.h"
#define pix_format agg::pix_format_gray8
typedef agg::pixfmt_gray8 pixfmt;
typedef agg::gray8 color_type;

#endif

#include "fggradient.h"

#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_u.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_span_gradient.h"
#include "agg_span_interpolator_linear.h"

#define RESOLUTION		256.0

#ifdef FG_NAMESPACE
namespace fgl {
#endif

class gradient_polymorphic_wrapper_base
{
	public:
		virtual int calculate(int x, int y, int) const = 0;
		FGGradient::FGGradientMode mode;
};

template<class GradientF>
	class gradient_polymorphic_wrapper : public gradient_polymorphic_wrapper_base
{
	public:
		gradient_polymorphic_wrapper()
		{
			mode = FGGradient::Wrap;
		}

		virtual int calculate(int x, int y, int d) const
		{
			int ret = m_gradient.calculate(x, y, d);

			switch((int)mode)
			{
				case FGGradient::Reflect:
				{
					int d2 = d << 1;
					ret =  ret % d2;
					if(ret <  0) ret += d2;
					if(ret >= d) ret  = d2 - ret;
					return ret;
				}
				case FGGradient::Repeat:
				{
					ret =  ret % d;
					if(ret < 0) ret += d;
					return ret;
				}
			}

			return ret;
		}

		GradientF m_gradient;
};


// The gradient objects declarations
//----------------
gradient_polymorphic_wrapper<agg::gradient_radial>       gr_radial;
gradient_polymorphic_wrapper<agg::gradient_diamond>      gr_diamond;
gradient_polymorphic_wrapper<agg::gradient_x>            gr_x;
gradient_polymorphic_wrapper<agg::gradient_xy>           gr_xy;
gradient_polymorphic_wrapper<agg::gradient_sqrt_xy>      gr_sqrt_xy;
gradient_polymorphic_wrapper<agg::gradient_conic>        gr_conic;


// Pixel format and basic renderers.
//-----------------
typedef agg::renderer_base<pixfmt> renderer_base_type;


// Gradient shape function (linear, radial, custom, etc)
//-----------------
typedef gradient_polymorphic_wrapper_base gradient_func_type;


// Span interpolator. This object is used in all span generators
// that operate with transformations during iterating of the spans,
// for example, image transformers use the interpolator too.
//-----------------
typedef agg::span_interpolator_linear<> interpolator_type;


// Span allocator is an object that allocates memory for
// the array of colors that will be used to render the
// color spans. One object can be shared between different
// span generators.
//-----------------
typedef agg::span_allocator<color_type> span_allocator_type;


// Finally, the gradient span generator working with the agg::rgba8
// color type.
// The 4-th argument is the color function that should have
// the [] operator returning the color in range of [0...255].
// In our case it will be a simple look-up table of 256 colors.
//-----------------
typedef agg::span_gradient<color_type,
			   interpolator_type,
			   gradient_func_type,
			   const color_type*,
			   span_allocator_type> span_gradient_type;


// The gradient scanline renderer type
//-----------------
typedef agg::renderer_scanline_aa<renderer_base_type,
									  span_gradient_type> renderer_gradient_type;

//---------------------------------------------------------------------------


FGGradient::FGGradient()
{
	init();
}

FGGradient::FGGradient(FGColor& _start_color, FGColor& _end_color)
{
	init();

	agg::rgba8 start_color(_start_color.GetRed(), _start_color.GetGreen(), _start_color.GetBlue());
	agg::rgba8 end_color(_end_color.GetRed(), _end_color.GetGreen(), _end_color.GetBlue() );

	array = malloc(sizeof(color_type) * 256);                // The gradient colors
	color_type* color_array = (color_type* )array;                // The gradient colors

	unsigned i;
	for(i = 0; i < 256; ++i)
	{
		color_array[i] = start_color.gradient(end_color, i / 256.0);
	}
}

FGGradient::FGGradient(FGColor& _start_color, FGColor& _middle_color, FGColor& _end_color)
{
	init();

	agg::rgba8 start_color(_start_color.GetRed(), _start_color.GetGreen(), _start_color.GetBlue());
	agg::rgba8 middle_color(_middle_color.GetRed(), _middle_color.GetGreen(), _middle_color.GetBlue() );
	agg::rgba8 end_color(_end_color.GetRed(), _end_color.GetGreen(), _end_color.GetBlue() );

	array = malloc(sizeof(color_type) * 256);                // The gradient colors
	color_type* color_array = (color_type* )array;                // The gradient colors

	unsigned i;
	for(i = 0; i < 128; ++i)
	{
		color_array[i] = start_color.gradient(middle_color, i / 128.0);
	}
	for(; i < 256; ++i)
	{
		color_array[i] = middle_color.gradient(end_color, (i - 128) / 128.0);
	}
}

FGGradient::~FGGradient()
{
	if (array)
	{
		free(array);
	}
	init();
}

void FGGradient::init()
{
	fnc = Linear;
//	mode = Wrap;
	array = 0;
}

void FGGradient::Draw(FGDrawBuffer& image, FGRect* rectangle)
{
	FGPoint start(0,0);
	FGPoint end(image.GetW(), image.GetH());
	Draw(image, start, end);
}

void FGGradient::Draw(FGDrawBuffer& image, FGPoint& _start_point, FGPoint& _end_point, FGRect* rectangle)
{
	color_type* color_array = (color_type* )array;                // The gradient colors
	unsigned char* buffer = (unsigned char*) image.GetArray();
	int w = image.GetW();
	int h = image.GetH();

	agg::rendering_buffer rbuf(buffer,
							   w,
							   h,
							   sizeof(FGPixel)*w);

	// Common declarations (pixel format and basic renderer).
	//----------------
	pixfmt pixf(rbuf);
	renderer_base_type rbase(pixf);

	gradient_polymorphic_wrapper_base* gr_ptr = &gr_x;


	switch(fnc)
	{
		case Radial: gr_ptr = &gr_radial;        break;
		case Linear: gr_ptr = &gr_x;        break;
		case Diamond: gr_ptr = &gr_diamond; break;
		case LinearXY: gr_ptr = &gr_xy;     break;
		case Sqrt: gr_ptr = &gr_sqrt_xy;    break;
		case Conic: gr_ptr = &gr_conic;     break;
	}

	gr_ptr->mode = mode;

	agg::trans_affine   gradient_mtx;                    // Affine transformer
	interpolator_type   span_interpolator(gradient_mtx); // Span interpolator
	span_allocator_type span_allocator;                  // Span Allocator

	// Declare the gradient span itself.
	// The last two arguments are so called "d1" and "d2"
	// defining two distances in pixels, where the gradient starts
	// and where it ends. The actual meaning of "d1" and "d2" depands
	// on the gradient function.
	//----------------
	span_gradient_type span_gradient(span_allocator,
									 span_interpolator,
									 *gr_ptr,
									 color_array,
									 0, RESOLUTION);

	// The gradient renderer
	//----------------
	renderer_gradient_type ren_gradient(rbase, span_gradient);


	// The rasterizing/scanline stuff
	//----------------
	agg::rasterizer_scanline_aa<> ras;
	agg::scanline_u8 sl;

	// Calculate the affine transformation matrix for the linear gradient
	// from (x1, y1) to (x2, y2). gradient_d2 is the "base" to scale the
	// gradient. Here d1 must be 0.0, and d2 must equal gradient_d2.
	//---------------------------------------------------------------
	double dx = _end_point.x - _start_point.x;
	double dy = _end_point.y - _start_point.y;
	gradient_mtx.reset();
	gradient_mtx *= agg::trans_affine_scaling(sqrt(dx * dx + dy * dy) / RESOLUTION);
	gradient_mtx *= agg::trans_affine_rotation(atan2(dy, dx));
	gradient_mtx *= agg::trans_affine_translation(_start_point.x, _start_point.y);
	gradient_mtx.invert();

	double rx[4], ry[4];
	if (rectangle)
	{
		rx[0] = rectangle->x;
		ry[0] = rectangle->y;
		rx[1] = rectangle->x+rectangle->w;
		ry[1] = rectangle->y;
		rx[2] = rectangle->x+rectangle->w;
		ry[2] = rectangle->y+rectangle->h;
		rx[3] = rectangle->x;
		ry[3] = rectangle->y+rectangle->h;
	}
	else
	{
		rx[0] = 0;
		ry[0] = 0;
		rx[1] = w;
		ry[1] = 0;
		rx[2] = w;
		ry[2] = h;
		rx[3] = 0;
		ry[3] = h;
	}
	ras.add_xy(rx,ry,4);

	agg::render_scanlines(ras, sl, ren_gradient);
}
#ifdef FG_NAMESPACE
}
#endif

