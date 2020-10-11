/*
*/

#include "fastgl.h"
#include "listbox.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

void	listbox::corecture(void)
{
	if (count==1) // because of zero divide
		slider = 0;
	else
	slider = int(maxv*((float)curr/(count-1)));
	draw();
}

void listbox::lbCall(FGControl *T)
{
	listbox	*This = (listbox *)(T->GetParameter());
	int rc = int ((This->count) * ((float)(This->slider) / This->maxv));
	This->SetIndex(rc);
	This->Draw(1);
}

/**
The right place for calling this function is into the Window handler
on a CLICKLEFTEVENT case.
@param ccx the 'x' coordinate of mouse click
@param ccy the 'y' coordinate of mouse click
@return (-1) if thest fails or the index to the listbox container.
*/
int listbox::Test(int ccx, int ccy)
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
	Create an empty shape aka listbox and draw one
*/
listbox::listbox(int xs, int ys, int w, int h, int DropDown, FGWindow *wind)
	:	FGSlider(wind,
		xs+w+1, ys-1,   // geometry
		18, h*DropDown+2,
		0, h*DropDown-16,	// min..max
		1, (slider=0, &slider),     // steps, *value, CallBack
		lbCall,
		UNDEFINED_USER_DATA )
{
	SetParameter((void *) (this));
	// hide cursor
	cursor = 0;
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
	Draw(0);
}

/*
	redraw whole object on the screen
*/
void listbox::Draw(int pp)
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

	Cursor(cursor=pp);
	draw();
}

void listbox::Resize(int dx, int dy)
{
	owner->WindowBox(xx,yy,onew,oneh*drop_down,color_bg0);

	x += dx;
	onew += dx;

	int lines = dy/oneh;
	drop_down += lines;
	FGControl::Resize(0, lines*oneh);
	Draw(0);
}

/*
	draw on the cursor
*/
void listbox::ShowCursor(void)
{
	Cursor(1);
}

/*
	draw off the cursor
*/
void listbox::HideCursor(void)
{
	Cursor(0);
}

/*
*/
void listbox::Up(void)
{
	if (Enabled()==0) return;
	Cursor(0);
	int old = fpol;
	AdjustCursor(curr-1);
	if (fpol!=old)
		owner->WindowScrollDown(xx,yy, onew,oneh*(drop_down-1), oneh);
	Cursor(1);
	corecture();
}

/*
*/
void listbox::Down(void)
{
	if (Enabled()==0) return;
	Cursor(0);
	int old = fpol;
	AdjustCursor(curr+1);
	if (fpol!=old)
		owner->WindowScrollUp(xx,yy+oneh, onew,oneh*(drop_down-1), oneh);
	Cursor(1);
	corecture();
}

/*
*/
void listbox::Begin(void)
{
	if (Enabled()==0) return;
	Cursor(0);
	AdjustCursor(0);
	Draw(1);
}

/*
*/
void listbox::End(void)
{
	if (Enabled()==0) return;
	Cursor(0);
	AdjustCursor(count-1);
	Draw(1);
}

//
// Adjust 'curr', 'line', 'fpol'
//
int listbox::AdjustCursor(int pos)
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
void listbox::Cursor(int flag)
{
	if (Enabled() && fpol+drop_down>curr && fpol<=curr)
	{
		owner->WindowBox(xx,yy+oneh*line,onew,oneh,flag?color_bg1:color_bg0);
		DrawItem(xx, yy+oneh*line, curr, flag);
	}
	cursor = !!flag; // cursor state
}

/*
*/
void listbox::SetSize(int size)
{
	if (size<=0) line=curr=cursor=count=fpol=0;
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
void listbox::SetIndex(int pos)
{
	if (Enabled()==0) return;
	Cursor(0);
	AdjustCursor(pos);
	Cursor(1);
}

/*
*/
void listbox::SetIndexRel(int pos)
{
	if (Enabled()==0) return;
	Cursor(0);
	AdjustCursor(curr+pos);
	Cursor(1);
}

/**
The default listbox keypress handler. This function
handle keys for moving across the listbox. The right
place to call this is into the FGWindow handler
on KEYEVENT switch.
*/
int listbox::DoListBox(int key)
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
			Draw(1);
			return 5;
		case PGDOWN:
			SetIndexRel(drop_down);
			Draw(1);
			return 6;
	}
	return 0;
}

void listbox::draw(void)
{
	if (IsYourTabPage())
	{
		switch (status & 7)
	    {
		    case BNORMAL:
				box(1, 1, w - 3, h - 3, CScheme->slider);
	    		FGSlider::frame(0, 0, w, h);
		    	if (drop_down<count)
			    	FGSlider::frame(1, 1+((*val-minv)/steps),16,16,1);
				break;

	    	case BDISABLED:
		    	break;
		}
	    owner->StateLock();
		SAVE_CLIPPING(owner)
		DrawAsSelected();
	    owner->bitblit(x,y,0,0,w,h,this);
		RESTORE_CLIPPING(owner)
	    owner->WindowRepaint(x, y, w, h);
		owner->StateUnlock();
	}
}

#ifdef FG_NAMESPACE
}
#endif
