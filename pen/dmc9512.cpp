//---------------------------------------------------------------------------

#include <fastgl/fastgl.h>
#include "system.h"
#include "storage.h"
#include "iobase.h"
#include "ioserial.h"
#include "dmc9512.h"

//---------------------------------------------------------------------------


/*
 * Be sure to set vmin appropriately for your device's protocol. You want to
 * read a full packet before returning
 */
const char *default_options[] =
{
	/*	"Device", "/dev/ttyS1",*/
	"BaudRate", "19200",
	"StopBits", "1",
	"DataBits", "8",
	"Parity", "None",
	"Vmin", "3",
	"Vtime", "1",
	"FlowControl", "None",
	NULL,
};

PenMountPrivate::PenMountPrivate()
{
	x = y = 0;
	lex_mode = PenMount_byte0;
	button_down = false;
	lex_mode = PenMount_byte0;
}

int PenMountDevice::ReadByte(void)
{
	int c = 1;
	if (com->Recv(&c, 1) != 1)
		return -1;
	printf("%02X\n", c&255);
	return c;
}

bool PenMountDevice::Attach(void)
{
	/* echo Success Code */
	unsigned char	buf[5] = { 'D', 'G', 0x02, 0x80, 0x00 };

	com->Open();
	com->SetPort(19200, 'A', 8);
	com->SetNonBlocked();
//	com->SetDump(true);

	if ( SendPacket(buf, 5) == true )
	{
		/* wait for right response */
		priv.lex_mode = PenMount_Response0;
		if (GetPacket() == true )
		{
			if (( priv.packet[0] == 0xff ) && ( priv.packet[1] == 0x70 ) )
			{
				/* disable DMC9512 */
				buf[2] = 0x0a;
				buf[3] = 0x00;
				buf[4] = 0x00;
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* set screen width */
				buf[2] = 0x02;
				buf[3] = 0x03; /*(priv.screen_width & 0x0fff) >> 8;*/
				buf[4] = 0xfc; /*priv.screen_width & 0xff;*/
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* set screen height */
				buf[2] = 0x02;
				buf[3] = 0x13; /*(priv.screen_height & 0x0fff) >> 8;*/
				buf[4] = 0xfc; /*priv.screen_height & 0xff;*/
				SendPacket(buf,5);

				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* Set Calibration Data */
				/* Set X-coordinate of the Left Top corner */
				buf[2] = 0x02;
				buf[3] = 0x40;
				buf[4] = 0x03;
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* Set Y-coordinate of the Left Top corner */
				buf[2] = 0x02;
				buf[3] = 0x50;
				buf[4] = 0x03;
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* Set X-coordinate of the Right bottom corner */
				buf[2] = 0x02;
				buf[3] = 0x60;
				buf[4] = 0xfc;
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* Set Y-coordinate of the Right bottom corner */
				buf[2] = 0x02;
				buf[3] = 0x70;
				buf[4] = 0xfc;
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* Set Screen Width Again */
				buf[2] = 0x02;
				buf[3] = 0x03;
				buf[4] = 0xfc;
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* Set Screen Height Again */
				buf[2] = 0x02;
				buf[3] = 0x13;
				buf[4] = 0xfc;
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

				/* enable DMC9512 */
				buf[2] = 0x0a;
				buf[3] = 0x01;
				buf[4] = 0x00;
				SendPacket(buf,5);
				priv.lex_mode = PenMount_Response0;
				GetPacket();

			}
		}
	}
				working = true;
	priv.lex_mode = PenMount_byte0;
	priv.button_down = false;
	return true;
}

bool PenMountDevice::SendPacket(unsigned char *buf, int len)
{
	com->Send(buf,len);
   System::Delay(50);
	return true;
}

bool PenMountDevice::GetPacket(void)
{
	int count = 0;
	int c;

	while ((c = ReadByte()) >= 0)
	{
		/*
		 * fail after 500 bytes so the server doesn't hang forever if a
		 * device sends bad data.
		 */
		if (count++ > 500)
			return false;

		switch (priv.lex_mode)
		{
		case PenMount_byte0:
			if ( c != 0xff && c != 0xbf )
					return false;
			priv.packet[0] = (unsigned char) c;
			priv.lex_mode = PenMount_byte1;
			break;

		case PenMount_byte1:
			priv.packet[1] = (unsigned char) c;
			priv.lex_mode = PenMount_byte2;
			break;

		case PenMount_byte2:
			priv.packet[2] = (unsigned char) c;
			priv.lex_mode = PenMount_byte0;
			if (( priv.packet[2] == 0xfe ) && ( priv.packet[1] == 0xfe ))
				return true;
			if (( priv.packet[2] == 0xfd ) && ( priv.packet[1] == 0xfd ))
				return true;
			priv.lex_mode = PenMount_byte3;
			break;

		case PenMount_byte3:
			priv.packet[3] = (unsigned char) c;
			priv.lex_mode = PenMount_byte4;
			break;

		case PenMount_byte4:
			priv.packet[4] = (unsigned char) c;
			priv.lex_mode = PenMount_byte0;
			return true;

		case PenMount_Response0:
printf("r = %02x\n", c);
			if ( c == 0xff )
				priv.lex_mode = PenMount_Response1;
			priv.packet[0] = (unsigned char) c;
			break;

		case PenMount_Response1:
			priv.packet[1] = (unsigned char) c;
			priv.lex_mode = PenMount_Response2;
			break;
		case PenMount_Response2:
			priv.packet[2] = (unsigned char) c;
			priv.lex_mode = PenMount_byte0;
			return true;
		}
	}
	return false;
}

void PenMountDevice::ReadInput(void)
{
	int x,y;
	unsigned char opck[ PENMOUNT_PACKET_SIZE ];

	while (1)
	{
		if ( GetPacket() == false )
				break;

		if (priv.packet[0] == 0xff)
		{
printf("down\n");
			priv.packet[0] = 1;
		}

		if (priv.packet[0] == 0xbf)
		{
printf("up\n");
			priv.packet[0] = 0;
		}

		x = priv.packet[1]*256+priv.packet[2];
		y = priv.packet[3]*256+priv.packet[4];

		was_something_interesting = true;
		priv.x = x;
		priv.y = y;
printf("Move %d %d\n", x,y);

		/*
		 * Emit a button press or release.
		 */
		if ((priv.button_down == false) && (priv.packet[0] == 1))
		{
printf("Push\n");
			priv.button_down = true;
		}
		else if ((priv.button_down == true) && (priv.packet[0] == 0))
		{
printf("Release\n");
			priv.button_down = false;
		}
	}
}

int PenMountDevice::GetInputEvent(int& type, int& key, int& x, int& y, int& button)
{
	if (working)
	{
		ReadInput();
	}
	if (was_something_interesting)
	{
		type = MOUSEEVENT;
		key = 0;
		button = priv.button_down ? FG_BUTTON_LEFT : FG_BUTTON_NONE;
		x = priv.x;
		y = priv.y;
		was_something_interesting = false;
		return 1;
	}
	return 0;
}


