/*
 *    This file is part of RCBot.
 *
 *    RCBot by Paul Murphy adapted from Botman's HPB Bot 2 template.
 *
 *    RCBot is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    RCBot is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RCBot; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */
#include "bot.h"
#include "bot_const.h"
#include "bot_profiling.h"
#include "bot_strings.h"
#include "bot_client.h"

// List of all timers
CProfileTimer CProfileTimers :: m_Timers[PROFILING_TIMERS] = 
{
CProfileTimer("CBots::botThink()"), // BOTS_THINK_TIMER
CProfileTimer("CBot::think()"), // BOT_THINK_TIMER
CProfileTimer("Nav::findRoute()") // BOT_ROUTE_TIMER
};

// initialise update time
float CProfileTimers::m_fNextUpdate = 0;

inline unsigned __int64 RDTSC(void)
{
    _asm    _emit 0x0F
    _asm    _emit 0x31
}
// if windows USE THE QUERYPERFORMANCECOUNTER
#ifdef _WIN32
 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>
#endif

CProfileTimer :: CProfileTimer (const char *szFunction)
{
	m_szFunction = CStrings::getString(szFunction);
	m_min = 9999999999;
	m_max = 0;
	m_average = 2;
	m_bInvoked = false;
}

// "Begin" Timer i.e. update time
void CProfileTimer :: Start()
{
#ifdef _WIN32
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start_cycle));
#else
    start_cycle = RDTSC();
#endif

}
// Stop Timer, work out min/max values and set invoked
void CProfileTimer :: Stop()
{
	unsigned __int64 end_cycle;

#ifdef _WIN32
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&end_cycle));
#else
	end_cycle = RDTSC();
#endif

	m_last = end_cycle - start_cycle;

	if ( m_last > m_max )
		m_max = m_last;
	if ( !m_bInvoked || (m_last < m_min) )
		m_min = m_last;

	m_bInvoked = true;

}
// print the values, first work out average (use max/min/previous values), 
// and work out percentage of power
void CProfileTimer :: print (int i, float *high)
{
	if ( m_szFunction )
	{
		char str[256];
		float percent = 1;

		m_average = (m_last/3)+(m_max/3)+(m_min/3);

		if ( i == 0 )
		{
			percent = 100;

			if ( !m_bInvoked )
				*high = 1;
			else
				*high = m_average;
		}
		else
		{
			percent = (((float)m_average)/(*high))*100.0f;
		}
		

		if ( m_bInvoked )
		{
			sprintf(str,"%17s|%10lld|%10lld|%10lld|%10lld|%6.1f",m_szFunction,m_last,m_min,m_max,m_average,percent);			

			CClients::clientDebugMsg(BOT_DEBUG_PROFILE,str);

			// uninvoke now
			m_bInvoked = false;
		}
		// reset max
		m_max = 0;
	}
}

// get the required timer
CProfileTimer *CProfileTimers::getTimer (int id)
{
	if ( id >= 0 && id < PROFILING_TIMERS )
		return &m_Timers[id];
	return NULL;
}
// do this every map start
void CProfileTimers :: reset ()
{
	m_fNextUpdate = 0;
}
// update and show every x seconds
void CProfileTimers::updateAndDisplay()
{
	if ( CClients::clientsDebugging(BOT_DEBUG_PROFILE) )
	{
		if ( m_fNextUpdate < engine->Time() )
		{
			int i = 0;

			float high = 1;

			// next update in 1 second
			m_fNextUpdate = engine->Time() + 1.0f;

			CClients::clientDebugMsg(BOT_DEBUG_PROFILE,"|----------------PROFILING---UPDATE---------------------------------|");
			CClients::clientDebugMsg(BOT_DEBUG_PROFILE,"|------name------|---prev---|---min----|---max----|----avg---|-prct-|");

			for ( i = 0; i < PROFILING_TIMERS; i ++ )
			{		
				m_Timers[i].print(i,&high);
			}
		}
	}
}