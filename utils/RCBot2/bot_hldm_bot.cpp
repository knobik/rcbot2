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
#include "in_buttons.h"
#include "bot.h"
#include "bot_hldm_bot.h"
#include "bot_client.h"
#include "bot_buttons.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_navigator.h"
#include "bot_waypoint.h"
#include "bot_utility.h"
#include "bot_task.h"
#include "bot_schedule.h"
#include "bot_waypoint.h"
#include "bot_weapons.h"
#include "bot_mtrand.h"
//#include "vstdlib/random.h" // for random functions

void CHLDMBot :: init ()
{
	CBot::init();
}

void CHLDMBot :: setup ()
{
	CBot::setup();
}


bool CHLDMBot :: startGame ()
{
	return true;
}

void CHLDMBot :: killed ( edict_t *pVictim )
{
	extern ConVar bot_beliefmulti;

	if ( pVictim && CBotGlobals::entityIsValid(pVictim) )
		m_pNavigator->belief(getOrigin(),getOrigin(),bot_beliefmulti.GetFloat(),distanceFrom(pVictim),BELIEF_SAFETY);
}

void CHLDMBot :: died ( edict_t *pKiller )
{
	extern ConVar bot_beliefmulti;

	spawnInit();

	//if ( randomInt(0,1) )
	m_pButtons->attack(); // respawn

	if ( pKiller )
	{
		if ( CBotGlobals::entityIsValid(pKiller) )
		{
			m_pNavigator->belief(getOrigin(),getOrigin(),bot_beliefmulti.GetFloat(),distanceFrom(pKiller),BELIEF_DANGER);
		}
	}
}

void CHLDMBot :: spawnInit ()
{
	CBot::spawnInit();

	if ( m_pWeapons )
		m_pWeapons->clearWeapons();

	m_FailedPhysObj = NULL;
	m_flSprintTime = 0;
	m_NearestPhysObj = NULL;
	m_pBattery = NULL;
	m_pHealthKit = NULL;
	m_pAmmoKit = NULL;
	m_pCurrentWeapon = NULL;
	m_pCharger = NULL;
	m_fFixWeaponTime = 0;
}

void CHLDMBot :: fixWeapons ()
{
	m_Weapons = CClassInterface::getWeaponList(m_pEdict);

	if ( m_pWeapons ) 
	{
		//m_pWeapons->clearWeapons();

		for ( unsigned int i = 0; i < MAX_WEAPONS; i ++ )
		{
			const char *classname;
			CWeapon *pBotWeapon;
			CBotWeapon *pHasWeapon;

			edict_t *pWeapon = INDEXENT(m_Weapons[i].GetEntryIndex());

			if ( pWeapon && CBotGlobals::entityIsValid(pWeapon) )
			{
				classname = pWeapon->GetClassName();

				pBotWeapon = CWeapons::getWeapon(classname);

				if ( pBotWeapon )
				{
					pHasWeapon = m_pWeapons->getWeapon(pBotWeapon);

					if ( !pHasWeapon || !pHasWeapon->hasWeapon() )
						m_pWeapons->addWeapon(pBotWeapon->getID(),pWeapon);
				}
			}
			//else
			//{
			//	CWeapons::getWeapon();
			//}
		}
	}
}

bool CHLDMBot :: isEnemy ( edict_t *pEdict,bool bCheckWeapons )
{
	extern ConVar rcbot_notarget;

	if ( rcbot_notarget.GetBool() && (ENTINDEX(pEdict) == 1) )
		return false;

	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( !CBotGlobals::entityIsAlive(pEdict) )
		return false;

	if ( CBotGlobals::getTeamplayOn() )
	{
		if ( CBotGlobals::getTeam(pEdict) == getTeam() )
			return false;
	}

	return true;	
}

bool CHLDMBot :: executeAction ( eBotAction iAction )
{
	switch ( iAction )
	{
	case BOT_UTIL_FIND_NEAREST_HEALTH:
		m_pSchedules->add(new CBotPickupSched(m_pHealthKit.get()));
		return true;
	case BOT_UTIL_HL2DM_FIND_ARMOR:
		m_pSchedules->add(new CBotPickupSched(m_pBattery.get()));
		return true;
	case BOT_UTIL_FIND_NEAREST_AMMO:
		m_pSchedules->add(new CBotPickupSched(m_pAmmoKit.get()));
		m_fUtilTimes[BOT_UTIL_FIND_NEAREST_AMMO] = engine->Time() + randomFloat(5.0f,10.0f);
		return true;
	case BOT_UTIL_HL2DM_USE_HEALTH_CHARGER:
		{
			CBotSchedule *pSched = new CBotSchedule();
			
			pSched->addTask(new CFindPathTask(m_pHealthCharger));
			pSched->addTask(new CBotHL2DMUseCharger(m_pHealthCharger));

			m_pSchedules->add(pSched);

			m_fUtilTimes[BOT_UTIL_HL2DM_USE_HEALTH_CHARGER] = engine->Time() + randomFloat(5.0f,10.0f);
			return true;
		}
		break;
	case BOT_UTIL_HL2DM_USE_CHARGER:
		{
			CBotSchedule *pSched = new CBotSchedule();
			
			pSched->addTask(new CFindPathTask(m_pCharger));
			pSched->addTask(new CBotHL2DMUseCharger(m_pCharger));

			m_pSchedules->add(pSched);

			m_fUtilTimes[BOT_UTIL_HL2DM_USE_CHARGER] = engine->Time() + randomFloat(5.0f,10.0f);
			return true;
		}
		break;
	case BOT_UTIL_HL2DM_GRAVIGUN_PICKUP:
		{
		CBotSchedule *pSched = new CBotSchedule(new CBotGravGunPickup(m_pCurrentWeapon,m_NearestPhysObj));
		m_pSchedules->add(pSched);
		return true;
		}
	case BOT_UTIL_FIND_LAST_ENEMY:
		{
			Vector vVelocity = Vector(0,0,0);
			CClient *pClient = CClients::get(m_pLastEnemy);
			CBotSchedule *pSchedule = new CBotSchedule();
			
			CFindPathTask *pFindPath = new CFindPathTask(m_vLastSeeEnemy);	
			
			if ( pClient )
				vVelocity = pClient->getVelocity();

			pSchedule->addTask(pFindPath);
			pSchedule->addTask(new CFindLastEnemy(m_vLastSeeEnemy,vVelocity));

			//////////////
			pFindPath->setNoInterruptions();

			m_pSchedules->add(pSchedule);

			m_bLookedForEnemyLast = true;

			return true;
		}
	case BOT_UTIL_ROAM:
		// roam
		CWaypoint *pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomFlaggedWaypoint(getTeam()));

		if ( pWaypoint )
		{
			m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
			return true;
		}

		break;
	}

	return false;
}
bool CHLDMBot :: handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy )
{
	if ( pWeapon )
	{
		extern ConVar rcbot_enemyshoot_gravgun_fov;

		clearFailedWeaponSelect();

		if ( pWeapon->isMelee() )
			setMoveTo(CBotGlobals::entityOrigin(m_pEdict));

		if ( (pWeapon->getID() == HL2DM_WEAPON_PHYSCANNON) && (DotProductFromOrigin(m_vAimVector) < rcbot_enemyshoot_gravgun_fov.GetFloat()) ) 
			return true; // keep enemy / don't shoot : until angle between enemy is less than 20 degrees

		if ( pWeapon->canUseSecondary() && pWeapon->getAmmo(this,2) && pWeapon->secondaryInRange(distanceFrom(pEnemy)) )
			secondaryAttack();

		if ( pWeapon->mustHoldAttack() )
			primaryAttack(true);
		else
			primaryAttack();
	}
	else
		primaryAttack();

	return true;
}

void CHLDMBot :: getTasks (unsigned int iIgnore)
{
	static CBotUtilities utils;
	static CBotUtility *next;

	if ( !m_pSchedules->isEmpty() )
		return;

	CBotWeapon *gravgun = m_pWeapons->getWeapon(CWeapons::getWeapon(HL2DM_WEAPON_PHYSCANNON));

	if ( gravgun )
	{
		edict_t *pent = INDEXENT(gravgun->getWeaponIndex());

		if ( CBotGlobals::entityIsValid(pent) )
		{
			ADD_UTILITY(BOT_UTIL_HL2DM_GRAVIGUN_PICKUP,(!m_pEnemy||(m_pCurrentWeapon&&(strcmp("weapon_physcannon",m_pCurrentWeapon->GetClassName())))) && gravgun && gravgun->hasWeapon() && (m_NearestPhysObj.get()!=NULL) && (gravgun->getWeaponIndex() > 0) && (CClassInterface::gravityGunObject(INDEXENT(gravgun->getWeaponIndex()))==NULL),1.0f);
		}
	}

	ADD_UTILITY(BOT_UTIL_FIND_NEAREST_HEALTH,(m_pHealthKit.get()!=NULL) && (getHealthPercent()<1.0f),1.0f-getHealthPercent());
	ADD_UTILITY(BOT_UTIL_HL2DM_FIND_ARMOR,(m_pBattery.get() !=NULL) && (getArmorPercent()<1.0f),(1.0f-getArmorPercent())*0.75f);
	ADD_UTILITY(BOT_UTIL_FIND_NEAREST_AMMO,(m_pAmmoKit.get() !=NULL) && (getAmmo(0)<5),0.01f*(100-getAmmo(0)));
	ADD_UTILITY(BOT_UTIL_HL2DM_USE_CHARGER,(m_pCharger.get() !=NULL) && (getArmorPercent()<1.0f),(1.0f-getArmorPercent())*0.75f);
	ADD_UTILITY(BOT_UTIL_HL2DM_USE_HEALTH_CHARGER,(m_pHealthCharger.get() != NULL) && (getHealthPercent()<1.0f),(1.0f-getHealthPercent()));
	ADD_UTILITY(BOT_UTIL_ROAM,true,0.1f);
	ADD_UTILITY(BOT_UTIL_FIND_LAST_ENEMY,wantToFollowEnemy() && !m_bLookedForEnemyLast && m_pLastEnemy,getHealthPercent()*(getArmorPercent()+0.1));

	utils.execute();

	while ( (next = utils.nextBest()) != NULL )
	{
		if ( executeAction(next->getId()) )
		{
			if ( m_fUtilTimes[next->getId()] < engine->Time() )
				m_fUtilTimes[next->getId()] = engine->Time() + randomFloat(0.1f,2.0f);
			/*if ( CClients::clientsDebugging() )
			{
				CClients::clientDebugMsg(BOT_DEBUG_UTIL,g_szUtils[util->getId()],this);
			}*/
			break;
		}
	}

	utils.freeMemory();
}

void CHLDMBot :: modThink ()
{
	extern ConVar rcbot_jump_obst_dist;
	extern ConVar rcbot_jump_obst_speed;

	if ( !CBotGlobals::entityIsValid(m_NearestPhysObj) )
		m_NearestPhysObj = NULL;

	if ( !CBotGlobals::entityIsValid(m_FailedPhysObj) )
		m_FailedPhysObj = NULL;

	if ( m_fFixWeaponTime < engine->Time() )
	{
		fixWeapons();
		m_pCurrentWeapon = CClassInterface::getCurrentWeapon(m_pEdict);
		m_fFixWeaponTime = engine->Time() + 1.0f;
	}

	if ( m_pCurrentWeapon )
		CClassInterface::getWeaponClip(m_pCurrentWeapon,&m_iClip1,&m_iClip2);

	if ( CClassInterface::onLadder(m_pEdict) != NULL )
	{
		setMoveLookPriority(MOVELOOK_OVERRIDE);
		setLookAtTask(LOOK_WAYPOINT);
		m_pButtons->holdButton(IN_FORWARD,0,1,0);
		setMoveLookPriority(MOVELOOK_MODTHINK);
	}

	if ( (m_fCurrentDanger >= 20.0f) && (CClassInterface::auxPower(m_pEdict) > 90.f ) && (m_flSprintTime < engine->Time()))
	{
		m_pButtons->holdButton(IN_SPEED,0,1,0);
	}
	else if (( m_fCurrentDanger < 1 ) || (CClassInterface::auxPower(m_pEdict) < 5.0f ))
	{
		m_flSprintTime = engine->Time() + randomFloat(5.0f,20.0f);
	}

	if ( m_fLastSeeEnemy && ((m_fLastSeeEnemy + 5.0)<engine->Time()) )
	{
		m_fLastSeeEnemy = 0;
		m_pButtons->tap(IN_RELOAD);
	}

	if ( m_NearestPhysObj.get() )
	{
		bool bCarry = false;
		edict_t *pEntity = m_NearestPhysObj.get();

		if ( m_pCurrentWeapon && !strcmp("weapon_physcannon",m_pCurrentWeapon->GetClassName()) )
		{
			bCarry = (CClassInterface::gravityGunObject(m_pCurrentWeapon) == m_NearestPhysObj.get());
		}

		if ( !bCarry && (distanceFrom(pEntity) < rcbot_jump_obst_dist.GetFloat()) )
		{
			Vector vel;

			if ( CClassInterface::getVelocity(m_pEdict,&vel) )
			{
				Vector v_size = pEntity->GetCollideable()->OBBMaxs() - pEntity->GetCollideable()->OBBMins();

				if ( v_size.z <= 48 ) // jump height
				{
					if ( (vel.Length() > rcbot_jump_obst_speed.GetFloat()) && 
						
						(CBotGlobals::entityOrigin(pEntity)-(CBotGlobals::entityOrigin(m_pEdict)+Vector(0,0,16)+((vel/vel.Length())*distanceFrom(pEntity)))).Length() <= (v_size.Length()/2) )
					{
						if ( randomInt(0,1) )
							jump();
					}
				}
			}
		}
		
	}
}

void CHLDMBot :: setVisible ( edict_t *pEntity, bool bVisible )
{
	static float fDist;

	CBot::setVisible(pEntity,bVisible);

	fDist = distanceFrom(pEntity);

	if ( bVisible )
	{
		if ( ( strncmp(pEntity->GetClassName(),"item_ammo",9)==0 ) && 
			( !m_pAmmoKit.get() || (fDist<distanceFrom(m_pAmmoKit.get())) ))
		{
			m_pAmmoKit = pEntity;
		}
		else if ( ( strncmp(pEntity->GetClassName(),"item_health",11)==0 ) && 
			( !m_pHealthKit.get() || (fDist<distanceFrom(m_pHealthKit.get())) ))
		{
			m_pHealthKit = pEntity;
		}
		else if ( ( strcmp(pEntity->GetClassName(),"item_battery")==0 ) && 
			( !m_pBattery.get() || (fDist<distanceFrom(m_pBattery.get())) ))
		{
			m_pBattery = pEntity;
		
		}
		else if ( (pEntity != m_FailedPhysObj) && ( strncmp(pEntity->GetClassName(),"prop_physics",12)==0 ) && 
			( !m_NearestPhysObj.get() || (fDist<distanceFrom(m_NearestPhysObj.get())) ))
		{
			m_NearestPhysObj = pEntity;
		}
		else if ( ( strncmp(pEntity->GetClassName(),"item_suitcharger",16)==0 ) && 
			( !m_pCharger.get() || (fDist<distanceFrom(m_pCharger.get())) ))
		{
			m_pCharger = pEntity;
		}
		else if ( ( strncmp(pEntity->GetClassName(),"item_healthcharger",18)==0 ) && 
			( !m_pHealthCharger.get() || (fDist<distanceFrom(m_pHealthCharger.get())) ))
		{
			m_pHealthCharger = pEntity;
		}
	}
	else
	{
		if ( m_pAmmoKit == pEntity )
			m_pAmmoKit = NULL;
		else if ( m_pHealthKit == pEntity )
			m_pHealthKit = NULL;
		else if ( m_pBattery == pEntity )
			m_pBattery = NULL;
		else if ( m_NearestPhysObj == pEntity )
			m_NearestPhysObj = NULL;
		else if ( m_pCharger == pEntity )
			m_pCharger = NULL;
		else if ( m_pHealthCharger == pEntity )
			m_pHealthCharger = NULL;
	}

}

void CHLDMBot :: enemyLost ()
{
	m_pSchedules->freeMemory();
}