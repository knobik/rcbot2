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
#include "bot_dod_bot.h"
#include "in_buttons.h"
#include "bot_buttons.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_getprop.h"
#include "bot_mtrand.h"
#include "bot_task.h"
#include "bot_schedule.h"
#include "bot_weapons.h"
#include "bot_waypoint.h"
#include "bot_waypoint_locations.h"
//#include "vstdlib/random.h" // for random functions

void CDODBot :: init ()
{
	CBot::init();

	m_iSelectedClass = -1;
}

void CDODBot :: setup ()
{
	CBot::setup();
}

bool CDODBot::canGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint)
{
	static int iTeam;
	
	iTeam = getTeam();

	if ( CBot::canGotoWaypoint(vPrevWaypoint,pWaypoint) )
	{
		if ( (iTeam == TEAM_ALLIES) && pWaypoint->hasFlag(CWaypointTypes::W_FL_NOALLIES) )
		{
			return false;
		}
		else if ( (iTeam == TEAM_AXIS) && pWaypoint->hasFlag(CWaypointTypes::W_FL_NOAXIS) )
		{
			return false;
		}

		return true;
	}

	return false;
}

void CDODBot :: setVisible ( edict_t *pEntity, bool bVisible )
{
	//static float fDist;
	//const char *szClassname;

	CBot::setVisible(pEntity,bVisible);

	if ( bVisible && !(CClassInterface::getEffects(pEntity) & EF_NODRAW) )
	{
		if ( strcmp(pEntity->GetClassName(),"dod_control_point") && 
			(!m_pNearestFlag || (distanceFrom(pEntity)<distanceFrom(m_pNearestFlag)) ) )
		{
			m_pNearestFlag = pEntity;
		}
	}
	else
	{
		if ( pEntity == m_pNearestFlag )
			m_pNearestFlag = NULL;
	}
}

void CDODBot :: selectedClass ( int iClass )
{
	m_iSelectedClass = iClass;
}

bool CDODBot :: startGame ()
{
	if ( m_pPlayerInfo->GetTeamIndex() <= 1 )
	{
		m_pPlayerInfo->ChangeTeam(randomInt(2,3));
		return false;

	}

	if ( CClassInterface::getDesPlayerClassDOD(m_pEdict) == -1 )
	{
		//if ( m_iDesiredClass == 0 )
		//	m_iDesiredClass = randomInt(1,6);

		engine->ClientCommand(m_pEdict,"joinclass rifleman"); 

		/*switch ( m_iDesiredClass )
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		}
		engine->ClientCommand(m_pEdict,"joinclass %d",m_iDesiredClass); */

		return false;
	}

	//if ( m_pPlayerInfo->get

	return true;
}

void CDODBot :: killed ( edict_t *pVictim )
{
	return;
}

void CDODBot :: died ( edict_t *pKiller )
{
	spawnInit();

	if ( randomInt(0,1) )
		m_pButtons->attack();
}

void CDODBot :: spawnInit ()
{
	CBot::spawnInit();

	//if ( m_pWeapons )
	//	m_pWeapons->clearWeapons();

	m_CurrentUtil = BOT_UTIL_MAX;
	// reset objects
	m_flSprintTime = 0;
	m_pCurrentWeapon = NULL;
	m_fFixWeaponTime = 0;
}

bool CDODBot :: isEnemy ( edict_t *pEdict,bool bCheckWeapons )
{
	if ( !pEdict )
		return false;

	if ( !pEdict->GetUnknown() )
		return false; // left the server

	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > gpGlobals->maxClients) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	return true;	
}

void CDODBot :: modThink ()
{
	m_fIdealMoveSpeed = CClassInterface::getMaxSpeed(m_pEdict);

	// find weapons and neat stuff
	if ( m_fFixWeaponTime < engine->Time() )
	{
		fixWeapons();
		m_pCurrentWeapon = CClassInterface::getCurrentWeapon(m_pEdict);
		m_fFixWeaponTime = engine->Time() + 1.0f;
	}

	CClassInterface::getPlayerInfoDOD(m_pEdict,&m_bProne,&m_flStamina);

	if ( (m_fCurrentDanger >= 20.0f) && (m_flStamina > 90.f ) && (m_flSprintTime < engine->Time()))
	{
		m_pButtons->holdButton(IN_SPEED,0,1,0);
	}
	else if (( m_fCurrentDanger < 1 ) || (m_flStamina < 5.0f ))
	{
		m_flSprintTime = engine->Time() + randomFloat(5.0f,20.0f);
	}

	if ( m_fLastSeeEnemy && ((m_fLastSeeEnemy + 5.0)<engine->Time()) )
	{
		m_fLastSeeEnemy = 0;
		m_pButtons->tap(IN_RELOAD);
	}

	if ( m_pNearestFlag )
	{

	}
}

void CDODBot :: fixWeapons ()
{
	m_Weapons = CClassInterface::getWeaponList(m_pEdict);

	if ( m_pWeapons ) 
	{
		//m_pWeapons->clearWeapons();

		// loop through the weapons array in the CBaseCombatCharacter
		for ( unsigned int i = 0; i < MAX_WEAPONS; i ++ )
		{
			// get the information from the bot 
			const char *classname;
			CWeapon *pBotWeapon;
			CBotWeapon *pHasWeapon;

			edict_t *pWeapon = INDEXENT(m_Weapons[i].GetEntryIndex());

			if ( pWeapon && CBotGlobals::entityIsValid(pWeapon) )
			{
				classname = pWeapon->GetClassName();

				// find this weapon in our initialized array in the code
				pBotWeapon = CWeapons::getWeapon(classname);

				if ( pBotWeapon )
				{
					// see if the bot has this weapon or not already
					pHasWeapon = m_pWeapons->getWeapon(pBotWeapon);

					// if the bot doesn't have it or the state has changed, update
					if ( !pHasWeapon || !pHasWeapon->hasWeapon() )
						m_pWeapons->addWeapon(pBotWeapon->getID(),pWeapon);
				}
			}
		}
	}
}

void CDODBot ::defending()
{
	// check to go prone or not
}

bool CDODBot :: executeAction ( eBotAction iAction )
{
	switch ( iAction )
	{
	case BOT_UTIL_DEFEND_POINT:
		{
			CWaypoint *pGoal = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT,getTeam(),0,0,this);
			
			CWaypoint *pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),pGoal->getOrigin());

			if ( pWaypoint )
			{
				CBotSchedule *defend = new CBotSchedule();

				defend->addTask(new CFindPathTask(pWaypoint->getOrigin()));
				defend->addTask(new CBotDefendTask(pWaypoint->getOrigin(),randomFloat(15.0f,20.0f),0,true,pGoal->getOrigin()));
				// add defend task
				m_pSchedules->add(defend);
				
				return true;
			}
		}
		break;
	case BOT_UTIL_ATTACK_POINT:
		{
			CWaypoint *pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT,getTeam(),0,0,this);

			if ( pWaypoint )
			{
				// add capture point task
				m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
				return true;
			}
		}
		break;
	case BOT_UTIL_ROAM:
		{
		// roam
			CWaypoint *pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomFlaggedWaypoint(getTeam()));

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
				return true;
			}
		}
		break;
	default:
		break;
	}

	return false;
}

void CDODBot :: getTasks (unsigned int iIgnore)
{

	//CBotUtilities util;
	//CBot::getTasks(iIgnore);

	//ADD_UTILITY(BOT_UTIL_CAPTURE_POINT,,(numFriendlies/numEnemies)/numPlayersOnTeam);
	//ADD_UTILITY();

	static CBotUtilities utils;
	static CBotUtility *next;
	static CBotWeapon *grenade;
	static CWeapon *pWeapon;

	// if condition has changed or no tasks
	if ( !hasSomeConditions(CONDITION_CHANGED) && !m_pSchedules->isEmpty() )
		return;

	removeCondition(CONDITION_CHANGED);

	// low on health? Pick some up if there's any near by
	//ADD_UTILITY(BOT_UTIL_HL2DM_USE_HEALTH_CHARGER,(m_pHealthCharger.get() != NULL) && (getHealthPercent()<1.0f),(1.0f-getHealthPercent()));
	//ADD_UTILITY(BOT_UTIL_FIND_NEAREST_HEALTH,(m_pHealthKit.get()!=NULL) && (getHealthPercent()<1.0f),1.0f-getHealthPercent());

	// low on armor?
	//ADD_UTILITY(BOT_UTIL_HL2DM_FIND_ARMOR,(m_pBattery.get() !=NULL) && (getArmorPercent()<1.0f),(1.0f-getArmorPercent())*0.75f);
	//ADD_UTILITY(BOT_UTIL_HL2DM_USE_CHARGER,(m_pCharger.get() !=NULL) && (getArmorPercent()<1.0f),(1.0f-getArmorPercent())*0.75f);
	
	// low on ammo? ammo nearby?
	//ADD_UTILITY(BOT_UTIL_FIND_NEAREST_AMMO,(m_pAmmoKit.get() !=NULL) && (getAmmo(0)<5),0.01f*(100-getAmmo(0)));

	// always able to roam around -- low utility
	ADD_UTILITY(BOT_UTIL_ROAM,true,0.01f);

	// I have an enemy 
	ADD_UTILITY(BOT_UTIL_FIND_LAST_ENEMY,wantToFollowEnemy() && !m_bLookedForEnemyLast && m_pLastEnemy && CBotGlobals::entityIsValid(m_pLastEnemy) && CBotGlobals::entityIsAlive(m_pLastEnemy),getHealthPercent()*(getArmorPercent()+0.1));

	ADD_UTILITY(BOT_UTIL_ATTACK_POINT,true,randomFloat(0.4,0.6f));
	ADD_UTILITY(BOT_UTIL_DEFEND_POINT,true,randomFloat(0.4,0.6f));

	/*if ( !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && hasSomeConditions(CONDITION_SEE_LAST_ENEMY_POS) && m_pLastEnemy && m_fLastSeeEnemy && ((m_fLastSeeEnemy + 10.0) > engine->Time()) && m_pWeapons->hasWeapon(HL2DM_WEAPON_FRAG) )
	{
		float fDistance = distanceFrom(m_vLastSeeEnemyBlastWaypoint);

		if ( ( fDistance > BLAST_RADIUS ) && ( fDistance < 1500 ) )
		{
			CWeapon *pWeapon = CWeapons::getWeapon(HL2DM_WEAPON_FRAG);
			CBotWeapon *pBotWeapon = m_pWeapons->getWeapon(pWeapon);

			ADD_UTILITY(BOT_UTIL_THROW_GRENADE, pBotWeapon && (pBotWeapon->getAmmo(this) > 0) ,1.0f-(getHealthPercent()*0.2));
		}
	}*/

	/*if ( m_pNearbyWeapon.get() )
	{
		pWeapon = CWeapons::getWeapon(m_pNearbyWeapon.get()->GetClassName());

		if ( pWeapon && !m_pWeapons->hasWeapon(pWeapon->getID()) )
		{
			ADD_UTILITY(BOT_UTIL_PICKUP_WEAPON, true , 0.6f + pWeapon->getPreference()*0.1f);
		}
	}*/

	utils.execute();

	while ( (next = utils.nextBest()) != NULL )
	{
		if ( !m_pSchedules->isEmpty() && (m_CurrentUtil != next->getId() ) )
			m_pSchedules->freeMemory();

		if ( executeAction(next->getId()) )
		{
			m_CurrentUtil = next->getId();

			if ( m_fUtilTimes[next->getId()] < engine->Time() )
				m_fUtilTimes[next->getId()] = engine->Time() + randomFloat(0.1f,2.0f); // saves problems with consistent failing

			if ( CClients::clientsDebugging() )
				CClients::clientDebugMsg(BOT_DEBUG_UTIL,g_szUtils[next->getId()],this);
			
			break;
		}
	}

	utils.freeMemory();
}

bool CDODBot :: select_CWeapon ( CWeapon *pWeapon )
{
	char cmd[128];

	sprintf(cmd,"use %s",pWeapon->getWeaponName());

	helpers->ClientCommand(m_pEdict,cmd);

	return true;
}

bool CDODBot :: selectBotWeapon ( CBotWeapon *pBotWeapon )
{
	CWeapon *pSelect = pBotWeapon->getWeaponInfo();

	if ( pSelect )
	{
		//int id = pSelect->getWeaponIndex();
		char cmd[128];

		sprintf(cmd,"use %s",pSelect->getWeaponName());

		helpers->ClientCommand(m_pEdict,cmd);

		return true;
	}
	else
		failWeaponSelect();

	return false;
}
