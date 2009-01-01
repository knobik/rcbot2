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
#ifndef __RCBOT_SCHEDULE_H__
#define __RCBOT_SCHEDULE_H__

#include "bot.h"
#include "bot_task.h"
#include "bot_genclass.h"
#include "bot_fortress.h"

class CBotTask;
class CAttackEntityTask;

#define BITS_SCHED_PASS_INT		(1<<0)
#define BITS_SCHED_PASS_FLOAT	(1<<1)
#define BITS_SCHED_PASS_VECTOR	(1<<2)
#define BITS_SCHED_PASS_EDICT	(1<<3)

typedef enum
{
	SCHED_NONE = 0,
	SCHED_ATTACK,
	SCHED_RUN_FOR_COVER,
	SCHED_GOTO_ORIGIN,
	SCHED_GOOD_HIDE_SPOT,
	SCHED_TF2_GET_FLAG,
	SCHED_TF2_GET_HEALTH,
	SCHED_TF_BUILD,
	SCHED_HEAL,
	SCHED_GET_METAL,
	SCHED_SNIPE,
	SCHED_UPGRADE
}eBotSchedule;

class CBotSchedule
{
public:
	CBotSchedule(CBotTask *pTask)
	{
		_init();

		addTask(pTask);
	}

	CBotSchedule();

	void _init ();
	virtual void init () { return; } // nothing, used by sub classes

	void addTask( CBotTask *pTask );

	void execute ( CBot *pBot );
/*
	CBotTask *currentTask ()
	{
		if ( m_Tasks.IsEmpty() )
			return NULL;
		return m_Tasks.Front();
	}
*/
	bool hasFailed ()
	{
		return m_bFailed;
	}

	bool isComplete ()
	{
		return m_Tasks.IsEmpty();
	}

	void freeMemory ()
	{
		m_Tasks.Destroy();
	}	

	void removeTop ();

	//////////////////////////

	void clearPass () { m_bitsPass = 0; }

	void passInt(int i);
	void passFloat(float f);
	void passVector(Vector v);
	void passEdict(edict_t *p);
	//////////////////////////

	bool hasPassInfo () { return (m_bitsPass!=0); }

	inline int passedInt () { return iPass; }
	inline float passedFloat() { return fPass; }
	inline Vector passedVector() { return vPass; }
	inline edict_t *passedEdict() { return pPass; }
	inline bool isID ( eBotSchedule iId ) { return m_iSchedId == iId; }

	inline bool hasPassInt () { return ((m_bitsPass&BITS_SCHED_PASS_INT)>0); }
	inline bool hasPassFloat () { return ((m_bitsPass&BITS_SCHED_PASS_FLOAT)>0); }
	inline bool hasPassVector () { return ((m_bitsPass&BITS_SCHED_PASS_VECTOR)>0); }
	inline bool hasPassEdict () { return ((m_bitsPass&BITS_SCHED_PASS_EDICT)>0); }

protected:
	inline void setID ( eBotSchedule iId ) { m_iSchedId = iId; }


private:
    dataQueue <CBotTask*> m_Tasks;
	bool m_bFailed;
	eBotSchedule m_iSchedId;
	
	// passed information to next task(s)
	int iPass;
	float fPass;
	Vector vPass;
	edict_t *pPass;

	int m_bitsPass;
};

class CBotSchedules
{
public:
	bool hasSchedule ( eBotSchedule iSchedule )
	{
		dataQueue<CBotSchedule*> tempQueue = m_Schedules;

		while ( !tempQueue.IsEmpty() )
		{	
			CBotSchedule *sched = tempQueue.ChooseFrom();

			if ( sched->isID(iSchedule) )
			{
				tempQueue.Init();
				return true;
			}
		}

		return false;
	}

	bool isCurrentSchedule ( eBotSchedule iSchedule )
	{
		if ( m_Schedules.IsEmpty() )
			return false;

		return m_Schedules.GetFrontInfo()->isID(iSchedule);
	}

	void removeSchedule ( eBotSchedule iSchedule )
	{
		dataQueue<CBotSchedule*> tempQueue = m_Schedules;

		CBotSchedule *toRemove = NULL;

		while ( !tempQueue.IsEmpty() )
		{	
			CBotSchedule *sched = tempQueue.ChooseFrom();

			if ( sched->isID(iSchedule) )
			{
				toRemove = sched;
				tempQueue.Init();
				break;
			}
		}

		if ( toRemove )
			m_Schedules.Remove(toRemove);

		return;
	}

	void execute ( CBot *pBot )
	{
		if ( isEmpty() )
			return;

		CBotSchedule *pSched = m_Schedules.GetFrontInfo();
		
		pSched->execute(pBot);

		if ( pSched->isComplete() || pSched->hasFailed() )
			removeTop();
	}

	void removeTop ()
	{
		CBotSchedule *pSched = m_Schedules.GetFrontInfo();
		
		m_Schedules.RemoveFront();

		pSched->freeMemory();

		delete pSched;
	}

	void freeMemory ()
	{
		m_Schedules.Destroy();
	}

	void add ( CBotSchedule *pSchedule )
	{
		// initialize
		pSchedule->init();
		// add
		m_Schedules.Add(pSchedule);
	}

	void addFront ( CBotSchedule *pSchedule )
	{
		pSchedule->init();
		m_Schedules.AddFront(pSchedule);
	}

	bool isEmpty ()
	{
		return m_Schedules.IsEmpty();
	}
/*
	CBotTask *getCurrentTask ()
	{
		CBotSchedule *sched;

		if ( (sched = m_Schedules.Top()) != NULL )
		{
			return sched->currentTask();
		}
	}*/

private:
	dataQueue <CBotSchedule*> m_Schedules;
};

//////////////////////////////////

class CBotTFEngiUpgrade : public CBotSchedule
{
public:
	CBotTFEngiUpgrade ( edict_t *pBuilding );

	void init ();
};

class CBotTFEngiBuild : public CBotSchedule
{
public:
	CBotTFEngiBuild ( eEngiBuild iObject, Vector vOrigin );

	void init ();
};

class CBotTF2HealSched : public CBotSchedule
{
public:
	CBotTF2HealSched();
	void init ();
};

class CBotGetMetalSched : public CBotSchedule
{
public:
	CBotGetMetalSched ( Vector vOrigin );

	void init ();
};


class CBotTF2SnipeSched : public CBotSchedule
{
public:
	CBotTF2SnipeSched ( Vector vOrigin );

	void init ();
};


class CBotTF2GetHealthSched : public CBotSchedule
{
public:
	CBotTF2GetHealthSched ( Vector vOrigin );

	void init ();
};

class CBotTF2GetFlagSched : public CBotSchedule
{
public:
	CBotTF2GetFlagSched ( Vector vOrigin );

	void init ();
};

class CBotTF2FindFlagSched : public CBotSchedule
{
public:
	CBotTF2FindFlagSched ( Vector vOrigin );

	void init ();
};


class CBotGotoOriginSched : public CBotSchedule
{
public:
	CBotGotoOriginSched ( Vector vOrigin );

	CBotGotoOriginSched ( edict_t *pEdict );

	void init ();
};

class CBotAttackSched : public CBotSchedule
{
public:
	CBotAttackSched ( edict_t *pEdict );

	void init ();
};

class CRunForCover : public CBotSchedule
{
public:
	CRunForCover ( Vector vOrigin );

	void init ()
	{
		setID(SCHED_RUN_FOR_COVER);
	}
};

class CGotoHideSpotSched : public CBotSchedule
{
public:
	CGotoHideSpotSched ( edict_t *pEdict );
	CGotoHideSpotSched ( Vector vOrigin );

	void init ();
};
#endif