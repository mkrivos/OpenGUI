//---------------------------------------------------------------------------

#ifndef pen_gunzeH
#define pen_gunzeH

class GunzeDevice : public FGInputDevice
{
		class LocalPrint : public PrintInterface
		{
		public:
			virtual void PrintLog(char* format, ...) {}               ///< Vypise spravu do prislusneho logu
			virtual void PrintErr(int errnum, char* format, ...) {}  ///< Vypise chybovu spravu do prislusneho logu aj so std popisom chyby
		} print_face;

		SerialLine *com;
		char packet[16];
		bool button_down;
		int count;
		int x;
		int y;
		bool was_something_interesting;

		bool GetPacket(void);
		bool SendPacket(unsigned char *buf, int len);
		int ReadByte(void);
		virtual void ReadInput(void);

	public:
		GunzeDevice(int port)
		{
			com = new SerialLine(port, false, IOBaseParams("TOUCHSCREEN", false));
//			fixme: povolit pre presmerovanie vypisov do prazdna
			com->SetPrint(&print_face);
			count = 0;
			button_down = false;
			packet[0] = 0;
			was_something_interesting = false;
		}
		virtual ~GunzeDevice()
		{
			com->Close();
			delete com;
			com = 0;
		}
		virtual bool Attach(void);
		virtual int GetInputEvent(int& type, int& key, int& x, int& y, int& button);
};

// $Id: pen_gunze.h 2086 2005-05-12 10:52:34Z majo $
//---------------------------------------------------------------------------
#endif
