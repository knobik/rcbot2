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
//============================================================================//
//
// HPB_bot2.cpp - bot source code file (Copyright 2004, Jeffrey "botman" Broome)
//
//============================================================================//

#include <stdio.h>
#include <math.h>

//#define GAME_DLL

//#include "cbase.h"
#include "mathlib.h"
#include "vector.h"
#include "vplane.h"
#include "eiface.h"
#include "usercmd.h"
#include "bitbuf.h"
#include "in_buttons.h"
#include "vstdlib/random.h" // for random functions
#include "iservernetworkable.h" // may come in handy

//#include "cbase.h"
//#include "basehlcombatweapon.h"
//#include "basecombatcharacter.h"

#include "bot.h"
#include "bot_schedule.h"
#include "bot_buttons.h"
#include "bot_navigator.h"
#include "bot_css_bot.h"
#include "bot_coop.h"
#include "bot_zombie.h"
#include "bot_hldm_bot.h"
#include "bot_hl1dmsrc_bot.h"
#include "bot_fortress.h"
#include "bot_visibles.h"
//#include "bot_memory.h"
#include "bot_ga.h"
#include "bot_ga_ind.h"
#include "bot_perceptron.h"
#include "bot_ga_nn_const.h"
#include "bot_weapons.h"
#include "bot_profile.h"

#include "bot_mtrand.h"
//#include "vstdlib/random.h" // for random functions

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// instantiate bots -- make different for different mods
CBot **CBots::m_Bots = NULL;

const float CBot :: m_fAttackLowestHoldTime = 0.1f;
const float CBot :: m_fAttackHighestHoldTime = 0.6f;
const float CBot :: m_fAttackLowestLetGoTime = 0.1f;
const float CBot :: m_fAttackHighestLetGoTime = 0.5f;

int CBots :: m_iMaxBots = -1;
int CBots :: m_iMinBots = -1;
// add or kick bot time
float  CBots :: m_flAddKickBotTime = 0;

////////////////////////////////////////////////
void CBot :: runPlayerMove()
{
	extern ConVar bot_attack;

	//////////////////////////////////
	Q_memset( &cmd, 0, sizeof( cmd ) );
	//////////////////////////////////
	cmd.forwardmove = m_fForwardSpeed;
	cmd.sidemove = m_fSideSpeed;
	cmd.upmove = m_fUpSpeed;
	cmd.buttons = m_iButtons;
	cmd.impulse = m_iImpulse;
	cmd.viewangles = m_vViewAngles;
	cmd.weaponselect = m_iSelectWeapon;

	if ( bot_attack.GetInt() == 1 )
		cmd.buttons = IN_ATTACK;

	m_iSelectWeapon = 0;
	m_iImpulse = 0;

	if ( CClients::clientsDebugging() )
	{
			char dbg[512];

			sprintf(dbg,"m_pButtons = %d/%x, Weapon Select = %d, impulse = %d",cmd.buttons,cmd.buttons,cmd.weaponselect,cmd.impulse);

			CClients::clientDebugMsg(BOT_DEBUG_BUTTONS,dbg,this);
	}

	m_pController->PostClientMessagesSent();

	m_pController->RunPlayerMove(&cmd);

}

bool CBot :: wantsToShoot ()
{
	return true;
}

bool CBot :: startGame ()
{
	return true;
}

void CBot :: setEdict ( edict_t *pEdict)
{
	m_pEdict = pEdict;
	m_bUsed = true;
	m_szBotName[0] = 0;

	if ( m_pEdict )
	{
		m_pPlayerInfo = playerinfomanager->GetPlayerInfo(m_pEdict);
		m_pController = g_pBotManager->GetBotController(m_pEdict);		
		strncpy(m_szBotName,m_pPlayerInfo->GetName(),63);
		m_szBotName[63]=0;
	}
	else
	{
		return;
	}

	spawnInit();
}

bool CBot :: isUnderWater ()
{
	return m_pController->IsEFlagSet(EFL_TOUCHING_FLUID);
}

// return false if there is a problem
bool CBot :: createBotFromEdict(edict_t *pEdict, CBotProfile *pProfile)
{
	char szModel[128];

	init();
	setEdict(pEdict);
	setup();
	m_fTimeCreated = engine->Time();

	/////////////////////////////

	m_pProfile = pProfile;

	engine->SetFakeClientConVarValue(pEdict,"cl_team","default");
	engine->SetFakeClientConVarValue(pEdict,"cl_defaultweapon","pistol");
	engine->SetFakeClientConVarValue(pEdict,"cl_autowepswitch","1");	

	if ( m_pPlayerInfo && (pProfile->getTeam() != -1) )
		m_pPlayerInfo->ChangeTeam(pProfile->getTeam());

	/////////////////////////////
	// safe copy
	strncpy(szModel,pProfile->getModel(),127);
	szModel[127] = 0;

	if ( FStrEq(szModel,"default") )	
	{
		int iModel = randomInt(1,7);	

		if ( randomInt(0,1) )
			sprintf(szModel,"models/humans/Group03/Male_0%d.mdl",iModel);
		else
			sprintf(szModel,"models/humans/Group03/female_0%d.mdl",iModel);
	}

	m_iDesiredTeam = pProfile->getTeam();
	m_iDesiredClass = pProfile->getClass();

	engine->SetFakeClientConVarValue(pEdict,"cl_playermodel",szModel);
	engine->SetFakeClientConVarValue(pEdict,"hud_fastswitch","1");
	/////////////////////////////

	return true;
}

bool CBot :: FVisible ( Vector &vOrigin )
{
	return CBotGlobals::isVisible(getEyePosition(),vOrigin);
}

bool CBot :: FVisible ( edict_t *pEdict )
{
	return CBotGlobals::isVisible(m_pEdict,getEyePosition(),pEdict);
}

QAngle CBot :: eyeAngles ()
{
	return CBotGlobals::playerAngles(m_pEdict);
}

Vector CBot :: getEyePosition ()
{
	Vector vOrigin;

	gameclients->ClientEarPosition(m_pEdict,&vOrigin);

	return vOrigin;
}

bool CBot :: checkStuck ()
{
	static float fTime;

	float fSpeed;
	float fIdealSpeed;

	if ( !moveToIsValid() )
		return false;

	fTime = engine->Time();

	if ( m_fLastWaypointVisible == 0 )
	{
		m_bFailNextMove = false;

		if ( !hasSomeConditions(CONDITION_SEE_WAYPOINT) )
			m_fLastWaypointVisible = fTime;
	}
	else
	{
		if ( hasSomeConditions(CONDITION_SEE_WAYPOINT) )
			m_fLastWaypointVisible = 0;
		else
		{
			if ( (m_fLastWaypointVisible + 2.0) < fTime )
			{
				m_fLastWaypointVisible = 0;
				m_bFailNextMove = true;

				return true;
			}
		}
	}

	if ( m_fWaypointStuckTime && (m_fWaypointStuckTime < engine->Time()) )
	{
		m_bFailNextMove = true;
		m_fWaypointStuckTime = engine->Time() + randomFloat(7.0f,11.0f);
	}

	if ( m_fCheckStuckTime > fTime )
		return m_bThinkStuck;

	fSpeed = m_vVelocity.Length();
	fIdealSpeed = m_fIdealMoveSpeed;

	if ( m_pButtons->holdingButton(IN_DUCK) )
		fIdealSpeed /= 2;

	if ( fIdealSpeed == 0 )
		m_bThinkStuck = false; // not stuck
	else
	{
		float fPercentMoved = fSpeed/fIdealSpeed;

		if ( fPercentMoved < 0.1 )
		{
			m_bThinkStuck = true;
			m_pButtons->jump();
			m_pButtons->duck(0.25f,randomFloat(0.2f,0.4f));

			if ( m_fStrafeTime < engine->Time() )
			{
				if ( CBotGlobals::yawAngleFromEdict(m_pEdict,m_vMoveTo) > 0 )
					m_fSideSpeed = m_fIdealMoveSpeed/2;
				else
					m_fSideSpeed = -(m_fIdealMoveSpeed/2);

				m_fStrafeTime = engine->Time() + 2.0f;
			}

			m_fCheckStuckTime = m_fCheckStuckTime + 4.0f;
		}
	}

	return m_bThinkStuck;
}

bool CBot :: isVisible ( edict_t *pEdict )
{
	return m_pVisibles->isVisible(pEdict);
}

bool CBot :: canAvoid ( edict_t *pEntity )
{
	float distance;
	Vector vAvoidOrigin;

	if ( !CBotGlobals::entityIsValid(pEntity) )
		return false;
	if ( m_pEdict == pEntity ) // can't avoid self!!!
		return false;
	if ( m_pLookEdict == pEntity )
		return false;
	if ( m_pLastEnemy == pEntity )
		return false;

	vAvoidOrigin = CBotGlobals::entityOrigin(pEntity);

	distance = distanceFrom(vAvoidOrigin);

	if ( ( distance > 1 ) && ( distance < 160 ) && (fabs(getOrigin().z - vAvoidOrigin.z) < 32) )
	{
		SolidType_t solid = pEntity->GetCollideable()->GetSolid() ;

		if ( (solid == SOLID_BBOX) || (solid == SOLID_VPHYSICS) )
		{			
			return isEnemy(pEntity,false);
		}
	}

	return false;
}

void CBot :: reachedCoverSpot ()
{

}

// something now visiable or not visible anymore
void CBot :: setVisible ( edict_t *pEntity, bool bVisible )
{
	if ( bVisible )
	{
		if ( canAvoid(pEntity) )
		{
			if ( !m_pAvoidEntity || (distanceFrom(pEntity) < distanceFrom(m_pAvoidEntity)) )
					m_pAvoidEntity = pEntity;
		}
	}
	else
	{
		if ( m_pAvoidEntity == pEntity )
			m_pAvoidEntity = NULL;
	}

}

bool CBot :: isUsingProfile ( CBotProfile *pProfile )
{
	return (m_pProfile == pProfile);
}

void CBot :: currentlyDead ()
{
	/*if ( m_bNeedToInit )
	{
		spawnInit();
		m_bNeedToInit = false;
	}*/

	//attack();
	return;
}

CBotWeapon *CBot::getCurrentWeapon()
{
	return m_pWeapons->getActiveWeapon(m_pPlayerInfo->GetWeaponName());
}

void CBot :: selectWeaponName ( const char *szWeapon )
{
	m_pController->SetActiveWeapon(szWeapon);
}

void CBot :: selectWeaponSlot ( int iSlot )
{
	char cmd[16];

	sprintf(cmd,"slot%d",iSlot);

	helpers->ClientCommand(m_pEdict,cmd);
	//m_iSelectWeapon = iSlot;
}

CBotWeapon *CBot :: getBestWeapon (edict_t *pEnemy)
{
	return m_pWeapons->getBestWeapon(pEnemy);
}

void CBot :: debugMsg ( int iLev, const char *szMsg )
{
	if ( CClients::clientsDebugging () )
	{
		char szMsg2[512];

		sprintf(szMsg2,"(%s):%s",m_pPlayerInfo->GetName(),szMsg);

		CClients::clientDebugMsg (iLev,szMsg2,this);
	}
}

void CBot :: think ()
{
	float fTime = engine->Time();
//	Vector *pvVelocity;

	m_iLookPriority = 0;
	m_iMovePriority = 0;

	// if bot is not in game, start it!!!
	if ( !startGame() )
	{
		doButtons();
		return; // don't do anything just now
	}

	if ( m_pController->IsEFlagSet(EFL_BOT_FROZEN) )
		return;

	doButtons();

	if ( !isAlive() )
	{/*
	 // Dont need this anymore!! events does it, woohoo!
		if ( m_bNeedToInit )
		{
			spawnInit();
			m_bNeedToInit = false;
		}*/
		currentlyDead();

		return;
	}

	checkDependantEntities();

	//m_bNeedToInit = true;

	doMove();
	doLook();

	if ( m_fNextThink > fTime )
		return;

	m_fNextThink = fTime + 0.04;

	//////////////////////////////

	//m_pCurrentWeapon = m_pBaseCombatChar->GetActiveWeapon (); 

	m_pVisibles->updateVisibles();
	
	checkStuck();

	getTasks();	

	// update m_pEnemy with AttackTask
	m_pOldEnemy = m_pEnemy;
	m_pEnemy = NULL;

	m_pSchedules->execute(this);

	m_vGoal = m_pNavigator->getGoalOrigin();

	if ( m_pNavigator->hasNextPoint() )
	{		
		m_pNavigator->updatePosition();
	}
	else
		stopMoving();

	if ( m_pOldEnemy )
		findEnemy(m_pOldEnemy); // any better enemies than this one?
	else
		findEnemy();

	updateConditions();

	/*pvVelocity = CClassInterface::getVelocity(m_pEdict);

	if ( pvVelocity )
	{
		m_vVelocity = *pvVelocity;
	}
	else
	*/
	if ( m_fUpdateOriginTime < fTime )
	{
		Vector vOrigin = getOrigin();

		m_vVelocity = m_vLastOrigin-vOrigin;
		m_vLastOrigin = vOrigin;
		m_fUpdateOriginTime = fTime+1.0f;
	}

	modThink();

	m_iPrevHealth = m_pPlayerInfo->GetHealth();
}

CBot :: CBot()
{
	init(true);
}
/*
* init()
*
* initialize all bot variables 
* (this is called when bot is made for the first time)
*/
void CBot :: init (bool bVarInit)
{
	//m_bNeedToInit = false; // doing this now

	m_iAmmo = NULL;
	m_pButtons = NULL;
	m_pNavigator = NULL;
	m_pSchedules = NULL;
	m_pVisibles = NULL;
	m_pEdict = NULL;
//	m_pBaseEdict = NULL;
	m_pFindEnemyFunc = NULL;
	m_bUsed = false;
	m_pController = NULL;
	m_pPlayerInfo = NULL;
//	m_pGAvStuck = NULL;
//	m_pGAStuck = NULL;
//	m_pThinkStuck = NULL;
	m_pWeapons = NULL;
	m_fTimeCreated = 0;	
	m_pProfile = NULL;
	m_szBotName[0] = 0;
	m_fIdealMoveSpeed = 320;

	if ( bVarInit )
		spawnInit();
}

edict_t *CBot :: getEdict ()
{
	return m_pEdict;
}

void CBot :: updateConditions ()
{
	if ( m_pEnemy )
	{
		if ( !CBotGlobals::entityIsAlive(m_pEnemy) )
		{
			updateCondition(CONDITION_ENEMY_DEAD);
			m_pEnemy = NULL;
		}
		else
		{
			removeCondition(CONDITION_ENEMY_DEAD);

			// clear enemy
			if ( m_pVisibles->isVisible(m_pEnemy) )
			{
				updateCondition(CONDITION_SEE_CUR_ENEMY);
				removeCondition(CONDITION_ENEMY_OBSCURED);
			}
			else
			{
				m_pLastEnemy = m_pEnemy;
				m_vLastSeeEnemy = CBotGlobals::entityOrigin(m_pLastEnemy);

				removeCondition(CONDITION_SEE_CUR_ENEMY);
				updateCondition(CONDITION_ENEMY_OBSCURED);
			}
		}
	}
}

// Called when working out route
bool CBot :: canGotoWaypoint ( Vector vPrevWaypoint, CWaypoint *pWaypoint )
{
	if ( !pWaypoint->forTeam(getTeam()) )
		return false;

	if ( pWaypoint->hasFlag(CWaypointTypes::W_FL_OPENS_LATER) )
	{
		if ( !CBotGlobals::isVisible(m_pEdict,vPrevWaypoint,pWaypoint->getOrigin()) )
			return false;
	}

	return true;
}

void CBot::updatePosition()
{
	m_pNavigator->rollBackPosition();
}

bool CBot::handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy )
{
	if ( pWeapon )
	{
		if ( pWeapon->isMelee() )
			setMoveTo(CBotGlobals::entityOrigin(m_pEdict),2);

		if ( pWeapon->mustHoldAttack() )
			primaryAttack(true);
		else
			primaryAttack();
	}
	else
		primaryAttack();

	return true;
}

int CBot :: getHealth ()
{
	return m_pPlayerInfo->GetHealth();
}

float CBot :: getHealthPercent ()
{
	return (((float)m_pPlayerInfo->GetHealth())/m_pPlayerInfo->GetMaxHealth());
}

void CBot::	 stopMoving (int iPriority ) 
{ 
	if ( iPriority > m_iMovePriority )
	{
		m_bMoveToIsValid = false; 
		m_iMovePriority = iPriority;
		m_fWaypointStuckTime = 0;
		m_fCheckStuckTime = engine->Time() + randomFloat(3.0f,5.0f);
	}
}

void CBot :: spawnInit ()
{
	if ( m_pSchedules != NULL )
		m_pSchedules->freeMemory(); // clear tasks, im dead now!!
	if ( m_pVisibles != NULL )
		m_pVisibles->reset();	

	if ( m_pEdict && (m_iAmmo == NULL) )
		m_iAmmo = CClassInterface::getAmmoList(m_pEdict);

	m_fWaypointStuckTime = 0.0f;
	m_pPickup = NULL;
	m_pAvoidEntity = NULL;
	m_bThinkStuck = false;
	m_pLookEdict = NULL;
	m_fLookAroundTime = 0.0f;
	m_pAvoidEntity = NULL;
	m_bLookedForEnemyLast = false;
	////////////////////////
	m_iPrevHealth = 0;  // 
	////////////////////////
	m_vStuckPos = Vector(0,0,0);
	//m_iTimesStuck = 0;
	m_fUpdateDamageTime = 0;
	m_iAccumulatedDamage = 0;
	m_fCheckStuckTime = engine->Time() + 8.0f;
	m_fStuckTime = 0;
	m_vLastOrigin = Vector(0,0,0);
	m_vVelocity = Vector(0,0,0);
	m_fUpdateOriginTime = 0;
	m_fNextUpdateAimVector = 0;
	m_vAimVector = Vector(0,0,0);

	m_fLookSetTime = 0;
	m_vHurtOrigin = Vector(0,0,0);

	m_pOldEnemy = NULL;
	m_pEnemy = NULL;	

	m_vLastSeeEnemy = Vector(0,0,0);
	m_pLastEnemy = NULL; // enemy we were fighting before we lost it
	//m_pAvoidEntity = NULL; // avoid this guy
	m_fLastWaypointVisible = 0;
	m_vGoal = Vector(0,0,0);
	m_bHasGoal = false;
	m_fLookAtTimeStart = 0;
	m_fLookAtTimeEnd = 0;
	m_fNextThink = 0;
	m_iImpulse = 0;
	m_iButtons = 0;
	m_fForwardSpeed = 0;
	m_fSideSpeed = 0;
	m_fUpSpeed = 0;
	m_iConditions = 0;
	m_fStrafeTime = 0;

	m_vMoveTo = Vector(0,0,0);
	m_bMoveToIsValid = false;
	m_vLookAt = Vector(0,0,0);
	m_bLookAtIsValid = false;
	m_iSelectWeapon = 0;

	m_iLookTask = LOOK_WAYPOINT;

	//
	m_vViewAngles = QAngle(0,0,0);

	if ( m_pVisibles != NULL )
		m_pVisibles->reset();
}

bool CBot :: selectBotWeapon ( CBotWeapon *pBotWeapon )
{
	int id = pBotWeapon->getWeaponIndex();

	if ( id )
	{
		selectWeapon(id);
		return true;
	}
	return false;
}

void CBot :: touchedWpt ( CWaypoint *pWaypoint )
{
	m_fWaypointStuckTime = engine->Time() + randomFloat(7.0f,11.0f);

	if ( pWaypoint->getFlags() & CWaypointTypes::W_FL_JUMP )
		jump();
	if ( pWaypoint->getFlags() & CWaypointTypes::W_FL_CROUCH )
		duck();
}

// setup buttons and data structures
void CBot :: setup ()
{
	/////////////////////////////////
	m_pButtons = new CBotButtons();
	/////////////////////////////////
	m_pSchedules = new CBotSchedules();
	/////////////////////////////////
	m_pNavigator = new CWaypointNavigator(this);    
	/////////////////////////////////
	m_pVisibles = new CBotVisibles(this);
	/////////////////////////////////
	m_pFindEnemyFunc = new CFindEnemyFunc(this);
	/////////////////////////////////
	m_pWeapons = new CBotWeapons(this);
//	m_pGAvStuck = new CBotStuckValues();
//	m_pGAStuck = new CGA(10);
	//m_pThinkStuck = new CPerceptron(2);
}

/*
* called when a bot dies
*/
void CBot :: died ( edict_t *pKiller )
{	
	spawnInit();
}

/*
* called when a bot kills something
*/
void CBot :: killed ( edict_t *pVictim )
{	
	m_pLastEnemy = NULL;

	if ( pVictim )
	{
		// TO DO: been killed code
		//IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pVictim);

		//pVictim->GetIServerEntity()->GetBaseEntity()
	}
}

// called when bot shoots a wall or similar object -i.e. not the enemy
void CBot :: shotmiss ()
{

}
// shot an enemy (or teammate?)
void CBot :: shot ( edict_t *pEnemy )
{

}
// got shot by someone
void CBot :: hurt ( edict_t *pAttacker, int iHealthNow )
{
	if ( !pAttacker )
		return;

	m_vHurtOrigin = CBotGlobals::entityOrigin(pAttacker);

	if ( !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{
		m_fLookSetTime = 0;
		setLookAtTask(LOOK_HURT_ORIGIN,2);
		m_fLookSetTime = engine->Time() + randomFloat(2.5,8.0);
	}

	float fTime = engine->Time();

	if ( m_fUpdateDamageTime < fTime )
	{
		m_fUpdateDamageTime = fTime + 0.5;
		m_iAccumulatedDamage = 0;
	}

	m_iAccumulatedDamage += (m_iPrevHealth-iHealthNow);
	m_iPrevHealth = iHealthNow;	

	// TO DO: replace with perceptron method
	if ( m_iAccumulatedDamage > (m_pPlayerInfo->GetMaxHealth()*0.4) )
	{
		m_pSchedules->removeSchedule(SCHED_GOOD_HIDE_SPOT);
		m_pSchedules->addFront(new CGotoHideSpotSched(m_vHurtOrigin));
		m_iAccumulatedDamage = 0;
	}
}

void CBot :: setLookAtTask ( eLookTask lookTask, int iPriority ) 
{ 
	if ( (iPriority > m_iLookPriority) || ( m_fLookSetTime < engine->Time() ) )
	{
		m_iLookPriority = iPriority;
		m_iLookTask = lookTask; 
	}	
}

void CBot :: checkEntity ( edict_t **pEdict )
{
	if ( pEdict && *pEdict && !CBotGlobals::entityIsValid(*pEdict) )
		*pEdict = NULL;
}

void CBot :: checkDependantEntities ()
{
	checkEntity(&m_pOldEnemy);
	checkEntity(&m_pLookEdict);
	checkEntity(&m_pAvoidEntity);
	checkEntity(&m_pEnemy);
	checkEntity(&m_pLastEnemy);
	checkEntity(&m_pPickup);
}

void CBot :: findEnemy ( edict_t *pOldEnemy )
{
	m_pFindEnemyFunc->init();

	if ( pOldEnemy ) 
		m_pFindEnemyFunc->setOldEnemy(pOldEnemy);

	m_pVisibles->eachVisible(m_pFindEnemyFunc);

	m_pEnemy = m_pFindEnemyFunc->getBestEnemy();

	if ( m_pEnemy && (m_pEnemy != pOldEnemy) )
	{
		enemyFound(m_pEnemy);
	}
}

bool CBot :: isAlive ()
{
	return !m_pPlayerInfo->IsDead();
}


int CBot :: getTeam ()
{
	return m_pPlayerInfo->GetTeamIndex();
}

void CBot :: freeMapMemory ()
{
	// values
	/*if ( m_pGAvStuck != NULL )
	{
		m_pGAvStuck->freeMemory();
		delete m_pGAvStuck;
		m_pGAvStuck = NULL;
	}
	// my Ga
	if ( m_pGAStuck != NULL )
	{
		m_pGAStuck->freeGlobalMemory();
		delete m_pGAStuck;
		m_pGAStuck = NULL;
	}
	if ( m_pThinkStuck != NULL )
	{
		delete m_pThinkStuck;
		m_pThinkStuck = NULL;
	}*/
	/////////////////////////////////
	if ( m_pButtons != NULL )
	{
		m_pButtons->freeMemory();
		delete m_pButtons;
		m_pButtons = NULL;
	}
	/////////////////////////////////
	if ( m_pSchedules != NULL )
	{
		m_pSchedules->freeMemory();
		delete m_pSchedules;
		m_pSchedules = NULL;
	}
	/////////////////////////////////
	if ( m_pNavigator != NULL )
	{
		m_pNavigator->freeMapMemory();
		delete m_pNavigator;
		m_pNavigator = NULL;
	}
	/////////////////////////////////
	if ( m_pVisibles != NULL )
	{
		m_pVisibles->reset();
		delete m_pVisibles;
		m_pVisibles = NULL;
	}
	/////////////////////////////////
	if ( m_pFindEnemyFunc != NULL )
	{
		delete m_pFindEnemyFunc;
		m_pFindEnemyFunc = NULL;
	}
	/////////////////////////////////
	if ( m_pWeapons != NULL )
	{
		delete m_pWeapons;
		m_pWeapons = NULL;
	}

	m_iAmmo = NULL;
	/////////////////////////////////
	init();
}

bool CBot :: onLadder ()
{	
	return false;//return (m_pBaseEdict->GetMoveType() == MOVETYPE_LADDER);
}

void CBot :: freeAllMemory ()
{	
	freeMapMemory();
	return;
}

inline Vector CBot :: getOrigin ()
{	
	return m_pController->GetLocalOrigin();
	//return CBotGlobals::entityOrigin(m_pEdict);//m_pEdict->GetCollideable()->GetCollisionOrigin();
}

// found a new enemy
void CBot :: enemyFound (edict_t *pEnemy)
{

}
// work move velocity
void CBot :: doMove ()
{
	// Temporary measure to make bot follow me when i make listen serevr
	//setMoveTo(CBotGlobals::entityOrigin(INDEXENT(1)));

	// moving somewhere?
	if ( moveToIsValid () )
	{
		Vector2D move;
		float flMove = 0.0;
		float flSide = 0.0;
		// fAngle is got from world realting to bots origin, not angles
		float fAngle;
		float radians;

		if ( m_pAvoidEntity )
		{
			if ( canAvoid(m_pAvoidEntity) )
			{
				Vector m_vAvoid = (getOrigin()-CBotGlobals::entityOrigin(m_pAvoidEntity));

				m_vAvoid = m_vAvoid/m_vAvoid.Length();
				//?			
				m_vMoveTo = m_vMoveTo - (m_vAvoid*(distanceFrom(m_vMoveTo)));
			}
			else
				m_pAvoidEntity = NULL;
		}

		fAngle = CBotGlobals::yawAngleFromEdict(m_pEdict,m_vMoveTo);

		/////////
		radians = fAngle * 3.141592f / 180.0f; // degrees to radians
        // fl Move is percentage (0 to 1) of forward speed,
        // flSide is percentage (0 to 1) of side speed.
		
		move.x = cos(radians);
		move.y = sin(radians);

		move = move / move.Length();

		flMove = move.x;
		flSide = move.y;

		/*if ( m_bThinkStuck )
		{
			m_vVelocity
			m_fSideSpeed = h;
		}*/

		m_fForwardSpeed = m_fIdealMoveSpeed * flMove;

		// dont want this to override strafe speed if we're trying 
		// to strafe to avoid a wall for instance.
		if ( m_fStrafeTime < engine->Time() )
		{
			// side speed 
			m_fSideSpeed = m_fIdealMoveSpeed * flSide;
		}

		// moving less than 1.0 units/sec? just stop to 
		// save bot jerking around..
		if ( fabs(m_fForwardSpeed) < 1.0 )
			m_fForwardSpeed = 0.0;
		if ( fabs(m_fSideSpeed) < 1.0 )
			m_fSideSpeed = 0.0;

		if ( isUnderWater() )
		{
			if ( m_vMoveTo.z > (getOrigin().z + 32.0) )
				m_fUpSpeed = m_fIdealMoveSpeed;
			else if ( m_vMoveTo.z < (getOrigin().z - 32.0) )
				m_fUpSpeed = -m_fIdealMoveSpeed;
		}
	}
	else
	{	
		m_fForwardSpeed = 0;
		// bots side move speed
		m_fSideSpeed = 0;
		// bots upward move speed (e.g in water)
		m_fUpSpeed = 0;
	}
}

bool CBot :: FInViewCone ( edict_t *pEntity )
{	
	Vector origin = CBotGlobals::entityOrigin(pEntity);

	return ( ((origin - getEyePosition()).Length()>1) && (DotProductFromOrigin(origin) > 0) ); // 90 degree !! 0.422618f ); // 65 degree field of view   
}

float CBot :: DotProductFromOrigin ( Vector &pOrigin )
{
	static Vector vecLOS;
	static float flDot;
	
	Vector vForward;
	QAngle eyes;

	eyes = eyeAngles();

	// in fov? Check angle to edict
	AngleVectors(eyes,&vForward);
	
	vecLOS = pOrigin - getEyePosition();
	vecLOS = vecLOS/vecLOS.Length();
	
	flDot = DotProduct (vecLOS , vForward );
	
	return flDot; 

	//return m_pBaseEdict->FInViewCone(CBaseEntity::Instance(pEntity));
}

Vector CBot :: getAimVector ( edict_t *pEntity )
{
	Vector v_right;
	QAngle angles;
	
	if ( m_fNextUpdateAimVector > engine->Time() )
		return m_vAimVector;	

	angles = eyeAngles();
	// to the right
	angles.y += 90;

	CBotGlobals::fixFloatDegrees360(&angles.y);

	AngleVectors(angles,&v_right);

    v_right = v_right/VectorDistance(v_right); // normalize
	m_fNextUpdateAimVector = engine->Time() + randomFloat(0.1f,0.4f);

	m_vAimVector = CBotGlobals::entityOrigin(pEntity) + Vector(0,0,36) + Vector(v_right.x*randomFloat(-16,16),v_right.y*randomFloat(-16,16),randomFloat(-16,16));

	return m_vAimVector;
}

void CBot :: setLookAt ( Vector vNew )
{
	//if ( m_vLookAt != vNew )
	//{
		/*float fTime = engine->Time();
		m_fLookAtTimeStart = fTime;

		float angles = CBotGlobals::yawAngleFromEdict(m_pEdict,vNew);

		if ( angles != 0 )
		{
			//CBotGlobals::fixFloatAngle(&angles);
			//float screen_dist = fabs(180/angles);
			//float dist = distanceFrom(vNew);
			//float width = 72/dist;		

			// fitts meh
			m_fLookAtTimeEnd = fTime + (1.03+(0.96*log((2*screen_dist)/width)));
		}
		else
			m_fLookAtTimeEnd = fTime;*/

	//}

	m_vLookAt = vNew;
	m_bLookAtIsValid = true;
}


void CBot :: checkCanPickup ( edict_t *pPickup )
{


}

void CBot :: lookAtEdict ( edict_t *pEdict )
{
	m_pLookEdict = pEdict;
}

void CBot :: getLookAtVector ()
{
	switch ( m_iLookTask )
	{
	case LOOK_NONE:
		{
			stopLooking();
		}
		break;
	case LOOK_TSK_VECTOR:
		{
		//setLookAt(m_pSchedules->);
		}
		break;
	case LOOK_EDICT:
		{
			if ( m_pLookEdict )
				setLookAt(CBotGlobals::entityOrigin(m_pLookEdict)+Vector(0,0,32));
		}
		break;
	case LOOK_GROUND:
		setLookAt(getOrigin()-Vector(0,0,64));
		break;
	case LOOK_LAST_ENEMY:
		{
			if ( m_pLastEnemy )
				setLookAt(m_vLastSeeEnemy);
			else
				setLookAtTask(LOOK_WAYPOINT,2);
		}
		break;
	case LOOK_ENEMY:
		{
			if ( m_pEnemy )
				setLookAt(getAimVector(m_pEnemy));
			else
				setLookAtTask(LOOK_WAYPOINT,2);
		}		
		break;
	case LOOK_WAYPOINT:
		{
			setLookAt(Vector(m_vMoveTo.x,m_vMoveTo.y,getEyePosition().z));
		}
		break;
	case LOOK_SNIPE:
		{
			if ( m_fLookAroundTime < engine->Time() )
			{
				CTraceFilterWorldAndPropsOnly filter;
				Vector m_vAiming = Vector(0,0,0);
				float bestfraction = -1.0f;
				Vector eye = getEyePosition();
				Vector aiming;
				QAngle eyes = CBotGlobals::playerAngles(m_pEdict);
				float eyestart;
				int i;

				trace_t *res;

				AngleVectors(eyes,&aiming);

				res = CBotGlobals::getTraceResult();

				// find aiming vector
				CBotGlobals::traceLine (eye,eye+(aiming*4096),MASK_SOLID_BRUSHONLY,&filter);
				
				if ( res->fraction > bestfraction ) 
				{
					bestfraction = res->fraction;
					m_vAiming = res->endpos;
				}

				CBotGlobals::traceLine (eye,eye-(aiming*4096),MASK_SOLID_BRUSHONLY,&filter);

				if ( res->fraction > bestfraction ) 
				{
					bestfraction = res->fraction;
					m_vAiming = res->endpos;
				}

				eyestart = eyes.y - 10.0f;

				CBotGlobals::fixFloatAngle(&eyestart);

				for ( i = 0; i < 4; i ++ )
				{
					AngleVectors(eyes,&aiming);

					CBotGlobals::traceLine (eye,eye+(aiming*4096),MASK_SOLID_BRUSHONLY,&filter);

					if ( res->fraction > bestfraction ) 
					{
						bestfraction = res->fraction;
						m_vAiming = res->endpos;
					}

					eyestart = eyestart + 5.0f;
					CBotGlobals::fixFloatAngle(&eyestart);
				}

				setLookAt(m_vAiming);

				m_fLookAroundTime = engine->Time() + randomFloat(8.0f,18.0f);

			}
		}
		break;
	case LOOK_AROUND:
		{
			
			if ( m_fLookAroundTime < engine->Time() )
			{
				setLookAt(getEyePosition()+Vector(randomFloat(-128,128),randomFloat(-128,128),randomFloat(0,32)));
				m_fLookAroundTime = engine->Time() + randomFloat(8.0f,18.0f);
			//setLookAt();
			//setLookAt(...);
			}
		}
		break;
	case LOOK_HURT_ORIGIN:
		{
			setLookAt(m_vHurtOrigin);
		}
		break;
	default:
		break;
	}
}

int CBot :: getPlayerID ()
{
	return m_pPlayerInfo->GetUserID();
}

void CBot :: changeAngles ( float fSpeed, float *fIdeal, float *fCurrent, float *fUpdate )
{
   float fCurrent180;  // current +/- 180 degrees
   float fDiff;

   // turn from the current v_angle yaw to the ideal_yaw by selecting
   // the quickest way to turn to face that direction
   
   // find the difference in the current and ideal angle
   fDiff = fabs(*fCurrent - *fIdeal);

   // check if the bot is already facing the ideal_yaw direction...
   if (fDiff <= 0.1)
   {
      //fSpeed = fDiff;

      return;
   }

   // check if difference is less than the max degrees per turn
   if (fDiff < fSpeed)
      fSpeed = fDiff;  // just need to turn a little bit (less than max)

   // here we have four cases, both angle positive, one positive and
   // the other negative, one negative and the other positive, or
   // both negative.  handle each case separately...

   if ((*fCurrent >= 0) && (*fIdeal >= 0))  // both positive
   {
      if (*fCurrent > *fIdeal)
         *fCurrent -= fSpeed;
      else
         *fCurrent += fSpeed;
   }
   else if ((*fCurrent >= 0) && (*fIdeal < 0))
   {
      fCurrent180 = *fCurrent - 180;

      if (fCurrent180 > *fIdeal)
         *fCurrent += fSpeed;
      else
         *fCurrent -= fSpeed;
   }
   else if ((*fCurrent < 0) && (*fIdeal >= 0))
   {
      fCurrent180 = *fCurrent + 180;
      if (fCurrent180 > *fIdeal)
         *fCurrent += fSpeed;
      else
         *fCurrent -= fSpeed;
   }
   else  // (current < 0) && (ideal < 0)  both negative
   {
      if (*fCurrent > *fIdeal)
         *fCurrent -= fSpeed;
      else
         *fCurrent += fSpeed;
   }

   CBotGlobals::fixFloatAngle(fCurrent);
   //CBotGlobals::fixFloatDegrees360(fCurrent);

   if ( fUpdate )
		*fUpdate = *fCurrent;
}

bool CBot :: select_CWeapon ( CWeapon *pWeapon )
{
	CBotWeapon *pSelect = m_pWeapons->getWeapon(pWeapon);

	if ( pSelect )
	{
		int id = pSelect->getWeaponIndex();

		if ( id )
		{
			selectWeapon(id);
			return true;
		}

	}

	return false;
}

void CBot :: doLook ()
{
	//static float fSigmoid[] = {0.1,0.3,0.4,0.5,0.4,0.3,0.1};
	// what do we want to look at
	getLookAtVector();

	// temporary measure for bot to face listen server client
	//setLookAt(CBotGlobals::entityOrigin(INDEXENT(1)));

	// looking at something?
    if ( lookAtIsValid () )
	{	
		QAngle requiredAngles;

		extern ConVar bot_anglespeed;
		
		VectorAngles(m_vLookAt-getEyePosition(),requiredAngles);
		CBotGlobals::fixFloatAngle(&requiredAngles.x);
		CBotGlobals::fixFloatAngle(&requiredAngles.y);
		changeAngles(bot_anglespeed.GetFloat(),&requiredAngles.x,&m_vViewAngles.x,NULL);
		changeAngles(bot_anglespeed.GetFloat(),&requiredAngles.y,&m_vViewAngles.y,NULL);
		CBotGlobals::fixFloatAngle(&m_vViewAngles.x);
		CBotGlobals::fixFloatAngle(&m_vViewAngles.y);
	}

	//m_pController->SetLocalAngles(m_vViewAngles);
}

void CBot :: doButtons ()
{
	m_iButtons = m_pButtons->getBitMask();
}

void CBot :: secondaryAttack ( bool bHold )
{
	float fLetGoTime = 0.15;
	float fHoldTime = 0.12;

	if ( bHold )
	{
		fLetGoTime = 0.0;
		fHoldTime = 1.0;
	}

	// not currently in "letting go" stage?
	if ( bHold || m_pButtons->canPressButton(IN_ATTACK2) )
	{
		m_pButtons->holdButton
			(
				IN_ATTACK2,
				0/* reaction time? (time to press)*/,
				fHoldTime/* hold time*/,
				fLetGoTime/*let go time*/
			); 
	}
}

void CBot :: primaryAttack ( bool bHold )
{
	float fLetGoTime = 0.15f;
	float fHoldTime = 0.12f;

	if ( bHold )
	{
		fLetGoTime = 0.0f;
		fHoldTime = 2.0f;
	}

	// not currently in "letting go" stage?
	if ( bHold || m_pButtons->canPressButton(IN_ATTACK) )
	{
		m_pButtons->holdButton
			(
				IN_ATTACK,
				0/* reaction time? (time to press)*/,
				fHoldTime/* hold time*/,
				fLetGoTime/*let go time*/
			); 
	}
}

void CBot :: tapButton ( int iButton )
{
	m_pButtons->tap(iButton);
}

void CBot :: jump ()
{
	if ( m_pButtons->canPressButton(IN_JUMP) )
	{		
		m_pButtons->holdButton(IN_JUMP,0/* time to press*/,0.5/* hold time*/,0.5/*let go time*/); 
		// do the trademark jump & duck
		m_pButtons->holdButton(IN_DUCK,0.2/* time to press*/,0.3/* hold time*/,0.5/*let go time*/); 
	}
}

void CBot :: duck ( bool hold )
{
	if ( hold || m_pButtons->canPressButton(IN_DUCK) )
		m_pButtons->holdButton(IN_DUCK,0.0/* time to press*/,1.0/* hold time*/,0.5/*let go time*/); 
}

// TO DO: perceptron method
bool CBot::wantToFollowEnemy ()
{
	return ((m_pPlayerInfo->GetHealth()/m_pPlayerInfo->GetMaxHealth())>0.5);
}
////////////////////////////
void CBot :: getTasks (unsigned int iIgnore)
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

	// roam
	CWaypoint *pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomFlaggedWaypoint(getTeam()));

	if ( pWaypoint )
	{
		m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
	}

}

///////////////////////

bool CBots :: createBot (const char *szClass, const char *szTeam, const char *szName)
{		
	edict_t *pEdict;	
	CBotProfile *pBotProfile;

	char *szOVName = "";

	if ( (m_iMaxBots != -1) && (CBotGlobals::numClients() >= m_iMaxBots) )
		CBotGlobals::botMessage(NULL,0,"Can't create bot, max_bots reached");

	m_flAddKickBotTime = engine->Time() + 2.0f;

	pBotProfile = CBotProfiles::getRandomFreeProfile();

	if ( pBotProfile == NULL )
	{
		CBotGlobals::botMessage(NULL,0,"No bot profiles are free, creating a default bot...");

		pBotProfile = CBotProfiles::getDefaultProfile();

		if ( pBotProfile == NULL )
			return false;
	}

	if ( szClass && *szClass )
	{
		pBotProfile->setClass(atoi(szClass));
	}

	if ( szTeam && *szTeam )
	{
		pBotProfile->setTeam(atoi(szTeam));
	}
	
	if ( szName && *szName )
	{
		szOVName = (char*)szName;
	}
	else
		szOVName = pBotProfile->getName();

	pEdict = g_pBotManager->CreateBot( szOVName );

	if ( pEdict == NULL )
		return false;

	return ( m_Bots[slotOfEdict(pEdict)]->createBotFromEdict(pEdict,pBotProfile) );
}

void CBots :: botFunction ( IBotFunction *function )
{
	for ( unsigned int i = 0; i < MAX_PLAYERS; i ++ )
	{
		if ( m_Bots[i]->inUse() )
			function->execute (m_Bots[i]);
	}
}

int CBots :: slotOfEdict ( edict_t *pEdict )
{
	return engine->IndexOfEdict(pEdict) - 1;
}

void CBots :: init ()
{
	unsigned int i;

	m_Bots = (CBot**)malloc(sizeof(CBot*) * MAX_PLAYERS);

	for ( i = 0; i < MAX_PLAYERS; i ++ )
	{
		switch ( CBotGlobals::getCurrentMod()->getBotType() )
		{
		case BOTTYPE_CSS:
			m_Bots[i] = new CCSSBot();
			break;
		case BOTTYPE_HL2DM:
			m_Bots[i] = new CHLDMBot();
			break;
		case BOTTYPE_HL1DM:
			m_Bots[i] = new CHL1DMSrcBot();
			break;
		case BOTTYPE_COOP:
			m_Bots[i] = new CBotCoop();
			break;
		case BOTTYPE_TF2:
			m_Bots[i] = new CBotTF2();//MAX_PLAYERS];
			//CBotGlobals::setEventVersion(2);
			break;
		case BOTTYPE_FF:
			m_Bots[i] = new CBotFF();
			break;
		case BOTTYPE_ZOMBIE:
			m_Bots[i] = new CBotZombie();
			break;
		default:
			m_Bots[i] = new CBot();
			break;
		}
	}
}
int CBots :: numBots ()
{
	static CBot *pBot;

	int iCount = 0;

	for ( short int i = 0; i < MAX_PLAYERS; i ++ )
	{
		pBot = m_Bots[i];

		if ( pBot->inUse() )
			iCount++;		
	}

	return iCount;
}

CBot *CBots :: findBotByProfile ( CBotProfile *pProfile )
{	
	CBot *pBot = NULL;

	for ( short int i = 0; i < MAX_PLAYERS; i ++ )
	{
		pBot = m_Bots[i];

		if ( pBot->inUse() )
		{
			if ( pBot->isUsingProfile(pProfile) )
				return pBot;
		}
	}
	
	return NULL;
}

void CBots :: botThink ()
{
	static CBot *pBot;

	for ( short int i = 0; i < MAX_PLAYERS; i ++ )
	{
		pBot = m_Bots[i];

		if ( pBot->inUse() )
		{
			pBot->think();
			pBot->runPlayerMove();
		}
	}
	
	if ( needToAddBot () )
	{
		createBot(NULL,NULL,NULL);
	}
	else if ( needToKickBot () )
	{
		kickRandomBot();		
	}
}

CBot *CBots :: getBotPointer ( edict_t *pEdict )
{
	if ( !pEdict )
		return NULL;

	CBot *pBot = m_Bots[slotOfEdict(pEdict)];

	if ( pBot->inUse() )
		return pBot;

	return NULL;
}

void CBots :: freeMapMemory ()
{
	//bots should have been freed when they disconnected
	// just incase do this 
	for ( short int i = 0; i < MAX_PLAYERS; i ++ )
	{
		m_Bots[i]->freeMapMemory();
	}
}

void CBots :: freeAllMemory ()
{
	if ( m_Bots == NULL )
		return;

	for ( short int i = 0; i < MAX_PLAYERS; i ++ )
	{
		m_Bots[i]->freeAllMemory();
		delete m_Bots[i];
		m_Bots[i] = NULL;
	}

	delete[] m_Bots;
	m_Bots = NULL;
}

void CBots :: roundStart ()
{
	for ( short int i = 0; i < MAX_PLAYERS; i ++ )
	{
		if ( m_Bots[i]->inUse() )
			m_Bots[i]->spawnInit();
	}
}

void CBots :: mapInit ()
{
	m_flAddKickBotTime = 0;
}

bool CBots :: needToAddBot ()
{
	int iClients = CBotGlobals::numClients();

	return (m_flAddKickBotTime < engine->Time()) && (((m_iMinBots!=-1)&&(CBots::numBots() < m_iMinBots)) || ((iClients < m_iMaxBots)&&(m_iMaxBots!=-1)));
}

bool CBots :: needToKickBot ()
{
	if ( m_flAddKickBotTime < engine->Time() )
	{
		if ( ((m_iMinBots != -1 ) && (CBots::numBots() <= m_iMinBots)) )
			return false;

		if ( (m_iMaxBots != -1 ) && (CBotGlobals::numClients() > m_iMaxBots) )
			return true;
	}

	return false;
}

void CBots :: kickRandomBot ()
{
	dataUnconstArray<int> list;
	int index;
	CBot *tokick;
	char szCommand[512];
	//gather list of bots
	for ( short int i = 0; i < MAX_PLAYERS; i ++ )
	{
		if ( m_Bots[i]->inUse() )
			list.Add(i);
	}

	if ( list.IsEmpty() )
	{
		CBotGlobals::botMessage(NULL,0,"kickRandomBot() : No bots to kick");
		return;
	}

	index = list.Random();
	list.Clear();
	
	tokick = m_Bots[index];
	
	sprintf(szCommand,"kickid %d\n",tokick->getPlayerID());

	m_flAddKickBotTime = engine->Time() + 2.0f;

	engine->ServerCommand(szCommand);
}

void CBots :: kickRandomBotOnTeam ( int team )
{
	dataUnconstArray<int> list;
	int index;
	CBot *tokick;
	char szCommand[512];
	//gather list of bots
	for ( short int i = 0; i < MAX_PLAYERS; i ++ )
	{
		if ( m_Bots[i]->inUse() )
		{
			if ( m_Bots[i]->getTeam() == team )
				list.Add(i);
		}
	}

	if ( list.IsEmpty() )
	{
		CBotGlobals::botMessage(NULL,0,"kickRandomBotOnTeam() : No bots to kick");
		return;
	}

	index = list.Random();
	list.Clear();
	
	tokick = m_Bots[index];
	
	sprintf(szCommand,"kickid %d\n",tokick->getPlayerID());

	m_flAddKickBotTime = engine->Time() + 2.0f;

	engine->ServerCommand(szCommand);
}
////////////////////////