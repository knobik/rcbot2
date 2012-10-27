/*
 *    part of https://rcbot2.svn.sourceforge.net/svnroot/rcbot2
 *
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
#include "bot_client.h"
#include "bot_weapons.h"
#include "bot_globals.h"
////////////////////////////////////

const char *szSchedules[] = 
{
	"SCHED_NONE",
	"SCHED_ATTACK",
	"SCHED_RUN_FOR_COVER",
	"SCHED_GOTO_ORIGIN",
	"SCHED_GOOD_HIDE_SPOT",
	"SCHED_TF2_GET_FLAG",
	"SCHED_TF2_GET_HEALTH",
	"SCHED_TF_BUILD",
	"SCHED_HEAL",
	"SCHED_GET_METAL",
	"SCHED_SNIPE",
	"SCHED_UPGRADE",
	"SCHED_USE_TELE",
	"SCHED_SPY_SAP_BUILDING",
	"SCHED_USE_DISPENSER",
	"SCHED_PICKUP",
	"SCHED_TF2_GET_AMMO",
	"SCHED_TF2_FIND_FLAG",
	"SCHED_LOOKAFTERSENTRY",
	"SCHED_DEFEND",
	"SCHED_ATTACKPOINT",
	"SCHED_DEFENDPOINT",
	"SCHED_TF2_PUSH_PAYLOADBOMB",
	"SCHED_TF2_DEFEND_PAYLOADBOMB",
	"SCHED_TF2_DEMO_PIPETRAP",
	"SCHED_BACKSTAB",
	"SCHED_REMOVESAPPER",
	"SCHED_GOTONEST",
	"SCHED_MESSAROUND",
	"SCHED_TF2_ENGI_MOVE_BUILDING"
};


CBotTF2DemoPipeTrapSched :: CBotTF2DemoPipeTrapSched ( eDemoTrapType type, Vector vStand, Vector vLoc, Vector vSpread )
{
	addTask(new CFindPathTask(vStand));
	addTask(new CBotTF2DemomanPipeTrap(type,vStand,vLoc,vSpread));
}

void CBotTF2DemoPipeTrapSched :: init()
{
	setID(SCHED_TF2_DEMO_PIPETRAP);
}


//////////////////////////////////////
CBotTF2HealSched::CBotTF2HealSched(edict_t *pHeal)
{
	addTask(new CFindPathTask(CBotGlobals::entityOrigin(pHeal)));
	addTask(new CBotTF2MedicHeal());
}

void CBotTF2HealSched::init()
{
	setID(SCHED_HEAL);
}

/////////////////////////////////////////////

CBotTFEngiBuild :: CBotTFEngiBuild ( eEngiBuild iObject, Vector vOrigin )
{
	addTask(new CFindPathTask(vOrigin)); // first
	addTask(new CBotTFEngiBuildTask(iObject,vOrigin)); // second
}

void CBotTFEngiBuild :: init ()
{
	setID(SCHED_TF_BUILD);
}
//////////////////////////////////////////////

CBotGetMetalSched :: CBotGetMetalSched ( Vector vOrigin )
{
	addTask(new CFindPathTask(vOrigin)); // first
	addTask(new CBotTF2WaitAmmoTask(vOrigin));
}

void CBotGetMetalSched :: init ()
{
	setID(SCHED_GET_METAL);
}
//////////////////////////////////////////////
CBotEngiMoveBuilding :: CBotEngiMoveBuilding ( edict_t *pBuilding, Vector vNewLocation )
{
	addTask(new CFindPathTask(pBuilding));
	addTask(new CBotTaskEngiPickupBuilding(pBuilding));
	addTask(new CFindPathTask(vNewLocation));
	addTask(new CBotTaskEngiPlaceBuilding(vNewLocation));
}

void CBotEngiMoveBuilding :: init ()
{
	setID(SCHED_TF2_ENGI_MOVE_BUILDING);
}


///////////////////////////////////////////

CBotTF2PushPayloadBombSched :: CBotTF2PushPayloadBombSched (edict_t * ePayloadBomb)
{
	addTask(new CFindPathTask(CBotGlobals::entityOrigin(ePayloadBomb))); // first
	addTask(new CBotTF2PushPayloadBombTask(ePayloadBomb)); // second
}

void CBotTF2PushPayloadBombSched :: init ()
{
	setID(SCHED_TF2_PUSH_PAYLOADBOMB);
}
///////////////////////////////////

CBotTF2DefendPayloadBombSched :: CBotTF2DefendPayloadBombSched (edict_t * ePayloadBomb)
{
	addTask(new CFindPathTask(CBotGlobals::entityOrigin(ePayloadBomb))); // first
	addTask(new CBotTF2DefendPayloadBombTask(ePayloadBomb)); // second
}

void CBotTF2DefendPayloadBombSched :: init ()
{
	setID(SCHED_TF2_DEFEND_PAYLOADBOMB);
}


//////////////////////////////////////////////

CBotTFEngiUpgrade :: CBotTFEngiUpgrade ( edict_t *pBuilding )
{
	addTask(new CFindPathTask(pBuilding));
	addTask(new CBotTF2UpgradeBuilding(pBuilding));
}

void CBotTFEngiUpgrade :: init ()
{
	setID(SCHED_UPGRADE);
}

//////////////////////////////////////////////////
CBotBackstabSched :: CBotBackstabSched ( edict_t *pEnemy )
{
	Vector vrear;
	Vector vangles;

	AngleVectors(CBotGlobals::entityEyeAngles(pEnemy),&vangles);
	vrear = CBotGlobals::entityOrigin(pEnemy) - (vangles * 45) + Vector(0,0,32);

	addTask(new CFindPathTask(vrear));
	addTask(new CBotBackstab(pEnemy));
}

void CBotBackstabSched :: init ()
{
	setID(SCHED_BACKSTAB);
}

///////////

CBotTF2SnipeSched :: CBotTF2SnipeSched ( Vector vOrigin )
{
	addTask(new CFindPathTask(vOrigin)); // first
	addTask(new CBotTF2Snipe()); // second
}
void CBotTF2SnipeSched :: init ()
{
	setID(SCHED_SNIPE);
}
/////////

CBotTFEngiLookAfterSentry :: CBotTFEngiLookAfterSentry ( edict_t *pSentry )
{
	addTask(new CFindPathTask(pSentry)); // first
	addTask(new CBotTF2EngiLookAfter()); // second
}

void CBotTFEngiLookAfterSentry :: init ()
{
	setID(SCHED_LOOKAFTERSENTRY);
}

////////////
CBotTF2GetHealthSched :: CBotTF2GetHealthSched ( Vector vOrigin )
{
	addTask(new CFindPathTask(vOrigin)); // first
	addTask(new CBotTF2WaitHealthTask(vOrigin)); // second
}

void CBotTF2GetHealthSched :: init ()
{
	setID(SCHED_TF2_GET_HEALTH);
}
///////////////////////////////////////

CBotTF2GetAmmoSched :: CBotTF2GetAmmoSched ( Vector vOrigin )
{
	addTask(new CFindPathTask(vOrigin)); // first
	addTask(new CBotTF2WaitAmmoTask(vOrigin)); // second
}

void CBotTF2GetAmmoSched ::  init ()
{
	setID(SCHED_TF2_GET_AMMO);
}

//////////////////////////////////////////////
CBotTF2GetFlagSched :: CBotTF2GetFlagSched ( Vector vOrigin, bool bUseRoute, Vector vRoute )
{
	if ( bUseRoute )
		addTask(new CFindPathTask(vRoute));

	addTask(new CFindPathTask(vOrigin)); // first
	addTask(new CBotTF2WaitFlagTask(vOrigin)); // second
}

void CBotTF2GetFlagSched :: init ()
{
	setID(SCHED_TF2_GET_FLAG);
}
///////////////////////////////////////////
CBotUseTeleSched :: CBotUseTeleSched ( edict_t *pTele )
{
	// find path
	addTask(new CFindPathTask(pTele)); // first
	addTask(new CBotTFUseTeleporter(pTele)); // second
}
void CBotUseTeleSched :: init ()
{
	setID(SCHED_USE_TELE);
}
//////////////////////////////////////////

CBotUseDispSched :: CBotUseDispSched ( edict_t *pDisp )
{
	addTask(new CFindPathTask(pDisp)); // first
	addTask(new CBotTF2WaitHealthTask(CBotGlobals::entityOrigin(pDisp))); // second
	addTask(new CBotNest()); // third
}

void CBotUseDispSched :: init ()
{
	setID(SCHED_USE_DISPENSER);
}


////////////////////////////////////////////
void CBotSpySapBuildingSched :: init ()
{
	setID(SCHED_SPY_SAP_BUILDING);
}

CBotSpySapBuildingSched :: CBotSpySapBuildingSched ( edict_t *pBuilding, eEngiBuild id )
{
	addTask(new CFindPathTask(pBuilding)); // first
	addTask(new CBotTF2SpySap(pBuilding,id)); // second
}

///////////////////////////////////////////
CBotTF2FindFlagSched :: CBotTF2FindFlagSched ( Vector vOrigin )
{
	addTask(new CFindPathTask(vOrigin)); // first
	addTask(new CBotTF2WaitFlagTask(vOrigin,true)); // second
}

void CBotTF2FindFlagSched :: init ()
{
	setID(SCHED_TF2_FIND_FLAG);
}
/////////////////////////////////////////////
CBotPickupSched::CBotPickupSched( edict_t *pEdict )
{
	addTask(new CFindPathTask(pEdict));	
	addTask(new CMoveToTask(pEdict));
}

void CBotPickupSched :: init ()
{
	setID(SCHED_PICKUP);
}
////////////////////////////////////////////////
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
///////////////////////////////////////
CBotDefendSched ::CBotDefendSched ( Vector vOrigin )
{
	addTask(new CFindPathTask(vOrigin));
	addTask(new CBotDefendTask(vOrigin));
}

void CBotDefendSched :: init ()
{
	setID(SCHED_DEFEND);
}

//////

CBotRemoveSapperSched :: CBotRemoveSapperSched ( edict_t *pBuilding, eEngiBuild id )
{
	addTask(new CFindPathTask(CBotGlobals::entityOrigin(pBuilding)));
	addTask(new CBotRemoveSapper(pBuilding,id));
}

void CBotRemoveSapperSched :: init ()
{
	setID(SCHED_REMOVESAPPER);
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
///////////////
CGotoNestSched :: CGotoNestSched (int iWaypoint )
{
	//addTask(new CFindGoodHideSpot(1));
	//addTask(new CNestTask());
}
void CGotoNestSched :: init ()
{
	setID(SCHED_GOTONEST);
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
///////////////////////////////////////////
CBotAttackPointSched :: CBotAttackPointSched ( Vector vPoint, int iRadius, int iArea, bool bHasRoute, Vector vRoute )
{
	// First find random route 
	if ( bHasRoute )
		addTask(new CFindPathTask(vRoute)); // first

	addTask(new CFindPathTask(vPoint)); // second / first
	addTask(new CBotTF2AttackPoint(iArea,vPoint,iRadius)); // third / second 
}

void CBotAttackPointSched ::init ()
{	
	setID(SCHED_ATTACKPOINT);
}
///////////////
CBotTF2MessAroundSched :: CBotTF2MessAroundSched ( edict_t *pFriendly )
{
	addTask(new CMessAround(pFriendly));
}

void CBotTF2MessAroundSched :: init()
{
	setID(SCHED_MESSAROUND);
}
 
//////////////////////////////////////////////////
CBotDefendPointSched ::	CBotDefendPointSched ( Vector vPoint, int iRadius, int iArea )
{
	addTask(new CFindPathTask(vPoint)); // first
	addTask(new CBotTF2DefendPoint(iArea,vPoint,iRadius)); // second
}

void CBotDefendPointSched ::init ()
{
	setID(SCHED_DEFENDPOINT);
}


/////////////////////////////////////////////
void CBotSchedule :: execute ( CBot *pBot )
{
	// current task
	CBotTask *pTask = m_Tasks.GetFrontInfo();
	static eTaskState iState;

	if ( pTask == NULL )
	{
		m_bFailed = TRUE;
		return;
	}

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
			if ( CClients::clientsDebugging() && CClients::clientsDebugging(BOT_DEBUG_TASK) )
			{
				char dbg[512];

				pTask->debugString(dbg);

				CClients::clientDebugMsg(BOT_DEBUG_TASK,dbg,pBot);
			}

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