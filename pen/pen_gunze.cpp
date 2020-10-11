//---------------------------------------------------------------------------

#include <fastgl/fastgl.h>
#include "system.h"
#include "storage.h"
#include "iobase.h"
#include "ioserial.h"

#include "pen_gunze.h"

/*
	$Log$
	Revision 1.1  2005/05/12 10:52:35  majo
	Initial revision

	Revision 1.1  2004/11/09 12:08:59  majo
	*
	
	Revision 1.4  2004/10/15 07:52:06  majo
	vypisy
	C
	
	Revision 1.3  2004/10/15 07:34:55  majo
	iii
	
	Revision 1.2  2004/10/14 14:55:47  majo
	gunze funguje!!!..
	
	Revision 1.1  2004/10/14 13:35:19  majo
	gunze + TouchKey
	
*/

// 	$Id: pen_gunze.cpp 2086 2005-05-12 10:52:34Z majo $
//---------------------------------------------------------------------------

int GunzeDevice::ReadByte(void)
{
	int c = 1;
	if (com->Recv(&c, 1) != 1)
		return -1;
//	printf("%02X\n", c&255);
	return c;
}

bool GunzeDevice::Attach(void)
{
	com->Open();
	com->SetPort(9600, 'A', 8);
	com->SetNonBlocked();
//	com->SetDump(true);
	working = true;

	return true;
}

bool GunzeDevice::SendPacket(unsigned char *buf, int len)
{
	com->Send(buf,len);
	return true;
}

/**
	Reads the complete & valid packet
*/
bool GunzeDevice::GetPacket(void)
{
	int c;

	while ((c = ReadByte()) >= 0)
	{
		packet[count++] = c;

		if (c == 0x0D)
		{
      		count = 0;
			if (packet[0] == 'T' || packet[0] == 'R')
				return true;

			return false;
		}
	}
	return false;
}

void GunzeDevice::ReadInput(void)
{
	static int _x=0, _y=0;
	
	while (1)
	{
		if ( GetPacket() == false )
				break;

		if (packet[0] == 'T')
		{
//printf("down\n");
			button_down = true;
		}

		if (packet[0] == 'R')
		{
//printf("up\n");
			button_down = false;
		}

		_x = atoi(packet+1);
		_y = atoi(packet+6);

		if ( abs(_x - x) < 3 && abs(_y - y) < 3 && button_down == true)
			return;
		x = _x;
		y = _y;
		
		was_something_interesting = true;
//printf("Move %d %d\n", x,y);
	}
}

int GunzeDevice::GetInputEvent(int& type, int& key, int& x, int& y, int& button)
{
	if (working)
	{
		ReadInput();
	}
	if (was_something_interesting)
	{
		type = MOUSEEVENT;
		key = 0;
		button = button_down ? FG_BUTTON_LEFT : FG_BUTTON_NONE;
		x = this->x;
		y = this->y;
		was_something_interesting = false;
		return 1;
	}
	return 0;
}


