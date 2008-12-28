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
#include "bot_fortress.h"
#include "bot_buttons.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_schedule.h"
#include "bot_task.h"
#include "bot_waypoint.h"
#include "bot_navigator.h"

#include "vstdlib/random.h" // for random functions


void CBotFortress :: init (bool bVarInit)
{
	CBot::init(bVarInit);
}

void CBotFortress :: setup ()
{
	CBot::setup();
}

bool CBotFortress :: startGame()
{
	if ( m_pPlayerInfo->GetTeamIndex() == 0 )
	{
		selectTeam();
	}
	else if ( (m_pPlayerInfo->GetWeaponName() == NULL) || (*(m_pPlayerInfo->GetWeaponName()) == 0) )
	{
		selectClass();
	}
	else
		return true;

	return false;
}

void CBotFortress :: killed ( edict_t *pVictim )
{
 return;
}

void CBotFortress :: died ( edict_t *pKiller )
{
	CBot::spawnInit();

	droppedFlag();

	if ( RandomInt(0,1) )
		m_pButtons->attack();
}

void CBotFortress :: spawnInit ()
{
	CBot::spawnInit();
}

bool CBotFortress :: isEnemy ( edict_t *pEdict )
{
	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	return true;	
}

void CBotFortress :: currentlyDead ()
{
	m_fUpdateClass = engine->Time() + 0.1f;
}


void CBotFortress :: modThink ()
{
	updateClass();

	if ( m_fCallMedic < engine->Time() )
	{
		if ( ((float)m_pPlayerInfo->GetHealth() / m_pPlayerInfo->GetMaxHealth()) < 0.5 )
		{
			m_fCallMedic = engine->Time() + RandomFloat(10.0f,30.0f);

			callMedic();
		}
	}
}


void CBotFortress :: selectTeam ()
{
	char buffer[32];

	int team = RandomInt(1,2);

	sprintf(buffer,"jointeam %d",team);

	helpers->ClientCommand(m_pEdict,buffer);
}

void CBotFortress :: selectClass ()
{
	char buffer[32];

	TF_Class _class = (TF_Class)RandomInt(1,9);

	m_iClass = _class;

	if ( _class == TF_CLASS_SCOUT )
	{
		sprintf(buffer,"joinclass scout");
		m_fIdealMoveSpeed = 300;
	}
	else if ( _class == TF_CLASS_ENGINEER )
	{
		sprintf(buffer,"joinclass engineer");
		m_fIdealMoveSpeed = 270;
	}
	else if ( _class == TF_CLASS_DEMOMAN )
	{
		sprintf(buffer,"joinclass demoman");
		m_fIdealMoveSpeed = 250;
	}
	else if ( _class == TF_CLASS_SOLDIER )
	{
		sprintf(buffer,"joinclass soldier");
		m_fIdealMoveSpeed = 220;
	}
	else if ( _class == TF_CLASS_HWGUY )
	{
		sprintf(buffer,"joinclass heavyweapons");
		m_fIdealMoveSpeed = 200;
	}
	else if ( _class == TF_CLASS_MEDIC )
	{
		sprintf(buffer,"joinclass medic");
		m_fIdealMoveSpeed = 280;
	}
	else if ( _class == TF_CLASS_SPY )
	{
		sprintf(buffer,"joinclass spy");
		m_fIdealMoveSpeed = 280;
	}
	else if ( _class == TF_CLASS_PYRO )
	{
		sprintf(buffer,"joinclass pyro");
		m_fIdealMoveSpeed = 270;
	}
	else
	{
		sprintf(buffer,"joinclass sniper");
		m_fIdealMoveSpeed = 270;
	}
	helpers->ClientCommand(m_pEdict,buffer);
}

/////////////////////////////////////////////////////////////////////////////
// TEAM FORTRESS 2


void CBotTF2 :: taunt ()
{
	helpers->ClientCommand(m_pEdict,"taunt");
}

void CBotTF2 :: engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd )
{
	char buffer[16];
	char cmd[256];

	if ( iEngiCmd == ENGI_BUILD )
		strcpy(buffer,"build");
	else
		strcpy(buffer,"destroy");

	sprintf(cmd,"%s %d",buffer,iBuilding);

	helpers->ClientCommand(m_pEdict,cmd);
}

void CBotTF2 :: spyDisguise ( int iTeam, int iClass )
{
	char cmd[256];

	sprintf(cmd,"disguise %d %d",iClass,iTeam);

	helpers->ClientCommand(m_pEdict,cmd);
}

void CBotTF2 :: updateClass ()
{
	if ( m_fUpdateClass < engine->Time() )
	{
		const char *model = m_pPlayerInfo->GetModelName();

		if ( strcmp(model,"soldier") )
			m_iClass = TF_CLASS_SOLDIER;
		else if ( strcmp(model,"sniper") )
			m_iClass = TF_CLASS_SNIPER;
		else if ( strcmp(model,"heavyweapons") )
			m_iClass = TF_CLASS_HWGUY;
		else if ( strcmp(model,"medic") )
			m_iClass = TF_CLASS_MEDIC;
		else if ( strcmp(model,"pyro") )
			m_iClass = TF_CLASS_PYRO;
		else if ( strcmp(model,"spy") )
			m_iClass = TF_CLASS_SPY;
		else if ( strcmp(model,"scout") )
			m_iClass = TF_CLASS_SCOUT;
		else if ( strcmp(model,"engineer") )
			m_iClass = TF_CLASS_ENGINEER;
		else if ( strcmp(model,"demoman") )
			m_iClass = TF_CLASS_DEMOMAN;
		else
			m_iClass = TF_CLASS_CIVILIAN;
	}
}

TF_Class CBotTF2 :: getClass ()
{
	return m_iClass;
}

bool CBotTF2 :: hasEngineerBuilt ( eEngiBuild iBuilding )
{
	switch ( iBuilding )
	{
	case ENGI_SENTRY:
		return false; // TODO
		break;
	case ENGI_DISP:
		return false; // TODO
		break;
	case ENGI_ENTRANCE:
		return false; // TODO
		break;
	case ENGI_EXIT:
		return false; // TODO
		break;
	}	

	return false;
}

void CBotFortress :: callMedic ()
{
	helpers->ClientCommand (m_pEdict,"saveme");
}

void CBotTF2 :: callMedic ()
{
	helpers->ClientCommand (m_pEdict,"voicemenu 0 0");
}

void CBotTF2 :: modThink ()
{
// mod specific think code here
CBotFortress :: modThink();
}

void CBotTF2 :: getTasks ( unsigned int iIgnore )
{
	// look for tasks
	if ( m_pEnemy && hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{		
		if ( !m_pSchedules->isCurrentSchedule(SCHED_ATTACK) && !m_pSchedules->isCurrentSchedule(SCHED_GOOD_HIDE_SPOT) )
		{
			m_pSchedules->removeSchedule(SCHED_ATTACK);
			m_pSchedules->removeSchedule(SCHED_GOOD_HIDE_SPOT);

			m_pSchedules->addFront(new CBotAttackSched(m_pEnemy));
			m_pSchedules->addFront(new CGotoHideSpotSched(m_pEnemy));

			return;
		}

		m_bLookedForEnemyLast = false;
	}
	else if ( !m_bLookedForEnemyLast && m_pLastEnemy && CBotGlobals::entityIsAlive(m_pLastEnemy) )
	{
		if ( wantToFollowEnemy() )
		{
			CBotSchedule *pSchedule = new CBotSchedule();
			CFindPathTask *pFindPath = new CFindPathTask(m_vLastSeeEnemy);		
			pSchedule->addTask(pFindPath);
			pSchedule->addTask(new CFindLastEnemy());

			//////////////
			pFindPath->setNoInterruptions();

			m_pSchedules->add(pSchedule);

			m_bLookedForEnemyLast = true;
		}
	}

	if ( !m_pSchedules->isEmpty() )
		return; // already got some tasks left

	if ( hasFlag () )
	{
		// Goto capture point

		// roam
		CWaypoint *pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomFlaggedWaypoint());

		if ( pWaypoint )
		{
			m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
		}
	}
	else
	{
		// Find enemy flag or defend flag or roam

		// roam
		CWaypoint *pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomFlaggedWaypoint());

		if ( pWaypoint )
		{
			m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
		}
	}

}

bool CBotTF2 :: isEnemy ( edict_t *pEdict )
{
	if ( pEdict == m_pEdict )
		return false;

	if ( !CBotGlobals::entityIsValid(pEdict) )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	return true;	
}

/////////////////////////////////////////////////////////////////////////
// FORTRESS FOREVER


void CBotFF :: modThink ()
{
// mod specific think code here
	CBotFortress :: modThink();
}

bool CBotFF :: isEnemy ( edict_t *pEdict )
{
	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	return true;	
}


// --------------------------------------------------------------------------
// Hackathon tiem

// Auto reconstructed from vtable block @ 0x00D3D380
// from "server_i486.so", by ida_vtables.idc
/*
#define TF_GetServerClass 9

class VfuncEmptyClass {}; 

void VFuncs_GetServerClass(CBaseEntity *pThisPtr)
{
	void **this_ptr = *(void ***)&pThisPtr;
	void **vtable = *(void ***)pThisPtr;
	void *func = vtable[TF_GetServerClass]; 
 
	union {CBaseEntity *(VfuncEmptyClass::*mfpnew)( void );
	#ifndef __linux__
			void *addr;	} u; 	u.addr = func;
	#else // GCC's member function pointers all contain a this pointer adjustor. You'd probably set it to 0 
				struct {void *addr; intptr_t adjustor;} s; } u; u.s.addr = func; u.s.adjustor = 0;
	#endif
 
	return(int)(reinterpret_cast<VfuncEmptyClass*>(this_ptr)->*u.mfpnew)();
}
*/