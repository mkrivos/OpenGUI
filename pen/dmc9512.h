#ifndef	_PENMOUNT_H_
#define _PENMOUNT_H_

/******************************************************************************
 *		Definitions
 *				structs, typedefs, #defines, enums
 *****************************************************************************/

#define PENMOUNT_PACKET_SIZE		5

typedef enum
{
	PenMount_byte0, PenMount_byte1, PenMount_byte2, PenMount_byte3, PenMount_byte4,
	PenMount_Response0, PenMount_Response1, PenMount_Response2
}
PenMountState;


struct PenMountPrivate
{
	int x,y;
	bool button_down;			/* is the "button" currently down */

	unsigned char packet[PENMOUNT_PACKET_SIZE];	/* packet being/just read */
	PenMountState lex_mode;

	PenMountPrivate();
};

class PenMountDevice : public FGInputDevice
{
		class LocalPrint : public PrintInterface
		{
		public:
			virtual void PrintLog(char* format, ...) {}               ///< Vypise spravu do prislusneho logu
			virtual void PrintErr(int errnum, char* format, ...) {}  ///< Vypise chybovu spravu do prislusneho logu aj so std popisom chyby
		} print_face;

		SerialLine *com;
		PenMountPrivate priv;
		bool was_something_interesting;

		bool GetPacket(void);
		bool SendPacket(unsigned char *buf, int len);
		int ReadByte(void);
		virtual void ReadInput(void);

	public:
		PenMountDevice(int port)
		{
			com = new SerialLine(port, false, IOBaseParams("TOUCHSCREEN", false));
//			fixme: povolit pre presmerovanie vypisov do prazdna
			com->SetPrint(&print_face);
			was_something_interesting = false;
		}
		virtual ~PenMountDevice()
		{
			com->Close();
			delete com;
			com = 0;
		}
		virtual bool Attach(void);
		virtual int GetInputEvent(int& type, int& key, int& x, int& y, int& button);
};

#define CHIP_UNKNOWN	0
#define DMC9512			1


#endif /* _PENMOUNT_H_ */

