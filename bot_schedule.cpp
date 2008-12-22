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
#include "bot_schedule.h"
#include "bot_task.h"

/////////////////////////////////////////////

CBotGotoOriginSched :: CBotGotoOriginSched ( Vector vOrigin )
{
	addTask(new CFindPathTask(vOrigin)); // first
	addTask(new CMoveToTask(vOrigin)); // second
}

CBotGotoOriginSched :: CBotGotoOriginSched ( edict_t *pEdict )
{
	addTask(new CFindPathTask(pEdict));	
	addTask(new CMoveToTask(pEdict));
}

void CBotGotoOriginSched :: init ()
{
	setID(SCHED_GOTO_ORIGIN);
}

///////////
CGotoHideSpotSched :: CGotoHideSpotSched ( edict_t *pEdict )
{
	// run at flank while shooting	
	CFindPathTask *pHideGoalPoint = new CFindPathTask();
	
	// no interrupts, should be a quick waypoint path anyway
	pHideGoalPoint->setNoInterruptions();
	// get vector from good hide spot task
	pHideGoalPoint->getPassedVector();
	
	addTask(new CFindGoodHideSpot(pEdict));
	addTask(pHideGoalPoint);
}

CGotoHideSpotSched :: CGotoHideSpotSched ( Vector vOrigin )
{
	// run at flank while shooting	
	CFindPathTask *pHideGoalPoint = new CFindPathTask();
	
	// no interrupts, should be a quick waypoint path anyway
	pHideGoalPoint->setNoInterruptions();
	// get vector from good hide spot task
	pHideGoalPoint->getPassedVector();
	
	addTask(new CFindGoodHideSpot(vOrigin));
	addTask(pHideGoalPoint);
}

void CGotoHideSpotSched :: init ()
{
	setID(SCHED_GOOD_HIDE_SPOT);
}
/////////////
CBotAttackSched :: CBotAttackSched ( edict_t *pEdict )
{
	addTask(new CAttackEntityTask(pEdict));
	//addTask(new CFindGoodHideSpot(pEdict));
}

void CBotAttackSched :: init ()
{
	setID(SCHED_ATTACK);
}

/////////////////////////////////////////////
void CBotSchedule :: execute ( CBot *pBot )
{
	// current task
	CBotTask *pTask = m_Tasks.GetFrontInfo();
	static eTaskState iState;

	iState = pTask->isInterrupted(pBot);

	if ( iState == STATE_FAIL )
		pTask->fail();
	else if ( iState == STATE_COMPLETE )
		pTask->complete();
	else // still running
	{
		// timed out ??
		if ( pTask->timedOut() )
			pTask->fail(); // fail
		else
		{			
			pTask->execute(pBot,this); // run
		}
	}

	if ( pTask->hasFailed() )
	{
		m_bFailed = true;
	}
	else if ( pTask->isComplete() )
	{
		removeTop();
	}
}

void CBotSchedule :: addTask ( CBotTask *pTask )
{
	// initialize
	pTask->init();
    // add
	m_Tasks.Add(pTask);
}

void CBotSchedule :: removeTop ()
{
	CBotTask *pTask = m_Tasks.GetFrontInfo();

	m_Tasks.RemoveFront();

	delete pTask;
}

/////////////////////

CBotSchedule :: CBotSchedule ()
{
	_init();
}

void CBotSchedule :: _init ()
{
	m_bFailed = false;
	m_bitsPass = 0;		

	// pass information
	iPass = 0;
	fPass = 0;
	vPass = Vector(0,0,0);
	pPass = 0;	

	init();
}

void CBotSchedule :: passInt(int i)
{
	iPass = i;
	m_bitsPass |= BITS_SCHED_PASS_INT;
}
void CBotSchedule :: passFloat(float f)
{
	fPass = f;
	m_bitsPass |= BITS_SCHED_PASS_FLOAT;
}
void CBotSchedule :: passVector(Vector v)
{
	vPass = v;
	m_bitsPass |= BITS_SCHED_PASS_VECTOR;
}
void CBotSchedule :: passEdict(edict_t *p)
{
	pPass = p;
	m_bitsPass |= BITS_SCHED_PASS_EDICT;
}
////////////////////