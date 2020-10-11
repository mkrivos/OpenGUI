/*
*/

#include "fastgl.h"
#include "fglistbox.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/*
	Create an empty shape aka listboxEx and draw one
*/
listboxEx::listboxEx(int xs, int ys, int w, int h, int DropDown, FGWindow *wind, void* user_data, XUIEnterHandlerInteger fnc)
 : FGControl(xs, ys, w+SLIDER_WIDTH, h*DropDown, 0, LISTBOX, 0, wind, 0, wind->GetPaper(), wind->GetInk(), user_data)
{
//	SetParameter((void *) (this));
//	XUIEnterHandlerInteger func = fnc;
	GetTempName(tempname,sizeof(tempname));
	RegisterOnEnterSignal(tempname, /*XUIEnterHandlerInteger fnc*/ (XUIEnterHandlerInteger)0, user_data);
	
	curr = 0;
	count = 0;
	// index on the screen
	line = 0;
	drop_down = DropDown;
	xx    = xs;
	yy    = ys;
	// size of one in the pixels
	onew = w;
	oneh = h;
	// first visible
	fpol = 0;
	color_bg0 = color_fg1 = wind->GetPaper();
	color_bg1 = color_fg0 = wind->GetInk();
	draw();
}

listboxEx::~listboxEx()
{
	DeregisterSignal(tempname, (void *)func, user_data);
}

void	listboxEx::corecture(void)
{
/*
	if (count==1) // because of zero divide
		slider = 0;
	else
		slider = int(maxv*((float)curr/(count-1)));
*/
	draw();
}

void listboxEx::ClickDown(int xx, int yy)
{
	xx -= GetX();
	yy -= GetY();
	if (xx >= 0)
	{
		if (xx >= w-SLIDER_WIDTH && xx < w)
		{ // slajder
::printf("slajder %d %d\n", xx,yy);
		}
		else
		{ // polozka
			int polozka = yy/oneh + fpol;
			SetIndex(polozka);
			if (GetSignalName())
			{
				CallClosure(GetSignalName(), this);
			}
::printf("polozka %d\n", polozka);
		}
	}
}

/**
The right place for calling this function is into the Window handler
on a CLICKLEFTEVENT case.
@param ccx the 'x' coordinate of mouse click
@param ccy the 'y' coordinate of mouse click
@return (-1) if thest fails or the index to the listboxEx container.
*/
int listboxEx::Test(int ccx, int ccy)
{
	int c;

	if (!Enabled())
		return -1;
	if (ccx<xx || ccx>(xx+onew))
		return -1;
	if (ccy<yy || ccy>(yy+oneh*drop_down))
		return -1;
	c = ((ccy-yy)/oneh)+fpol;
	if (c>=count)
		return -1;
	return c;
}

/*
	redraw whole object on the screen
*/
//void listboxEx::Draw(int pp)
//{
//	draw();
//}

void listboxEx::Resize(int dx, int dy)
{
	owner->WindowBox(xx,yy,onew,oneh*drop_down,color_bg0);

	x += dx;
	onew += dx;

	int lines = dy/oneh;
	drop_down += lines;
	FGControl::Resize(0, lines*oneh);
	draw();
}

/*
*/
void listboxEx::Up(void)
{
	if (Enabled()==0) return;
//	int old = fpol;
	AdjustCursor(curr-1);
	draw();
	corecture();
}

/*
*/
void listboxEx::Down(void)
{
	if (Enabled()==0) return;
	int old = fpol;
	AdjustCursor(curr+1);
	if (fpol!=old)
		owner->WindowScrollUp(xx,yy+oneh, onew,oneh*(drop_down-1), oneh);
	draw();
	corecture();
}

/*
*/
void listboxEx::Begin(void)
{
	if (Enabled()==0) return;
	AdjustCursor(0);
	draw();
}

/*
*/
void listboxEx::End(void)
{
	if (Enabled()==0) return;
	AdjustCursor(count-1);
	draw();
}

//
// Adjust 'curr', 'line', 'fpol'
//
int listboxEx::AdjustCursor(int pos)
{
	if (Enabled()==0) return 0;

	if (pos<0) pos = 0;
	if (pos>=count) pos = count-1;

	// if new is upper that top line
	if (pos<fpol)
	{
		fpol = pos;
		line = 0;
	}
	else if (pos>(fpol+drop_down-1))  // bottom last line
	{
		fpol = pos-(drop_down-1);
		line = drop_down-1;
	}
	else // in the visible area
	{
		line = pos-fpol;
	}
	curr = pos;
	corecture();
	return 1;
}

/*
*/
void listboxEx::SetSize(int size)
{
	if (size<=0) line = curr = count = fpol = 0;
	else
	{
		int shrink=0;
		if (size<count)	shrink = 1;
		count = size;
		if (shrink) End();
	}
}

/*
*/
void listboxEx::SetIndex(int pos)
{
	if (Enabled()==0) return;
	AdjustCursor(pos);
	draw();
}

/*
*/
void listboxEx::SetIndexRel(int pos)
{
	if (Enabled()==0) return;
	AdjustCursor(curr+pos);
	draw();
}

/**
The default listboxEx keypress handler. This function
handle keys for moving across the listboxEx. The right
place to call this is into the FGWindow handler
on KEYEVENT switch.
*/
int listboxEx::DoListBox(int key)
{
	switch(key)
	{
		case KUP:
			Up();
			return 1;
		case KDOWN:
			Down();
			return 2;
		case HOME:
			Begin();
			return 3;
		case END:
			End();
			return 4;
		case PGUP:
			SetIndexRel(-drop_down);
			draw();
			return 5;
		case PGDOWN:
			SetIndexRel(drop_down);
			draw();
			return 6;
	}
	return 0;
}

void listboxEx::draw(void)
{
	if (IsYourTabPage())
	{
		int i=0;

		owner->WindowBox(xx,yy,onew,oneh*drop_down,color_bg0);
		owner->WindowRect(xx-1,yy-1,onew+2,oneh*drop_down+2,color_fg0);

		if (count>0) while(i<drop_down && i+fpol<count)
		{
			DrawItem(xx, yy+oneh*i, i+fpol, 0);
			i++;
		}

		if (curr >= count)
			AdjustCursor(count-1);

		if (Enabled() && fpol+drop_down>curr && fpol<=curr)
		{
			owner->WindowBox(xx,yy+oneh*line,onew,oneh,color_bg1);
			DrawItem(xx, yy+oneh*line, curr, true);
		}
/*
		switch (status & 7)
		{
			case BNORMAL:
				box(1+w-SLIDER_WIDTH, 1, SLIDER_WIDTH - 3, h - 3, CScheme->slider);
				roller->frame(0+w-SLIDER_WIDTH, 0, SLIDER_WIDTH, h);
				if (drop_down<count)
					roller->frame(1, 1+((*roller->val - roller->minv) / roller->steps), SLIDER_WIDTH-2, SLIDER_WIDTH-2, 1);
				break;

			case BDISABLED:
				break;
		}
*/
		owner->StateLock();
		SAVE_CLIPPING(owner)
		DrawAsSelected();
		owner->bitblit(x+w-SLIDER_WIDTH,y,0,0,SLIDER_WIDTH,h,this);
		RESTORE_CLIPPING(owner)
		owner->WindowRepaint(x+w-SLIDER_WIDTH, y, SLIDER_WIDTH, h);
		owner->StateUnlock();
	}
}

#ifdef FG_NAMESPACE
}
#endif
