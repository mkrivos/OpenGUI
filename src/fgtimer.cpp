/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999  Sam Lantinga
    
    Improvements to OpenGUI - Marian Krivos - 27.01.2000

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

    Sam Lantinga
        slouken@devolution.com

*/

#include "fastgl.h"

#ifndef _WIN32
#include <stdio.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#else // WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

#undef Yield


#ifdef FG_NAMESPACE
namespace fgl {
#endif

/* The first ticks value of the application */

/* Data to handle a single periodic alarm */
static TimerCall alarm_callback = NULL;
static unsigned int alarm_interval;
#ifndef _WIN32
static struct timeval start;
#else
static unsigned start;
/* Data to handle a single periodic alarm */
static UINT timerID = 0;

static void CALLBACK HandleAlarmWin(UINT uID,  UINT uMsg, DWORD dwUser,
                                                DWORD dw1, DWORD dw2)
{
//      unsigned int ms;
        if ( alarm_callback )
        {
           ( *alarm_callback )( alarm_interval );
        }
}
#endif

#ifdef __rtems__
static rtems_unsigned32 ticks_per_second = 0;
static rtems_id    id = 0;

static inline rtems_unsigned32 get_ticks_per_second( void )
{
  (void) rtems_clock_get( RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticks_per_second );
  return ticks_per_second;
}


extern "C" void rtems_handler( rtems_id id, void *user_data )
{
        unsigned int ms;
        if ( alarm_callback )
        {
                ms = ( *alarm_callback )( alarm_interval );
		   FGTimer::SetTimer( ms, alarm_callback );
        }
}
#endif


void FGTimer::StartTicks(void)
{
        /* Set first ticks value */
#ifndef _WIN32
        gettimeofday(&start, NULL);
#else
        start = timeGetTime();
#endif
}

/** Get the number of milliseconds since the OpenGUI library initialization.
 * Note that this value wraps if the program runs for more than ~49 days.
 */
unsigned int FGTimer::GetTicks(void)
{
		unsigned int ticks;
#ifndef _WIN32
		struct timeval now;
		gettimeofday(&now, NULL);
		ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
#else
		ticks = timeGetTime();
#endif
		return ticks;
}

/** Wait a specified number of milliseconds before returning */
void FGTimer::Delay(unsigned int ms)
{
#ifndef _WIN32
#ifdef __rtems__
		rtems_interval tcks = ( ms*ticks_per_second )/1000;
	if( !tcks ) tcks++;
    rtems_task_wake_after( tcks );
#else

        int was_error;
#ifndef linux   /* Non-Linux implementations need to calculate time left */
        unsigned int then, now, elapsed;
#endif
        struct timeval tv;

        /* Set the timeout interval - Linux only needs to do this once */
#ifdef linux
        tv.tv_sec = ms/1000;
        tv.tv_usec = (ms%1000)*1000;
#else
        then = GetTicks();
#endif
        do {
                errno = 0;
#ifndef linux
                /* Calculate the time interval left (in case of interrupt) */
                now = GetTicks();
                elapsed = (now-then);
                then = now;
				if ( elapsed >= ms ) {
						break;
                }
                ms -= elapsed;
                tv.tv_sec = ms/1000;
                tv.tv_usec = (ms%1000)*1000;
#endif
                was_error = select(0, NULL, NULL, NULL, &tv);
        } while ( was_error && (errno == EINTR) );

#endif
#else // win32
        Sleep(ms);
#endif // win32
}

#ifndef __rtems__
void HandleAlarm(int sig)
{
        unsigned int ms;

        if ( alarm_callback ) {
                ms = (*alarm_callback)(alarm_interval);
                if ( ms != alarm_interval ) {
                        FGTimer::SetTimer(ms, alarm_callback);
                }
        }
}
#endif

/** Set a callback to run after the specified number of milliseconds has
 * elapsed. The callback function is passed the current timer interval
 * and returns the next timer interval.  If the returned value is the
 * same as the one passed in, the periodic alarm continues, otherwise a
 * new alarm is scheduled.
 *
 * To cancel a currently running timer, call SetTimer(0, NULL);
 *
 * The timer callback function may run in a different thread than your
 * main code, and so shouldn't call any functions from within itself.
 *
 * The maximum resolution of this timer is 10 ms, which means that if
 * you request a 16 ms timer, your callback will run approximately 20 ms
 * later on an unloaded system.  If you wanted to set a flag signaling
 * a frame update at 30 frames per second (every 33 ms), you might set a
 * timer for 30 ms:
 *   SetTimer((33/10)*10, flag_update);
 *
 * Under UNIX, you should not use raise or use SIGALRM and this function
 * in the same program, as it is implemented using setitimer().  You also
 * should not use this function in multi-threaded applications as signals
 * to multi-threaded apps have undefined behavior in some implementations.
 */
int FGTimer::SetTimer(unsigned int ms, TimerCall callback)
{
#ifndef _WIN32
#ifndef __rtems__
		struct itimerval timer;
#endif
		alarm_callback = NULL;
		alarm_interval = ms;

#ifdef __rtems__
   if( !id )
   {
          return 0;
   }
#endif

        if ( ms ) {
                /* Set a new alarm */
                alarm_callback = callback;
#ifdef __rtems__
          rtems_interval tcks = ( ms*ticks_per_second )/1000;
          if( !tcks ) tcks++;
          rtems_timer_fire_after( id, tcks, rtems_handler, NULL );
#endif
        }
#ifdef __rtems__
   else
   {
          rtems_timer_cancel( id );
   }
#else
		timer.it_value.tv_sec = (ms/1000);
        timer.it_value.tv_usec = (ms%1000)*1000;
        timer.it_interval.tv_sec = (ms/1000);
        timer.it_interval.tv_usec = (ms%1000)*1000;
        setitimer(ITIMER_REAL, &timer, NULL);
#endif
        return(0);
#else  // win32
		if ( timerID )
				timeKillEvent(timerID);
        if ( ms )
        {       /* Set a new alarm */
                alarm_callback = callback;
                /* Allow 10 ms of drift so we don't chew on CPU */
                timerID = timeSetEvent(ms,1,HandleAlarmWin,0,TIME_PERIODIC);
                if ( ! timerID )
                {
                        printf("timeSetEvent() failed!\n");
                        return(-1);
                }
        }

#endif // win32
        return 0;
}

int FGTimer::TimerInit(void)
{
        StartTicks();
#ifndef _WIN32
#ifndef __rtems__
        struct sigaction action;

        /* Set the alarm handler (Linux specific) */
        memset(&action, 0, sizeof(action));
        action.sa_handler = &HandleAlarm;
#ifdef __linux__
        action.sa_flags = SA_RESTART;
#else
		action.sa_flags = 0;
#endif
        sigemptyset(&action.sa_mask);
        sigaction(SIGALRM, &action, NULL);
#endif

#ifdef __rtems__
   static rtems_name name;
   ticks_per_second = get_ticks_per_second();
   name = rtems_build_name( 'F', 'G', 'T', 'M' );
   rtems_timer_create( name, &id );
#endif // __rtems__
#else // win32
        MMRESULT result;

        /* Set timer resolution */
        result = timeBeginPeriod(TIMER_RESOLUTION);
        if ( result != TIMERR_NOERROR ) {
                printf("Warning: Can't set %d ms timer resolution!\n",
                                                        TIMER_RESOLUTION);
        }
#endif  // win32
        return(0);
}

void FGTimer::TimerQuit(void)
{
#ifdef __rtems__
   if( id )
   {
#endif

				SetTimer( 0, NULL );
#ifdef __rtems__
          rtems_timer_delete( id );
   }
   id = 0;
#endif

#ifdef _WIN32
        if ( timerID ) {
                timeKillEvent(timerID);
        }
        timeEndPeriod(TIMER_RESOLUTION);
#endif // win32
}

#ifdef FG_NAMESPACE
}
#endif

