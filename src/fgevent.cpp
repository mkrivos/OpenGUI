/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2004  Marian Krivos

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
	Get & translate user events
*/


#include <xmlconfig.h>

#include "fgbase.h"
#include "fgevent.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

const char *FGEvent::eventName[]=
{
	  "NOEVENT", "KEYEVENT", "MOUSEEVENT", "QUITEVENT", "ACCELEVENT",
	  "TERMINATEEVENT", "CLOSEEVENT", "MOVEEVENT", "CLICKRIGHTEVENT",
	  "CLICKLEFTEVENT", "CLICKLMIDDLEEVENT", "DRAGLEFTEVENT", "DRAGRIGHTEVENT",
	  "GETFOCUSEVENT", "LOSTFOCUSEVENT", "INITEVENT",
	  "BUTTONHOLDEVENT", "WINDOWRESIZEEVENT","WINDOWMOVEEVENT",
	  "STARTDRAGLEFTEVENT", "STARTDRAGRIGHTEVENT", "REPAINTEVENT",
	  "NOTIFYEVENT", "ICONIZEEVENT", "CURSOROUTEVENT", "DBLCLICKRIGHTEVENT",
	  "DBLCLICKLEFTEVENT", "DBLCLICKMIDDLEEVENT", "MOUSEWHEELEVENT", "TABSWITCHEVENT",
// must by at the end
	  "NONSENSE"
};

/**
	Key to string mapping.
*/
struct KeyPair
{
	const char* string;
	unsigned int key;
} keys[] = {
{ "CR", 13 },
{ "ESC", 27 },
{ "TAB", 9 },
{ "LF", 10 },
{ "PRTRSC", PRTRSC },
{ "BACKSP", BACKSP },
{ "KUP", KUP },
{ "KDOWN", KDOWN },
{ "KRIGHT", KRIGHT },
{ "KLEFT", KLEFT },
{ "INSERT", INSERT },
{ "DEL", DEL },
{ "HOME", HOME },
{ "END", END },
{ "PGUP", PGUP },
{ "PGDOWN", PGDOWN},

{ "ALT_A", ALT_A },
{ "ALT_B", ALT_B },
{ "ALT_C", ALT_C },
{ "ALT_D", ALT_D },
{ "ALT_E", ALT_E },
{ "ALT_F", ALT_F },
{ "ALT_G", ALT_G },
{ "ALT_H", ALT_H },
{ "ALT_I", ALT_I },
{ "ALT_J", ALT_J },
{ "ALT_K", ALT_K },
{ "ALT_L", ALT_L },
{ "ALT_M", ALT_M },
{ "ALT_N", ALT_N },
{ "ALT_O", ALT_O },
{ "ALT_P", ALT_P },
{ "ALT_Q", ALT_Q },
{ "ALT_R", ALT_R },
{ "ALT_S", ALT_S },
{ "ALT_T", ALT_T },
{ "ALT_U", ALT_U },
{ "ALT_V", ALT_V },
{ "ALT_W", ALT_W },
{ "ALT_X", ALT_X },
{ "ALT_Y", ALT_Y },
{ "ALT_Z", ALT_Z },

{ "F01", F01},
{ "F02", F02},
{ "F03", F03},
{ "F04", F04},
{ "F05", F05},
{ "F06", F06},
{ "F07", F07},
{ "F08", F08},
{ "F09", F09},
{ "F10", F10},
{ "F11", F11},
{ "F12", F12},

{ "ALT_F01", ALT_F01},
{ "ALT_F02", ALT_F02},
{ "ALT_F03", ALT_F03},
{ "ALT_F04", ALT_F04},
{ "ALT_F05", ALT_F05},
{ "ALT_F06", ALT_F06},
{ "ALT_F07", ALT_F07},
{ "ALT_F08", ALT_F08},
{ "ALT_F09", ALT_F09},
{ "ALT_F10", ALT_F10},
{ "ALT_F11", ALT_F11},
{ "ALT_F12", ALT_F12},

{ "CTRL_F01", CTRL_F01},
{ "CTRL_F02", CTRL_F02},
{ "CTRL_F03", CTRL_F03},
{ "CTRL_F04", CTRL_F04},
{ "CTRL_F05", CTRL_F05},
{ "CTRL_F06", CTRL_F06},
{ "CTRL_F07", CTRL_F07},
{ "CTRL_F08", CTRL_F08},
{ "CTRL_F09", CTRL_F09},
{ "CTRL_F10", CTRL_F10},
{ "CTRL_F11", CTRL_F11},
{ "CTRL_F12", CTRL_F12},

{ "ALT_UP", ALT_UP},
{ "ALT_DOWN", ALT_DOWN},
{ "ALT_RIGHT", ALT_RIGHT},
{ "ALT_LEFT", ALT_LEFT},
{ "ALT_TAB", ALT_TAB},
{ "ALT_INSERT", ALT_INSERT},
{ "ALT_DEL", ALT_DEL},
{ "ALT_HOME", ALT_HOME},
{ "ALT_END", ALT_END},
{ "ALT_PGUP", ALT_PGUP},
{ "ALT_PGDOWN", ALT_PGDOWN},

{ "CTRL_UP", CTRL_UP},
{ "CTRL_DOWN", CTRL_DOWN},
{ "CTRL_RIGHT", CTRL_RIGHT},
{ "CTRL_LEFT", CTRL_LEFT},

{ "CTRL_TAB", CTRL_TAB },
{ "CTRL_INSERT", CTRL_INSERT },
{ "CTRL_DEL", CTRL_DEL },
{ "CTRL_HOME", CTRL_HOME },
{ "CTRL_END", CTRL_END },
{ "CTRL_PGUP", CTRL_PGUP },
{ "CTRL_PGDOWN", CTRL_PGDOWN },
{0, 0},
};

const char* FGEvent::GetKeySymbolName(unsigned int code)
{
	struct KeyPair* ptr = keys;

	if (code>=' ' && code<127)
		return "Character";

	while(ptr->key)
	{
		if (ptr->key == code)
			return ptr->string;
		ptr++;
	}
	return "Undefined";
}

FGInputDevice::FGInputDevice()
{
	working = false;
	init(GetXRes(), GetYRes());
	load_values();
	setCalibrationMatrix(imaginary, real, &matrix);
}

/*
 *
 *   Copyright (c) 2001, Carlos E. Vidales. All rights reserved.
 *
 *   This sample program was written and put in the public domain
 *    by Carlos E. Vidales.  The program is provided "as is"
 *    without warranty of any kind, either expressed or implied.
 *   If you choose to use the program within your own products
 *    you do so at your own risk, and assume the responsibility
 *    for servicing, repairing or correcting the program should
 *    it prove defective in any manner.
 *   You may copy and distribute the program's source code in any
 *    medium, provided that you also include in each copy an
 *    appropriate copyright notice and disclaimer of warranty.
 *   You may also modify this program and distribute copies of
 *    it provided that you include prominent notices stating
 *    that you changed the file(s) and the date of any change,
 *    and that you do not charge any royalties or licenses for
 *    its use.
 *
 *
 *
 *   File Name:  calibrate.c
 *
 *
 *   This file contains functions that implement calculations
 *    necessary to obtain calibration factors for a touch screen
 *    that suffers from multiple distortion effects: namely,
 *    translation, scaling and rotation.
 *
 *   The following set of equations represent a valid display
 *    point given a corresponding set of touch screen points:
 *
 *
 *                                              /-     -\
 *              /-    -\     /-            -\   |       |
 *              |      |     |              |   |   Xs  |
 *              |  Xd  |     | A    B    C  |   |       |
 *              |      |  =  |              | * |   Ys  |
 *              |  Yd  |     | D    E    F  |   |       |
 *              |      |     |              |   |   1   |
 *              \-    -/     \-            -/   |       |
 *                                              \-     -/
 *
 *
 *    where:
 *
 *           (Xd,Yd) represents the desired display point
 *                    coordinates,
 *
 *           (Xs,Ys) represents the available touch screen
 *                    coordinates, and the matrix
 *
 *           /-   -\
 *           |A,B,C|
 *           |D,E,F| represents the factors used to translate
 *           \-   -/  the available touch screen point values
 *                    into the corresponding display
 *                    coordinates.
 *
 *
 *    Note that for practical considerations, the utilitities
 *     within this file do not use the matrix coefficients as
 *     defined above, but instead use the following
 *     equivalents, since floating point math is not used:
 *
 *            A = An/Divider
 *            B = Bn/Divider
 *            C = Cn/Divider
 *            D = Dn/Divider
 *            E = En/Divider
 *            F = Fn/Divider
 *
 *
 *
 *    The functions provided within this file are:
 *
 *          setCalibrationMatrix() - calculates the set of factors
 *                                    in the above equation, given
 *                                    three sets of test points.
 *               getDisplayPoint() - returns the actual display
 *                                    coordinates, given a set of
 *                                    touch screen coordinates.
 * translateRawScreenCoordinates() - helper function to transform
 *                                    raw screen points into values
 *                                    scaled to the desired display
 *                                    resolution.
 *
 *
*/

/**
 *
 *     Function: setCalibrationMatrix()
 *
 *  Description: Calling this function with valid input data
 *                in the display and screen input arguments
 *                causes the calibration factors between the
 *                screen and display points to be calculated,
 *                and the output argument - matrixPtr - to be
 *                populated.
 *
 *               This function needs to be called only when new
 *                calibration factors are desired.
 *
 *
 *  Argument(s): displayPtr (input) - Pointer to an array of three
 *                                     sample, reference points.
 *               screenPtr (input) - Pointer to the array of touch
 *                                    screen points corresponding
 *                                    to the reference display points.
 *               matrixPtr (output) - Pointer to the calibration
 *                                     matrix computed for the set
 *                                     of points being provided.
 *
 *
 *  From the article text, recall that the matrix coefficients are
 *   resolved to be the following:
 *
 *
 *      Divider =  (Xs0 - Xs2)*(Ys1 - Ys2) - (Xs1 - Xs2)*(Ys0 - Ys2)
 *
 *
 *
 *                 (Xd0 - Xd2)*(Ys1 - Ys2) - (Xd1 - Xd2)*(Ys0 - Ys2)
 *            A = ---------------------------------------------------
 *                                   Divider
 *
 *
 *                 (Xs0 - Xs2)*(Xd1 - Xd2) - (Xd0 - Xd2)*(Xs1 - Xs2)
 *            B = ---------------------------------------------------
 *                                   Divider
 *
 *
 *                 Ys0*(Xs2*Xd1 - Xs1*Xd2) +
 *                             Ys1*(Xs0*Xd2 - Xs2*Xd0) +
 *                                           Ys2*(Xs1*Xd0 - Xs0*Xd1)
 *            C = ---------------------------------------------------
 *                                   Divider
 *
 *
 *                 (Yd0 - Yd2)*(Ys1 - Ys2) - (Yd1 - Yd2)*(Ys0 - Ys2)
 *            D = ---------------------------------------------------
 *                                   Divider
 *
 *
 *                 (Xs0 - Xs2)*(Yd1 - Yd2) - (Yd0 - Yd2)*(Xs1 - Xs2)
 *            E = ---------------------------------------------------
 *                                   Divider
 *
 *
 *                 Ys0*(Xs2*Yd1 - Xs1*Yd2) +
 *                             Ys1*(Xs0*Yd2 - Xs2*Yd0) +
 *                                           Ys2*(Xs1*Yd0 - Xs0*Yd1)
 *            F = ---------------------------------------------------
 *                                   Divider
 *
 *
 *       Return: OK - the calibration matrix was correctly
 *                     calculated and its value is in the
 *                     output argument.
 *               NOT_OK - an error was detected and the
 *                         function failed to return a valid
 *                         set of matrix values.
 *                        The only time this sample code returns
 *                        NOT_OK is when Divider == 0
 *
 *
 *
 *                 NOTE!    NOTE!    NOTE!
 *
 *  setCalibrationMatrix() and getDisplayPoint() will do fine
 *  for you as they are, provided that your digitizer
 *  resolution does not exceed 10 bits (1024 values).  Higher
 *  resolutions may cause the integer operations to overflow
 *  and return incorrect values.  If you wish to use these
 *  functions with digitizer resolutions of 12 bits (4096
 *  values) you will either have to a) use 64-bit signed
 *  integer variables and math, or b) judiciously modify the
 *  operations to scale results by a factor of 2 or even 4.
 *
 *
 */
int FGInputDevice::setCalibrationMatrix(POINT * displayPtr, POINT * screenPtr, MATRIX * matrixPtr)
{
	int retValue = true;

	matrixPtr->Divider = (((double)screenPtr[0].x - (double)screenPtr[2].x) * ((double)screenPtr[1].y - (double)screenPtr[2].y)) -
		(((double)screenPtr[1].x - (double)screenPtr[2].x) * ((double)screenPtr[0].y - (double)screenPtr[2].y));

	if (matrixPtr->Divider == 0)
	{
		retValue = false;
	}
	else
	{
		matrixPtr->An = (((double)displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
			(((double)displayPtr[1].x - displayPtr[2].x) * ((double)screenPtr[0].y - screenPtr[2].y));

		matrixPtr->Bn = (((double)screenPtr[0].x - screenPtr[2].x) * ((double)displayPtr[1].x - displayPtr[2].x)) -
			(((double)displayPtr[0].x - displayPtr[2].x) * ((double)screenPtr[1].x - screenPtr[2].x));

		matrixPtr->Cn = ((double)screenPtr[2].x * displayPtr[1].x - (double)screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
			((double)screenPtr[0].x * displayPtr[2].x - (double)screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
			((double)screenPtr[1].x * displayPtr[0].x - (double)screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y;

		matrixPtr->Dn = (((double)displayPtr[0].y - displayPtr[2].y) * ((double)screenPtr[1].y - screenPtr[2].y)) -
			(((double)displayPtr[1].y - displayPtr[2].y) * ((double)screenPtr[0].y - screenPtr[2].y));

		matrixPtr->En = (((double)screenPtr[0].x - screenPtr[2].x) * ((double)displayPtr[1].y - displayPtr[2].y)) -
			(((double)displayPtr[0].y - displayPtr[2].y) * ((double)screenPtr[1].x - screenPtr[2].x));

		matrixPtr->Fn = ((double)screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * (double)displayPtr[2].y) * screenPtr[0].y +
			((double)screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * (double)displayPtr[0].y) * screenPtr[1].y +
			((double)screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * (double)displayPtr[1].y) * screenPtr[2].y;
	}

	return (retValue);
}

/**
 *
 *     Function: getDisplayPoint()
 *
 *  Description: Given a valid set of calibration factors and a point
 *                value reported by the touch screen, this function
 *                calculates and returns the true (or closest to true)
 *                display point below the spot where the touch screen
 *                was touched.
 *
 *
 *
 *  Argument(s): displayPtr (output) - Pointer to the calculated
 *                                      (true) display point.
 *               screenPtr (input) - Pointer to the reported touch
 *                                    screen point.
 *               matrixPtr (input) - Pointer to calibration factors
 *                                    matrix previously calculated
 *                                    from a call to
 *                                    setCalibrationMatrix()
 *
 *
 *  The function simply solves for Xd and Yd by implementing the
 *   computations required by the translation matrix.
 *
 *                                              /-     -\
 *              /-    -\     /-            -\   |       |
 *              |      |     |              |   |   Xs  |
 *              |  Xd  |     | A    B    C  |   |       |
 *              |      |  =  |              | * |   Ys  |
 *              |  Yd  |     | D    E    F  |   |       |
 *              |      |     |              |   |   1   |
 *              \-    -/     \-            -/   |       |
 *                                              \-     -/
 *
 *  It must be kept brief to avoid consuming CPU cycles.
 *
 *
 *       Return: OK - the display point was correctly calculated
 *                     and its value is in the output argument.
 *               NOT_OK - an error was detected and the function
 *                         failed to return a valid point.
 *
 *
 *
 *                 NOTE!    NOTE!    NOTE!
 *
 *  setCalibrationMatrix() and getDisplayPoint() will do fine
 *  for you as they are, provided that your digitizer
 *  resolution does not exceed 10 bits (1024 values).  Higher
 *  resolutions may cause the integer operations to overflow
 *  and return incorrect values.  If you wish to use these
 *  functions with digitizer resolutions of 12 bits (4096
 *  values) you will either have to a) use 64-bit signed
 *  integer variables and math, or b) judiciously modify the
 *  operations to scale results by a factor of 2 or even 4.
 *
 *
 */
int FGInputDevice::getDisplayPoint(POINT * displayPtr, POINT * screenPtr, MATRIX * matrixPtr)
{
	int retValue = true;


	if (matrixPtr->Divider != 0)
	{
		/* Operation order is important since we are doing integer */
		/*  math. Make sure you add all terms together before      */
		/*  dividing, so that the remainder is not rounded off     */
		/*  prematurely.                                           */

		displayPtr->x = (int) (((matrixPtr->An * screenPtr->x) + (matrixPtr->Bn * screenPtr->y) + matrixPtr->Cn) / matrixPtr->Divider);
		displayPtr->y = (int) (((matrixPtr->Dn * screenPtr->x) + (matrixPtr->En * screenPtr->y) + matrixPtr->Fn) / matrixPtr->Divider);
	}
	else
	{
		retValue = false;
	}

	return (retValue);

}

void FGInputDevice::init(int xmax, int ymax)
{
	imaginary[0].x = xmax / 100 * 15;
	imaginary[0].y = ymax / 100 * 15;

	imaginary[1].x = xmax / 100 * 50;
	imaginary[1].y = ymax / 100 * 80;

	imaginary[2].x = xmax / 100 * 85;
	imaginary[2].y = ymax / 100 * 10;

	real[0] = imaginary[0];
	real[1] = imaginary[1];
	real[2] = imaginary[2];
}

void FGInputDevice::load_values(void)
{
	ConfigInterface* cfg = new Config(".calibration_data.rc");

	cfg->ReadInt("im_x0", imaginary[0].x);
	cfg->ReadInt("im_y0", imaginary[0].y);
	cfg->ReadInt("im_x1", imaginary[1].x);
	cfg->ReadInt("im_y1", imaginary[1].y);
	cfg->ReadInt("im_x2", imaginary[2].x);
	cfg->ReadInt("im_y2", imaginary[2].y);

	cfg->ReadInt("re_x0", real[0].x);
	cfg->ReadInt("re_y0", real[0].y);
	cfg->ReadInt("re_x1", real[1].x);
	cfg->ReadInt("re_y1", real[1].y);
	cfg->ReadInt("re_x2", real[2].x);
	cfg->ReadInt("re_y2", real[2].y);

	delete cfg;
}

void FGInputDevice::save_values(void)
{
	ConfigInterface* cfg = new Config(".calibration_data.rc");

	cfg->WriteInt("im_x0", imaginary[0].x);
	cfg->WriteInt("im_y0", imaginary[0].y);
	cfg->WriteInt("im_x1", imaginary[1].x);
	cfg->WriteInt("im_y1", imaginary[1].y);
	cfg->WriteInt("im_x2", imaginary[2].x);
	cfg->WriteInt("im_y2", imaginary[2].y);

	cfg->WriteInt("re_x0", real[0].x);
	cfg->WriteInt("re_y0", real[0].y);
	cfg->WriteInt("re_x1", real[1].x);
	cfg->WriteInt("re_y1", real[1].y);
	cfg->WriteInt("re_x2", real[2].x);
	cfg->WriteInt("re_y2", real[2].y);

	delete cfg;
}

void FGInputDevice::Scale(int& x, int& y)
{
	POINT point = { x, y }, point2;
	getDisplayPoint(&point2, &point, &matrix);
	x = point2.x;
	y = point2.y;
}

void FGInputDevice::Calibration(void)
{
	if (working)
	{
		gprintf(0,CWHITE,200, 68,"                                                     ");
		gprintf(0,CWHITE,200, 84,"       TOUCHSCREEN CALIBRATION PROCEDURE             ");
		gprintf(0,CWHITE,200,100,"                                                     ");
		gprintf(0,CWHITE,200,116," Click by PEN on next 3 points - be accurate please! ");
		gprintf(0,CWHITE,200,132,"                                                     ");

		init( GetXRes(), GetYRes() );

		for(int i = 0; i < 3; i++)
		{
			DoPoint(imaginary+i, real+i);
		}
		save_values();
		setCalibrationMatrix(imaginary, real, &matrix);
	}
}

void FGInputDevice::DoPoint(POINT * displayPtr, POINT * screenPtr)
{
	int rc, type, x, y, buttons;
	long key;

	delay(800);
	while ( (rc = GetInputEvent(type,key,x,y,buttons)) != 0)
		delay(1);

	fill_circle(displayPtr->x, displayPtr->y, 48, 0, _GSET);
	fill_circle(displayPtr->x, displayPtr->y, 24, CWHITED, _GSET);
	draw_circle(displayPtr->x, displayPtr->y, 4, CWHITE, _GSET);
	draw_circle(displayPtr->x, displayPtr->y, 2, 0, _GSET);

	while ( (rc = GetInputEvent(type, key, x, y, buttons)) == 0)
		delay(1);
	Snd(1000, 100);
	delay(200);
	while ( (rc = GetInputEvent(type, key, x, y, buttons)) == 0)
		delay(1);
	fill_circle(displayPtr->x, displayPtr->y, 48, 0, _GSET);

	screenPtr->x = x;
	screenPtr->y = y;
}

#ifdef FG_NAMESPACE
}
#endif

