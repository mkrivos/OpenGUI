#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
	The TIMER procedure callback type.
	@ingroup Types
*/
typedef unsigned int (*TimerCall)(unsigned int);

#define TIMER_RESOLUTION	10	/* Experimentally determined */

/**
	Some timing class.
*/
class FGTimer : public FGConnector
{
	protected:
		friend void  			HandleAlarm(int sig);
		int						state;
		static void				StartTicks(void);
	public:
		FGTimer(unsigned int val, TimerCall cb)
		{
			state = SetTimer(val, cb);
		}
		FGTimer(TimerCall cb)
		{
			state = SetTimer(0, cb);
		}
		virtual ~FGTimer()
		{
			TimerQuit();
		}
		static int			SetTimer(unsigned int val, TimerCall cb);
		static unsigned int GetTicks(void);
		static void			Delay(unsigned int);
		static int			TimerInit(void);
		static void			TimerQuit(void);
};

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


