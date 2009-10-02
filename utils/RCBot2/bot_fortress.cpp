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
#include "bot_mods.h"
#include "bot_visibles.h"
#include "bot_weapons.h"
#include "bot_waypoint_locations.h"
#include "in_buttons.h"
#include "bot_utility.h"
#include "bot_script.h"

#include "bot_mtrand.h"

extern ConVar bot_beliefmulti;

// Payload stuff by   The_Shadow

//#include "vstdlib/random.h" // for random functions
void CBroadcastCapturedPoint :: execute ( CBot *pBot )
{
	((CBotTF2*)pBot)->pointCaptured(m_iPoint,m_iTeam,m_szName);
}

CBroadcastCapturedPoint :: CBroadcastCapturedPoint ( int iPoint, int iTeam, const char *szName )
{
	m_iPoint = iPoint;
	m_iTeam = iTeam;
	m_szName = CStrings::getString(szName);
}

void CBroadcastFlagDropped :: execute ( CBot *pBot )
{
	if ( pBot->getTeam() == m_iTeam )
		((CBotTF2*)pBot)->flagDropped(m_vOrigin);
	else
		((CBotTF2*)pBot)->teamFlagDropped(m_vOrigin);
}

void CBroadcastFlagCaptured :: execute ( CBot *pBot )
{
	if ( pBot->getTeam() == m_iTeam )
		((CBotTF2*)pBot)->flagReset();
	else
		((CBotTF2*)pBot)->teamFlagReset();
}

void CBroadcastRoundStart :: execute ( CBot *pBot )
{
	((CBotTF2*)pBot)->roundReset(m_bFullReset);
}

CBotFortress :: CBotFortress()
{ 
	CBot(); 
	m_fSnipeAttackTime = 0;
	m_pAmmo = NULL;
	m_pHealthkit = NULL;
	m_pFlag = NULL; 
	m_pHeal = NULL; 
	m_fCallMedic = 0; 
	m_fTauntTime = 0; 
	m_fLastKnownFlagTime = 0.0f; 
	m_bHasFlag = false; 
	m_pSentryGun = NULL; 
	m_pDispenser = NULL; 
	m_pTeleExit = NULL; 
	m_pTeleEntrance = NULL; 
	m_pNearestDisp = NULL;
	m_pNearestEnemySentry = NULL;
	m_pPrevSpy = NULL;
	m_fSeeSpyTime = 0.0f;
	m_bEntranceVectorValid = false;
}

void CBotFortress :: checkDependantEntities ()
{
	CBot::checkDependantEntities();

	checkEntity(&m_pPrevSpy);
	checkEntity(&m_pFlag); 
	checkEntity(&m_pHeal); 
	checkEntity(&m_pSentryGun); 
	checkEntity(&m_pDispenser); 
	checkEntity(&m_pTeleExit); 
	checkEntity(&m_pTeleEntrance); 
	checkEntity(&m_pNearestDisp); 
	checkEntity(&m_pNearestEnemySentry); 
	checkEntity(&m_pNearestAllySentry);
	checkEntity(&m_pAmmo);
	checkEntity(&m_pHealthkit);
}

void CBotFortress :: init (bool bVarInit)
{
	CBot::init(bVarInit);

	m_bHasFlag = false;
	m_iClass = TF_CLASS_UNDEFINED; // important

}

void CBotFortress :: setup ()
{
	CBot::setup();
}

bool CBotFortress :: startGame()
{
	int team = m_pPlayerInfo->GetTeamIndex();
	
	m_iClass = (TF_Class)CClassInterface::getTF2Class(m_pEdict);

	if ( (team != TF2_TEAM_BLUE) && (team != TF2_TEAM_RED) )
	{
		selectTeam();
	}
	else if ( (m_iDesiredClass && (m_iClass != m_iDesiredClass)) || (m_iClass == TF_CLASS_UNDEFINED) )
	{
		selectClass();
	}
	else
		return true;

	return false;
}

void CBotFortress ::pickedUpFlag()
{ 
	m_bHasFlag = true; 
	// clear tasks
	m_pSchedules->freeMemory();
}

void CBotFortress :: checkHealingValid ()
{
	if ( m_pHeal )
	{
		if ( !CBotGlobals::entityIsValid(m_pHeal) || !CBotGlobals::entityIsAlive(m_pHeal) )
			m_pHeal = NULL;
		else if ( !isVisible(m_pHeal) )
			m_pHeal = NULL;
		else if ( !wantToHeal(m_pHeal) )
		{
			m_pHeal = NULL;
		}
	}
}

bool CBotFortress :: wantToHeal ( edict_t *pPlayer )
{
	IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pPlayer);
	TF_Class iclass = (TF_Class)CClassInterface::getTF2Class(pPlayer);

	if ( ( iclass == TF_CLASS_SPY ) || (iclass == TF_CLASS_SNIPER ) || (iclass == TF_CLASS_MEDIC) || (iclass == TF_CLASS_ENGINEER) || (iclass == TF_CLASS_SCOUT) )
		return ( p->GetHealth() < p->GetMaxHealth() );
	
	return true;
}


/////////////////////////////////////////////////////////////////////
//
// When a new Entity becomes visible or Invisible this is called
// 
// bVisible = true when pEntity is Visible
// bVisible = false when pEntity becomes inVisible
void CBotFortress :: setVisible ( edict_t *pEntity, bool bVisible )
{
	CBot::setVisible(pEntity,bVisible);

	// check for people to heal
	if ( m_iClass == TF_CLASS_MEDIC )
	{
		if ( bVisible )
		{
			if ( ENTINDEX(pEntity) && (ENTINDEX(pEntity) <= gpGlobals->maxClients) ) // player
			{
				if ( CBotGlobals::getTeam(pEntity) == getTeam() )
				{
					Vector vPlayer = CBotGlobals::entityOrigin(pEntity);

					if ( distanceFrom(vPlayer) < 400 )
					{
						if ( wantToHeal(pEntity) )
						{
							if ( m_pHeal )
							{
								if ( m_pHeal != pEntity )
								{
									IPlayerInfo *p1 = playerinfomanager->GetPlayerInfo(m_pHeal);
									IPlayerInfo *p2 = playerinfomanager->GetPlayerInfo(pEntity);

									if ( ((float)p2->GetHealth()/p2->GetMaxHealth()) < ((float)p1->GetHealth()/p1->GetMaxHealth()) )
									{									
										m_pHeal = pEntity;
									}
								}					
							}
							else
							{
								m_pHeal = pEntity;
							}
						}
					}
				}
			}
		}
		else if ( m_pHeal == pEntity )
			m_pHeal = NULL;
	}
	else if ( m_iClass == TF_CLASS_SPY )
	{
		// Look for nearest sentry to sap!!!
		if ( bVisible )
		{
			if ( CTeamFortress2Mod::isSentry(pEntity,CTeamFortress2Mod::getEnemyTeam(getTeam())) )
			{
				if ( !m_pNearestEnemySentry || (pEntity != m_pNearestEnemySentry) && (distanceFrom(pEntity) < distanceFrom(m_pNearestEnemySentry)) )
					m_pNearestEnemySentry = pEntity;
			}
		}
		else if ( pEntity == m_pNearestEnemySentry )
		{
			m_pNearestEnemySentry = NULL;
		}
	}

	// Check for nearest Dispenser for health/ammo & flag
	if ( bVisible )
	{
		if ( CTeamFortress2Mod::isFlag(pEntity,getTeam()) )
			m_pFlag = pEntity;
		else if ( CTeamFortress2Mod::isSentry(pEntity,getTeam()) )
		{
			if ( !m_pNearestAllySentry || ((pEntity != m_pNearestAllySentry) && (distanceFrom(pEntity) < distanceFrom(m_pNearestAllySentry))) )
				m_pNearestAllySentry = pEntity;
		}
		else if ( CTeamFortress2Mod::isDispenser(pEntity,getTeam()) )
		{
			if ( !m_pNearestDisp || ((pEntity != m_pNearestDisp) && (distanceFrom(pEntity) < distanceFrom(m_pNearestDisp))) )
				m_pNearestDisp = pEntity;
		}
		else if ( CTeamFortress2Mod::isTeleporterEntrance(pEntity,getTeam()) )
		{
			if ( !m_pTeleEntrance || ((pEntity != m_pTeleEntrance) && (distanceFrom(pEntity) < distanceFrom(m_pTeleEntrance))) )
				m_pTeleEntrance = pEntity;
		}
		else if ( !(CClassInterface::getEffects(pEntity)&EF_NODRAW) && CTeamFortress2Mod::isAmmo(pEntity) )
		{
			float fDistance;

			fDistance = distanceFrom(pEntity);

			if ( fDistance > 200 )
				return;

			if ( !m_pAmmo || ((pEntity != m_pAmmo) && (fDistance < distanceFrom(m_pAmmo))) )
				m_pAmmo = pEntity;
			
		}
		else if ( !(CClassInterface::getEffects(pEntity)&EF_NODRAW) &&CTeamFortress2Mod::isHealthKit(pEntity) )
		{
			float fDistance;

			fDistance = distanceFrom(pEntity);

			if ( fDistance > 200 )
				return;

			if ( !m_pHealthkit || ((pEntity != m_pHealthkit) && (fDistance < distanceFrom(m_pHealthkit))) )
				m_pHealthkit = pEntity;
		}
	}
	else 
	{
		if ( pEntity == m_pFlag )
			m_pFlag = NULL;
		if ( pEntity == m_pNearestDisp )
			m_pNearestDisp = NULL;
		if ( pEntity == m_pAmmo )
			m_pAmmo = NULL;
		if ( pEntity == m_pHealthkit )
			m_pHealthkit = NULL;
		if ( pEntity == m_pHeal )
			m_pHeal = NULL;
	}
	
}

bool CBotFortress :: isAlive ()
{
	return !m_pPlayerInfo->IsDead()&&!m_pPlayerInfo->IsObserver();
}

void CBotFortress :: killed ( edict_t *pVictim )
{
	return;
}

void CBotFortress :: died ( edict_t *pKiller )
{
	CBot::spawnInit();

	droppedFlag();

	if ( randomInt(0,1) )
		m_pButtons->attack();
}

void CBotFortress :: spawnInit ()
{
	CBot::spawnInit();

	m_fPickupTime = 0.0f;

	m_fSnipeAttackTime = 0.0f;
	m_fSpyCloakTime = engine->Time() + randomFloat(10.0f,15.0f);

	m_fLastSaySpy = 0.0f;
	m_fSpyDisguiseTime = 0.0f;
	m_pHeal = NULL;
	m_pNearestDisp = NULL;
	m_pNearestEnemySentry = NULL;
	m_pNearestAllySentry = NULL;
	m_bHasFlag = false; 
	m_pPrevSpy = NULL;
	m_fSeeSpyTime = 0.0f;

	m_bSentryGunVectorValid = false;
	m_bDispenserVectorValid = false;
	m_bTeleportExitVectorValid = false;
}

int CBotFortress :: engiBuildObject (int *iState, eEngiBuild iObject, float *fTime, int *iTries )
{
	wantToShoot(false);

	switch ( *iState )
	{
	case 0:
		{
			if ( hasEngineerBuilt(iObject) )
			{
				engineerBuild(iObject,ENGI_DESTROY);
			}

			*iState = 1;
		}
		break;
	case 1:
		{
			CTraceFilterWorldAndPropsOnly filter;
			QAngle eyes = CBotGlobals::playerAngles(m_pEdict);
			QAngle turn;
			Vector forward;
			Vector building;
			// find best place to turn it to
			trace_t *tr = CBotGlobals::getTraceResult();
			int iNextState = 2;

			float bestfraction = 0.0f;			

			// unselect current weapon
			selectWeapon(0);
			engineerBuild(iObject,ENGI_BUILD);

			AngleVectors(eyes,&forward);
			iNextState = 8;
			building = getEyePosition() + (forward*100);
			//////////////////////////////////////////

			// forward
			CBotGlobals::traceLine(building,building + forward*4096.0,MASK_SOLID_BRUSHONLY,&filter);

			if ( tr->fraction > bestfraction )
			{
				iNextState = 8;
				bestfraction = tr->fraction;
			}
			////////////////////////////////////////
			turn = eyes;
			turn.y = turn.y - 90.0f;
			CBotGlobals::fixFloatAngle(&turn.y);

			AngleVectors(turn,&forward);

			// left
			CBotGlobals::traceLine(building,building + forward*4096.0,MASK_SOLID_BRUSHONLY,&filter);

			if ( tr->fraction > bestfraction )
			{
				iNextState = 6;
				bestfraction = tr->fraction;
			}
			////////////////////////////////////////
			turn = eyes;
			turn.y = turn.y + 180.0f;
			CBotGlobals::fixFloatAngle(&turn.y);

			AngleVectors(turn,&forward);

			// back
			CBotGlobals::traceLine(building,building + forward*4096.0,MASK_SOLID_BRUSHONLY,&filter);

			if ( tr->fraction > bestfraction )
			{
				iNextState = 4;
				bestfraction = tr->fraction;
			}
			///////////////////////////////////
			turn = eyes;
			turn.y = turn.y + 90.0f;
			CBotGlobals::fixFloatAngle(&turn.y);

			AngleVectors(turn,&forward);

			// right
			CBotGlobals::traceLine(building,building + forward*4096.0,MASK_SOLID_BRUSHONLY,&filter);

			if ( tr->fraction > bestfraction )
			{
				iNextState = 2;
				bestfraction = tr->fraction;
			}
			////////////////////////////////////
			*iState = iNextState;

		}
	case 2:
		{
			// let go
			*iState = *iState + 1;
		}
		break;
	case 3:
		{
			tapButton(IN_ATTACK2);
			*iState = *iState + 1;
		}
		break;
	case 4:
		{
			// let go
			*iState = *iState + 1;
		}
		break;
	case 5:
		{
			tapButton(IN_ATTACK2);
			*iState = *iState + 1;
		}
		break;
	case 6:

		{
			// let go 
			*iState = *iState + 1;
		}
		break;
	case 7:
		{
			tapButton(IN_ATTACK2);
			*iState = *iState + 1;
		}
		break;
	case 8:
		{
			// let go (wait)
			*iState = *iState + 1;
		}
		break;
	case 9:
		{
			tapButton(IN_ATTACK);

			*fTime = engine->Time() + randomFloat(0.5f,1.0f);

			*iState = *iState + 1;
		}
		break;
	case 10:
		{
			// Check if sentry built OK
			// Wait for Built object message

			m_pButtons->tap(IN_ATTACK);

			if ( *fTime < engine->Time() )
			{
				if ( hasEngineerBuilt(iObject) )
				{
					*iState = *iState + 1;
					// OK, set up whacking time!
					*fTime = engine->Time() + randomFloat(5.0f,10.0f);

					if ( iObject == ENGI_SENTRY )
							m_bSentryGunVectorValid = false;
					else
					{
						if ( iObject == ENGI_DISP )
							m_bDispenserVectorValid = false;
						else if ( iObject == ENGI_EXIT )
							m_bTeleportExitVectorValid = false;

						return 1;
					}
				}
				else if ( *iTries > 3 )
				{
					if ( iObject == ENGI_SENTRY )
						m_bSentryGunVectorValid = false;
					else if ( iObject == ENGI_DISP )
						m_bDispenserVectorValid = false;
					else if ( iObject == ENGI_EXIT )
						m_bTeleportExitVectorValid = false;

					return 0;
				}
				else
				{
					*fTime = engine->Time() + randomFloat(0.5,1.0);
					*iTries = *iTries + 1;
					*iState = 1;
				}
			}
		}
		break;
	case 11:
		{
			// whack it for a while
			if ( *fTime < engine->Time() )
			{
				return 1;
			}
			else
				tapButton(IN_ATTACK);
		}
		break;
	}

	return 2;
}

void CBotFortress :: setClass ( TF_Class _class )
{
	m_iClass = _class;
}

bool CBotFortress :: thinkSpyIsEnemy ( edict_t *pEdict )
{
	return ( (m_fSeeSpyTime > engine->Time()) && (m_pPrevSpy == pEdict) );
}

bool CBotFortress :: isEnemy ( edict_t *pEdict,bool bCheckWeapons )
{
	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	return true;	
}

bool CBotFortress :: needAmmo ()
{
	return false;
}

bool CBotFortress :: needHealth ()
{
	return getHealthPercent() < 0.7;
}

bool CBotTF2 :: needAmmo()
{
	if ( getClass() == TF_CLASS_ENGINEER )
	{		
		CBotWeapon *pWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(TF2_WEAPON_WRENCH));

		if ( pWeapon )
		{
			return ( pWeapon->getAmmo(this) < 200 );
		}
	}
	else if ( getClass() == TF_CLASS_SOLDIER )
	{
		CBotWeapon *pWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(TF2_WEAPON_ROCKETLAUNCHER));

		if ( pWeapon )
		{
			return ( pWeapon->getAmmo(this) < 1 );
		}
	}
	else if ( getClass() == TF_CLASS_DEMOMAN )
	{
		CBotWeapon *pWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(TF2_WEAPON_GRENADELAUNCHER));

		if ( pWeapon )
		{
			return ( pWeapon->getAmmo(this) < 1 );
		}
	}
	else if ( getClass() == TF_CLASS_HWGUY )
	{
		CBotWeapon *pWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(TF2_WEAPON_MINIGUN));

		if ( pWeapon )
		{
			return ( pWeapon->getAmmo(this) < 1 );
		}
	}
	else if ( getClass() == TF_CLASS_PYRO )
	{
		CBotWeapon *pWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(TF2_WEAPON_FLAMETHROWER));

		if ( pWeapon )
		{
			return ( pWeapon->getAmmo(this) < 1 );
		}
	}

	return false;
}

void CBotFortress :: currentlyDead ()
{
	m_fUpdateClass = engine->Time() + 0.1f;
}

void CBotFortress :: modThink ()
{
	// get class
	m_iClass = (TF_Class)CClassInterface::getTF2Class(m_pEdict);
	//updateClass();

	if ( needHealth() )
		updateCondition(CONDITION_NEED_HEALTH);
	else
		removeCondition(CONDITION_NEED_HEALTH);

	if ( needAmmo() )
		updateCondition(CONDITION_NEED_AMMO);
	else
		removeCondition(CONDITION_NEED_AMMO);

	if ( m_fCallMedic < engine->Time() )
	{
		if ( ((float)m_pPlayerInfo->GetHealth() / m_pPlayerInfo->GetMaxHealth()) < 0.5 )
		{
			m_fCallMedic = engine->Time() + randomFloat(10.0f,30.0f);

			callMedic();
		}
	}

	if ( (m_fUseTeleporterTime < engine->Time() ) && !hasFlag() && m_pTeleEntrance )
	{
		if ( isTeleporterUseful(m_pTeleEntrance) )
		{
			if ( !m_pSchedules->isCurrentSchedule(SCHED_USE_TELE) )
			{
				m_pSchedules->freeMemory();
				//m_pSchedules->removeSchedule(SCHED_USE_TELE);
				m_pSchedules->addFront(new CBotUseTeleSched(m_pTeleEntrance));

				m_fUseTeleporterTime = engine->Time() + randomFloat(25.0f,35.0f);
				return;
			}
		}
	}


	// Check redundant tasks
	if ( !hasSomeConditions(CONDITION_NEED_AMMO) && m_pSchedules->isCurrentSchedule(SCHED_TF2_GET_AMMO) )
	{
		m_pSchedules->removeSchedule(SCHED_TF2_GET_AMMO);
	}

	if ( !hasSomeConditions(CONDITION_NEED_HEALTH) && m_pSchedules->isCurrentSchedule(SCHED_TF2_GET_HEALTH) )
	{
		m_pSchedules->removeSchedule(SCHED_TF2_GET_HEALTH);
	}

	checkHealingValid();

	if ( m_bInitAlive )
	{
		CWaypoint *pWpt = CWaypoints::getWaypoint(CWaypoints::nearestWaypointGoal(CWaypointTypes::W_FL_TELE_ENTRANCE,getOrigin(),4096,getTeam()));

		if ( pWpt )
		{
			m_vTeleportEntrance = pWpt->getOrigin();// + Vector(randomFloat(-pWpt->getRadius(),pWpt->getRadius()),randomFloat(-pWpt->getRadius(),pWpt->getRadius()),0);
			m_bEntranceVectorValid = true;
		}
	}

	if ( m_fLastSeeEnemy && ((m_fLastSeeEnemy + 5.0)<engine->Time()) )
	{
		m_fLastSeeEnemy = 0;
		m_pButtons->tap(IN_RELOAD);
	}
}


bool CBotFortress :: isTeleporterUseful ( edict_t *pTele )
{
	edict_t *pExit = CTeamFortress2Mod::getTeleporterExit(pTele);

	if ( pExit )
	{
		if ( ((m_vGoal - CBotGlobals::entityOrigin(pExit)).Length()+distanceFrom(pTele)) < distanceFrom(m_vGoal) )
			return true;
	}

	return false;
}

void CBotFortress :: selectTeam ()
{
	char buffer[32];

	int team = randomInt(1,2);

	sprintf(buffer,"jointeam %d",team);

	helpers->ClientCommand(m_pEdict,buffer);
}

void CBotFortress :: selectClass ()
{
	char buffer[32];
	TF_Class _class;

	if ( m_iDesiredClass == 0 )
		_class = (TF_Class)randomInt(1,9);
	else
		_class = (TF_Class)m_iDesiredClass;

	m_iClass = _class;

	if ( _class == TF_CLASS_SCOUT )
	{
		sprintf(buffer,"joinclass scout");
	}
	else if ( _class == TF_CLASS_ENGINEER )
	{
		sprintf(buffer,"joinclass engineer");
	}
	else if ( _class == TF_CLASS_DEMOMAN )
	{
		sprintf(buffer,"joinclass demoman");
	}
	else if ( _class == TF_CLASS_SOLDIER )
	{
		sprintf(buffer,"joinclass soldier");
	}
	else if ( _class == TF_CLASS_HWGUY )
	{
		sprintf(buffer,"joinclass heavyweapons");
	}
	else if ( _class == TF_CLASS_MEDIC )
	{
		sprintf(buffer,"joinclass medic");
	}
	else if ( _class == TF_CLASS_SPY )
	{
		sprintf(buffer,"joinclass spy");
	}
	else if ( _class == TF_CLASS_PYRO )
	{
		sprintf(buffer,"joinclass pyro");
	}
	else
	{
		sprintf(buffer,"joinclass sniper");
	}
	helpers->ClientCommand(m_pEdict,buffer);
}

void CBotFortress :: foundSpy (edict_t *pEdict) 
{
	m_pPrevSpy = pEdict;
	m_fSeeSpyTime = engine->Time() + randomFloat(10.0f,30.0f);
};

// got shot by someone
void CBotFortress :: hurt ( edict_t *pAttacker, int iHealthNow )
{
	CBot::hurt(pAttacker,iHealthNow);

	if ( pAttacker )
	{
		m_fFrenzyTime = engine->Time() + randomFloat(5.0f,15.0f);
		return;
	}
}


/////////////////////////////////////////////////////////////////////////////
// TEAM FORTRESS 2

void CBotTF2 :: spawnInit()
{
	CBotFortress::spawnInit();

	// stickies destroyed now
	m_bDeployedStickies = false;

	m_fBlockPushTime = 0.0f;

	// update current areas
	CPoints::getAreas(getTeam(),&m_iCurrentDefendArea,&m_iCurrentAttackArea);

	m_fDoubleJumpTime = 0.0f;
	m_fFrenzyTime = 0.0f;
	m_fUseTeleporterTime = 0.0f;
	m_fSpySapTime = 0.0f;

	m_pPushPayloadBomb = NULL;
	m_pDefendPayloadBomb = NULL;

	m_bFixWeapons = true;
	m_iPrevWeaponSelectFailed = 0;

	
}

void CBotTF2 :: fixWeapons ()
{
	if ( m_pWeapons && (m_iClass != TF_CLASS_UNDEFINED) )
	{
		m_pWeapons->clearWeapons();
		
		switch ( m_iClass )
		{
		case TF_CLASS_SCOUT:

			m_pWeapons->addWeapon(TF2_WEAPON_BAT);
			m_pWeapons->addWeapon(TF2_WEAPON_PISTOL_SCOUT);
			m_pWeapons->addWeapon(TF2_WEAPON_SCATTERGUN);
		break;

		case TF_CLASS_ENGINEER:

			m_pWeapons->addWeapon(TF2_WEAPON_SHOTGUN_PRIMARY);
			m_pWeapons->addWeapon(TF2_WEAPON_PISTOL);
			m_pWeapons->addWeapon(TF2_WEAPON_WRENCH);

		break;

		case TF_CLASS_HWGUY:

			m_pWeapons->addWeapon(TF2_WEAPON_SHOTGUN_HWG);
			m_pWeapons->addWeapon(TF2_WEAPON_MINIGUN);
			m_pWeapons->addWeapon(TF2_WEAPON_FISTS);

		break;

		case TF_CLASS_SPY:

			m_pWeapons->addWeapon(TF2_WEAPON_REVOLVER);
			m_pWeapons->addWeapon(TF2_WEAPON_KNIFE);
			m_pWeapons->addWeapon(TF2_WEAPON_BUILDER);

		break;

		case TF_CLASS_PYRO:

			m_pWeapons->addWeapon(TF2_WEAPON_FIREAXE);
			m_pWeapons->addWeapon(TF2_WEAPON_FLAMETHROWER);
			m_pWeapons->addWeapon(TF2_WEAPON_SHOTGUN_PYRO);

		break;

		case TF_CLASS_SOLDIER:

			m_pWeapons->addWeapon(TF2_WEAPON_ROCKETLAUNCHER);
			m_pWeapons->addWeapon(TF2_WEAPON_SHOTGUN_SOLDIER);
			m_pWeapons->addWeapon(TF2_WEAPON_SHOVEL);

		break;

		case TF_CLASS_SNIPER:

			m_pWeapons->addWeapon(TF2_WEAPON_SNIPERRIFLE);
			m_pWeapons->addWeapon(TF2_WEAPON_SMG);
			m_pWeapons->addWeapon(TF2_WEAPON_CLUB);

		break;

		case TF_CLASS_MEDIC:

			m_pWeapons->addWeapon(TF2_WEAPON_SYRINGEGUN);
			m_pWeapons->addWeapon(TF2_WEAPON_BONESAW);
			m_pWeapons->addWeapon(TF2_WEAPON_MEDIGUN);

		break;

		case TF_CLASS_DEMOMAN:

			m_pWeapons->addWeapon(TF2_WEAPON_PIPEBOMBS);
			m_pWeapons->addWeapon(TF2_WEAPON_GRENADELAUNCHER);
			m_pWeapons->addWeapon(TF2_WEAPON_BOTTLE);

		break;
		}

	}
}

void CBotTF2 :: setClass ( TF_Class _class )
{
	m_iClass = _class;
}

void CBotTF2 :: taunt ()
{
	if ( !m_pEnemy && (m_fTauntTime < engine->Time()) )
	{
		helpers->ClientCommand(m_pEdict,"taunt");
		m_fTauntTime = engine->Time() + randomFloat(40.0,100.0);
		m_fTaunting = engine->Time() + 5.0;
	}
}

void CBotTF2 :: engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd )
{
	char buffer[16];
	char cmd[256];

	if ( iEngiCmd == ENGI_BUILD )
		strcpy(buffer,"build");
	else
	{
		strcpy(buffer,"destroy");

		switch ( iBuilding )
		{
		case ENGI_DISP :
			m_pDispenser = NULL;
			break;
		case ENGI_SENTRY :
			m_pSentryGun = NULL;
			break;
		case ENGI_ENTRANCE :
			m_pTeleEntrance = NULL;
			break;
		case ENGI_EXIT :
			m_pTeleExit = NULL;
			break;
		default:
			return;
			break;
		}
	}

	sprintf(cmd,"%s %d",buffer,iBuilding);

	helpers->ClientCommand(m_pEdict,cmd);
}

void CBotTF2 :: checkBuildingsValid ()
{
	if ( m_pSentryGun )
	{
		if ( !CBotGlobals::entityIsValid(m_pSentryGun) || !CBotGlobals::entityIsAlive(m_pSentryGun) || !CTeamFortress2Mod::isSentry(m_pSentryGun,getTeam()) )
			m_pSentryGun = NULL;
	}

	if ( m_pDispenser )
	{
		if ( !CBotGlobals::entityIsValid(m_pDispenser) || !CBotGlobals::entityIsAlive(m_pDispenser) || !CTeamFortress2Mod::isDispenser(m_pDispenser,getTeam()) )
			m_pDispenser = NULL;
	}

	if ( m_pTeleEntrance )
	{
		if ( !CBotGlobals::entityIsValid(m_pTeleEntrance) || !CBotGlobals::entityIsAlive(m_pTeleEntrance) || !CTeamFortress2Mod::isTeleporterEntrance(m_pTeleEntrance,getTeam()) )
			m_pTeleEntrance = NULL;
	}

	if ( m_pTeleExit )
	{
		if ( !CBotGlobals::entityIsValid(m_pTeleExit) || !CBotGlobals::entityIsAlive(m_pTeleExit) || !CTeamFortress2Mod::isTeleporterExit(m_pTeleExit,getTeam()) )
			m_pTeleExit = NULL;
	}
}

// Find the EDICT_T of the building that the engineer just built...
edict_t *CBotTF2 :: findEngineerBuiltObject ( eEngiBuild iBuilding )
{
	int i;

	QAngle eye = CBotGlobals::playerAngles(m_pEdict);
	Vector vForward;
	Vector vOrigin;
	edict_t *pEntity;
	edict_t *pBest = NULL;
	float fDistance;
	float fMinDistance = 100;
	bool bValid;
	int team;

	team = getTeam();

	AngleVectors(eye,&vForward);

	vOrigin = getOrigin() + (vForward*100);

	for ( i = gpGlobals->maxClients+1; i < gpGlobals->maxEntities; i ++ )
	{
		pEntity = INDEXENT(i);

		if ( CBotGlobals::entityIsValid(pEntity) )
		{
			bValid = false;
			fDistance = (CBotGlobals::entityOrigin(pEntity) - vOrigin).Length();

			if ( fDistance < fMinDistance )
			{
				switch ( iBuilding )
				{
				case ENGI_DISP :
					bValid = CTeamFortress2Mod::isDispenser(pEntity,team);
					break;
				case ENGI_ENTRANCE :
					bValid = CTeamFortress2Mod::isTeleporterEntrance(pEntity,team);
					break;
				case ENGI_EXIT:
					bValid = CTeamFortress2Mod::isTeleporterExit(pEntity,team);
					break;
				case ENGI_SENTRY :
					bValid = CTeamFortress2Mod::isSentry(pEntity,team);
					break;
				}

				if ( bValid )
				{
					fMinDistance = fDistance;
					pBest = pEntity;
				}
			}
		}
	}

	if ( pBest )
	{
			switch ( iBuilding )
			{
			case ENGI_DISP :
				m_pDispenser = pBest;
				break;
			case ENGI_ENTRANCE :
				m_pTeleEntrance = pBest;
				//m_vTeleportEntrance = CBotGlobals::entityOrigin(pBest);
				//m_bEntranceVectorValid = true;
				break;
			case ENGI_EXIT:
				m_pTeleExit = pBest;
				break;
			case ENGI_SENTRY :
				m_pSentryGun = pBest;
				break;
			default:
				return NULL;
			}
	}

	return pBest;
}

void CBotTF2 :: died ( edict_t *pKiller )
{
	spawnInit();

	if ( pKiller && CBotGlobals::entityIsValid(pKiller) )
		m_pNavigator->belief(getOrigin(),getOrigin(),bot_beliefmulti.GetFloat(),distanceFrom(pKiller),BELIEF_DANGER);
}

void CBotTF2 :: killed ( edict_t *pVictim )
{
	if ( pVictim && CBotGlobals::entityIsValid(pVictim) )
		m_pNavigator->belief(getOrigin(),getOrigin(),bot_beliefmulti.GetFloat(),distanceFrom(pVictim),BELIEF_SAFETY);

	taunt();
}

void CBotTF2 :: capturedFlag ()
{
	taunt();
}

void CBotTF2 :: spyDisguise ( int iTeam, int iClass )
{
	//char cmd[256];

	if ( iTeam == 3 )
		m_iImpulse = 230 + iClass;
	else if ( iTeam == 2 )
		m_iImpulse = 220 + iClass;

	//sprintf(cmd,"disguise %d %d",iClass,iTeam);

	//helpers->ClientCommand(m_pEdict,cmd);
}
// Test
bool CBotTF2 :: isCloaked ()
{
	return m_pController->IsEFlagSet(EF_NODRAW);
}
// Test
bool CBotTF2 :: isDisguised ()
{
	int _class,_team,_index,_health;

	if ( CClassInterface::getTF2SpyDisguised (m_pEdict,&_class,&_team,&_index,&_health) )

	{
		if ( _class > 0 )
			return true;
	}

	return false;
	
}

void CBotTF2 :: updateClass ()
{
	if ( m_fUpdateClass && (m_fUpdateClass < engine->Time()) )
	{
		/*const char *model = m_pPlayerInfo->GetModelName();

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
			*/

		m_fUpdateClass = 0;
	}
}

TF_Class CBotTF2 :: getClass ()
{
	return m_iClass;
}

void CBotTF2 :: setup ()
{
	CBotFortress::setup();
}


void CBotTF2 :: engiBuildSuccess ( eEngiBuild iBuilding )
{
	findEngineerBuiltObject(iBuilding);
}

bool CBotTF2 :: hasEngineerBuilt ( eEngiBuild iBuilding )
{
	switch ( iBuilding )
	{
	case ENGI_SENTRY:
		return m_pSentryGun!=NULL; // TODO
		break;
	case ENGI_DISP:
		return m_pDispenser!=NULL; // TODO
		break;
	case ENGI_ENTRANCE:
		return m_pTeleEntrance!=NULL; // TODO
		break;
	case ENGI_EXIT:
		return m_pTeleExit!=NULL; // TODO
		break;
	}	

	return false;
}

void CBotFortress :: flagDropped ( Vector vOrigin )
{ 
	m_vLastKnownFlagPoint = vOrigin; 
	m_fLastKnownFlagTime = engine->Time() + 60.0f;

	if ( m_pSchedules->hasSchedule(SCHED_TF2_GET_FLAG) )
		m_pSchedules->removeSchedule(SCHED_TF2_GET_FLAG);

}

void CBotFortress :: teamFlagDropped ( Vector vOrigin )
{
	m_vLastKnownTeamFlagPoint = vOrigin; 
	m_fLastKnownTeamFlagTime = engine->Time() + 60.0f;
}

void CBotFortress :: callMedic ()
{
	helpers->ClientCommand (m_pEdict,"saveme");
}

bool CBotFortress :: canGotoWaypoint (Vector vPrevWaypoint, CWaypoint *pWaypoint)
{
	if ( CBot::canGotoWaypoint(vPrevWaypoint,pWaypoint) )
	{
		if ( pWaypoint->hasFlag(CWaypointTypes::W_FL_AREAONLY) )
		{
			if ( !CPoints::isValidArea(pWaypoint->getArea()) )
				return false;
		}
		if ( pWaypoint->hasFlag(CWaypointTypes::W_FL_ROCKET_JUMP) )
		{
			return ( (getClass() == TF_CLASS_SOLDIER) && (getHealthPercent() > 0.6) );
		}
		else if ( pWaypoint->hasFlag(CWaypointTypes::W_FL_DOUBLEJUMP) )
		{
			return ( getClass() == TF_CLASS_SCOUT);
		}

		return true;
	}

	return false;
}

void CBotTF2 :: callMedic ()
{
	helpers->ClientCommand (m_pEdict,"voicemenu 0 0");
}

void CBotTF2 :: modThink ()
{
	bool bNeedHealth = hasSomeConditions(CONDITION_NEED_HEALTH);
	bool bNeedAmmo = hasSomeConditions(CONDITION_NEED_AMMO);
	// mod specific think code here
	CBotFortress :: modThink();

	if ( m_bFixWeapons || (m_iPrevWeaponSelectFailed>2) )
	{
		fixWeapons();
		m_bFixWeapons = false;
		m_iPrevWeaponSelectFailed = 0;
	}

	if ( getClass() == TF_CLASS_SNIPER )
	{
		if ( CTeamFortress2Mod::TF2_IsPlayerZoomed(m_pEdict) )
			m_fFov = 20.0f; // Jagger
		else
			m_fFov = BOT_DEFAULT_FOV;
	}

	if ( getClass() == TF_CLASS_HWGUY )
	{
		if ( m_pButtons->holdingButton(IN_ATTACK) || m_pButtons->holdingButton(IN_ATTACK2) )
		{
			if ( m_pButtons->holdingButton(IN_JUMP) )
				m_pButtons->letGo(IN_JUMP);
		}
	}

	if ( getClass() == TF_CLASS_ENGINEER )
		checkBuildingsValid();

	if ( m_fTaunting > engine->Time() )
	{
		m_pButtons->letGoAllButtons(true);
		stopMoving(10);
	}

	if ( m_fDoubleJumpTime && (m_fDoubleJumpTime < engine->Time()) )
	{
		tapButton(IN_JUMP);
		m_fDoubleJumpTime = 0;
	}

	if ( m_iClass == TF_CLASS_SPY )
	{
		if ( !hasFlag() )
		{
			if ( m_fSpyDisguiseTime < engine->Time() )
			{
				if ( !isDisguised() )
				{
					int iteam = CTeamFortress2Mod::getEnemyTeam(getTeam());

					spyDisguise(iteam,getSpyDisguiseClass(iteam));
				}

				m_fSpyDisguiseTime = engine->Time() + 5.0f;
			}

			if ( !m_pEnemy && !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && !CTeamFortress2Mod::TF2_IsPlayerCloaked(m_pEdict)  )
			{
				if (m_fSpyCloakTime < engine->Time())
				{
					m_fSpyCloakTime = engine->Time() + randomFloat(40.0f,60.0f);

					secondaryAttack();
				}
			}
			// uncloak
			else if ( CTeamFortress2Mod::TF2_IsPlayerCloaked(m_pEdict) && m_pEnemy && hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
			{
				secondaryAttack();
			}

			/*if ( m_pNearestEnemySentry && ( m_fSpySapTime < engine->Time() ))
			{
				m_fSpySapTime = engine->Time() + randomFloat(2.0f,7.0f);

				if ( !m_pSchedules->isCurrentSchedule(SCHED_SPY_SAP_BUILDING) )
				{
					m_pSchedules->removeSchedule(SCHED_SPY_SAP_BUILDING);
					m_pSchedules->addFront(new CBotSpySapBuildingSched(m_pNearestEnemySentry));
				}				
			}*/
		}
	}

	if ( m_pSchedules->isCurrentSchedule(SCHED_GOTO_ORIGIN) && (m_fPickupTime < engine->Time()) && (bNeedHealth || bNeedAmmo) && (!m_pEnemy && !hasSomeConditions(CONDITION_SEE_CUR_ENEMY)) )
	{
		if ( (m_fPickupTime<engine->Time()) && m_pNearestDisp && !m_pSchedules->isCurrentSchedule(SCHED_USE_DISPENSER) )
		{
			if ( fabs(CBotGlobals::entityOrigin(m_pNearestDisp).z - getOrigin().z) < BOT_JUMP_HEIGHT )
			{
				m_pSchedules->removeSchedule(SCHED_USE_DISPENSER);
				m_pSchedules->addFront(new CBotUseDispSched(m_pNearestDisp));

				m_fPickupTime = engine->Time() + randomFloat(6.0f,20.0f);
				return;
			}
			
		}
		else if ( (m_fPickupTime<engine->Time()) && bNeedHealth && m_pHealthkit && !m_pSchedules->isCurrentSchedule(SCHED_TF2_GET_HEALTH) )
		{
			if ( fabs(CBotGlobals::entityOrigin(m_pHealthkit).z - getOrigin().z) < BOT_JUMP_HEIGHT )
			{
				m_pSchedules->removeSchedule(SCHED_TF2_GET_HEALTH);
				m_pSchedules->addFront(new CBotTF2GetHealthSched(CBotGlobals::entityOrigin(m_pHealthkit)));

				m_fPickupTime = engine->Time() + randomFloat(5.0f,10.0f);

				return;
			}
			
		}
		else if ( (m_fPickupTime<engine->Time()) && bNeedAmmo && m_pAmmo && !m_pSchedules->isCurrentSchedule(SCHED_PICKUP) )
		{
			if ( fabs(CBotGlobals::entityOrigin(m_pAmmo).z - getOrigin().z) < BOT_JUMP_HEIGHT )
			{
				m_pSchedules->removeSchedule(SCHED_TF2_GET_AMMO);
				m_pSchedules->addFront(new CBotTF2GetAmmoSched(CBotGlobals::entityOrigin(m_pAmmo)));

				m_fPickupTime = engine->Time() + randomFloat(5.0f,10.0f);

				return;
			}
		}
	}

	m_fIdealMoveSpeed = CTeamFortress2Mod::TF2_GetPlayerSpeed(m_pEdict,m_iClass);
}

bool CBotFortress :: canAvoid ( edict_t *pEntity )
{
	float distance;
	Vector vAvoidOrigin;
	int index;

	if ( !CBotGlobals::entityIsValid(pEntity) )
		return false;
	if ( m_pEdict == pEntity ) // can't avoid self!!!
		return false;
	if ( m_pLookEdict == pEntity )
		return false;
	if ( m_pLastEnemy == pEntity )
		return false;
	if ( pEntity == m_pTeleEntrance )
		return false;
	if ( pEntity == m_pNearestDisp )
		return false;
	if ( pEntity == m_pHealthkit )
		return false;
	if ( pEntity == m_pAmmo )
		return false;
	if ( pEntity == m_pSentryGun )
		return false;
	if ( pEntity == m_pDispenser )
		return false;

	index = ENTINDEX(pEntity);

	if ( !index )
		return false;

	vAvoidOrigin = CBotGlobals::entityOrigin(pEntity);

	distance = distanceFrom(vAvoidOrigin);

	if ( ( distance > 1 ) && ( distance < 200 ) && (vAvoidOrigin.z >= getOrigin().z) && (fabs(getOrigin().z - vAvoidOrigin.z) < 64) )
	{
		if ( index <= gpGlobals->maxClients )
			return (m_pEnemy!=pEntity) && isEnemy(pEntity,false);
	}

	return false;
}

bool CBotFortress :: isClassOnTeam ( int iClass, int iTeam )
{
	int i = 0;
	edict_t *pPlayer;

	for ( i = 1; i <= gpGlobals->maxClients; i ++ )
	{
		pPlayer = INDEXENT(i);

		if ( CBotGlobals::entityIsValid(pPlayer) && (CTeamFortress2Mod::getTeam(pPlayer) == iTeam))
		{
			if ( CClassInterface::getTF2Class(pPlayer) == iClass )
				return true;
		}
	}

	return false;
}

bool CBotFortress :: wantToFollowEnemy ()
{
    if ( !m_pLastEnemy )
        return false;
    if ( hasFlag() )
        return false;
    if ( getClass() == TF_CLASS_SCOUT )
        return false;
    if ( (m_iClass == TF_CLASS_SPY) && isDisguised() ) // sneak around the enemy
        return true;

    if ( ((ENTINDEX(m_pLastEnemy) > 0)&&(ENTINDEX(m_pLastEnemy)<=gpGlobals->maxClients)) && (CClassInterface::getTF2Class(m_pLastEnemy) == TF_CLASS_SPY) && (thinkSpyIsEnemy(m_pLastEnemy)) )
        return true; // always find spies!
    
    return CBot::wantToFollowEnemy();
}

void CBotTF2 :: foundSpy (edict_t *pEdict)
{
	CBotFortress::foundSpy(pEdict);

	if ( m_fLastSaySpy < engine->Time() )
	{
		helpers->ClientCommand(m_pEdict,"voicemenu 1 1");
		m_fLastSaySpy = engine->Time() + randomFloat(10.0f,40.0f);
	}
}

int CBotFortress :: getSpyDisguiseClass ( int iTeam )
{
	int i = 0;
	edict_t *pPlayer;
	dataUnconstArray<int> m_classes;
	int _class;

	for ( i = 1; i <= gpGlobals->maxClients; i ++ )
	{
		pPlayer = INDEXENT(i);

		if ( CBotGlobals::entityIsValid(pPlayer) && (CTeamFortress2Mod::getTeam(pPlayer) == iTeam))
		{
			_class = CClassInterface::getTF2Class(pPlayer);

			if ( _class )
				m_classes.Add(_class);
		}
	}


	if ( m_classes.IsEmpty() )
		return randomInt(1,9);
	
	return m_classes.Random();
}


void CBotTF2 :: setVisible ( edict_t *pEntity, bool bVisible )
{
	CBotFortress::setVisible(pEntity,bVisible);

	if ( bVisible )
	{
		if ( CTeamFortress2Mod::isPayloadBomb(pEntity,TF2_TEAM_RED) )
		{
			m_pRedPayloadBomb = pEntity;
		}
		else if ( CTeamFortress2Mod::isPayloadBomb(pEntity,TF2_TEAM_BLUE) )
		{
			m_pBluePayloadBomb = pEntity;
		}
	}
}

// Preconditions :  Current weapon is Medigun
//					pPlayer is not NULL
//
bool CBotTF2 :: healPlayer ( edict_t *pPlayer, edict_t *pPrevPlayer )
{
	CBotWeapon *pWeap = getCurrentWeapon();
	IPlayerInfo *p;
	edict_t *pWeapon;

	Vector vOrigin = CBotGlobals::entityOrigin(m_pHeal);

	if ( !m_pHeal )
		return false;
	
	if ( !wantToHeal(m_pHeal) )
		return false;

	if ( !isVisible(m_pHeal) ) 
		return false;

	if ( distanceFrom(vOrigin) > 90 )
	{
		setMoveTo(vOrigin,3);
	}
	else
	{
		stopMoving();
	}

	lookAtEdict(m_pHeal);
	setLookAtTask(LOOK_EDICT,7);
	// unselect weapon
	//pBot->selectWeapon(0);

	pWeapon = INDEXENT(pWeap->getWeaponIndex());
	if ( pWeapon == NULL )
		return false;

	if ( !CClassInterface::getMedigunHealing(pWeapon) )
	{
		primaryAttack(true);
	}
	else
	{
		if ( pPrevPlayer != pPlayer )
			primaryAttack(true);
	}
	//else
	//	m_pHeal = CClassInterface::getMedigunTarget(INDEXENT(pWeap->getWeaponIndex()));

	// hack
	m_bLookedForEnemyLast = false;
	m_pLastEnemy = pPlayer;

	p = playerinfomanager->GetPlayerInfo(pPlayer);

	// Simple UBER check
	if ( ((((float)m_pPlayerInfo->GetHealth())/m_pPlayerInfo->GetMaxHealth())<0.25) || (getHealthPercent()<0.25) )
	{
		if ( randomInt(0,100) > 50 )
		{
			// uber if ready
			m_pButtons->tap(IN_ATTACK2);
		}
	}

	return true;
}
// The lower the better
float CBotTF2 :: getEnemyFactor ( edict_t *pEnemy )
{
	float fPreFactor = 0;

	if ( ((ENTINDEX(pEnemy) > 0)&&(ENTINDEX(pEnemy)<=gpGlobals->maxClients)) )
	{		
		if ( CTeamFortress2Mod::isFlagCarrier(pEnemy) )
		{
			// shoot flag carrier even if 1000 units away from nearest enemy
			fPreFactor = -1000;
		}
		else
		{
			int iclass = CClassInterface::getTF2Class(pEnemy);

			if ( iclass == TF_CLASS_MEDIC )
			{
				// shoot medic even if 250 units further from nearest enemy (approx. healing range)
				fPreFactor = -250;
			}
			else if ( iclass == TF_CLASS_SPY ) 
			{
				// shoot spy even if a little further from nearest enemy
				fPreFactor = -150;
			}
		}
	}
	else
	{
		if ( CTeamFortress2Mod::isSentry(pEnemy,CTeamFortress2Mod::getEnemyTeam(getTeam())) )
		{
			fPreFactor = -400;
		}
	}

	fPreFactor += distanceFrom(pEnemy);

	return fPreFactor;
}

void CBotTF2 :: getTasks ( unsigned int iIgnore )
{
	static TF_Class iClass;
	static int iMetal = 0;
	static bool bNeedAmmo = false;
	static bool bNeedHealth = false;
	static CBotUtilities utils;
	static CBotWeapon *pWeapon;
	static CWaypoint *pWaypointResupply;
	static CWaypoint *pWaypointAmmo;
	static CWaypoint *pWaypointHealth;
	static CBotUtility *util;
	static float fResupplyDist = 1;
	static float fHealthDist = 1;
	static float fAmmoDist = 1;
	static bool bHasFlag = false;
	static float fGetFlagUtility = 0.5;
	static float fDefendFlagUtility = 0.5;

	extern ConVar bot_defrate;
	//static float fResupplyUtil = 0.5;
	//static float fHealthUtil = 0.5;
	//static float fAmmoUtil = 0.5;

	extern const char *g_szUtils[BOT_UTIL_MAX];

	// if in setup time this will tell bot not to shoot yet
	wantToShoot(CTeamFortress2Mod::hasRoundStarted());

	bHasFlag = hasFlag();

	// look for tasks / more important tasks here
	if ( (m_iClass == TF_CLASS_MEDIC) && !bHasFlag && m_pHeal && CBotGlobals::entityIsAlive(m_pHeal) )
	{		
		if ( !m_pSchedules->isCurrentSchedule(SCHED_HEAL) )
		{
			m_pSchedules->freeMemory();
			m_pSchedules->addFront(new CBotTF2HealSched(m_pHeal));
		}
	}
	else if ( !m_bLookedForEnemyLast && m_pLastEnemy && CBotGlobals::entityIsValid(m_pLastEnemy) && CBotGlobals::entityIsAlive(m_pLastEnemy) )
	{
		if ( wantToFollowEnemy() )
		{
			Vector vVelocity = Vector(0,0,0);
			CClient *pClient = CClients::get(m_pLastEnemy);
			CBotSchedule *pSchedule = new CBotSchedule();
			
			CFindPathTask *pFindPath = new CFindPathTask(m_vLastSeeEnemy);	
			
			if ( pClient )
				vVelocity = pClient->getVelocity();

			m_pSchedules->freeMemory();

			pSchedule->addTask(pFindPath);
			pSchedule->addTask(new CFindLastEnemy(m_vLastSeeEnemy,vVelocity));

			//////////////
			pFindPath->setNoInterruptions();

			m_pSchedules->add(pSchedule);

			m_bLookedForEnemyLast = true;
		}
	}

	if ( !m_pSchedules->isEmpty() )
		return; // already got some tasks left

	/*if ( ((m_iClass!=TF_CLASS_MEDIC)||(!m_pHeal)) && m_pEnemy && hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{		
		if ( randomFloat(0.0f,100.0f) > 50.0f )
		{
			m_pSchedules->addFront(new CGotoHideSpotSched(m_pEnemy));
			return;
		}
	}*/

	// Shadow/Time must be Floating point
	if(m_fBlockPushTime < engine->Time())
	{
		m_bBlockPushing = (randomFloat(0.0,100)>50); // 50 % block pushing
		m_fBlockPushTime = engine->Time() + randomFloat(10.0f,30.0f); // must be floating point
	}

	// No Enemy now
	if ( m_iClass == TF_CLASS_SNIPER )
		// un zoom
	{
		if ( CTeamFortress2Mod::TF2_IsPlayerZoomed(m_pEdict) )
				secondaryAttack();
	}

	iClass = getClass();

	bNeedAmmo = hasSomeConditions(CONDITION_NEED_AMMO);
	bNeedHealth = hasSomeConditions(CONDITION_NEED_HEALTH);

	pWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(TF2_WEAPON_WRENCH));
	iMetal = pWeapon->getAmmo(this);

	if ( bNeedAmmo || bNeedHealth )
	{
		dataUnconstArray<int> *failed;

		m_pNavigator->getFailedGoals(&failed);

		unsigned char *failedlist = CWaypointLocations :: resetFailedWaypoints ( failed );

		fResupplyDist = 1;
		fHealthDist = 1;
		fAmmoDist = 1;

		pWaypointResupply = CWaypoints::getWaypoint(CWaypoints::getClosestFlagged(CWaypointTypes::W_FL_RESUPPLY,getOrigin(),getTeam(),&fResupplyDist,failedlist));
		
		if ( bNeedAmmo )
			pWaypointAmmo = CWaypoints::getWaypoint(CWaypoints::getClosestFlagged(CWaypointTypes::W_FL_AMMO,getOrigin(),getTeam(),&fAmmoDist,failedlist));
		if ( bNeedHealth )
			pWaypointHealth = CWaypoints::getWaypoint(CWaypoints::getClosestFlagged(CWaypointTypes::W_FL_HEALTH,getOrigin(),getTeam(),&fHealthDist,failedlist));
	}

	if ( iClass == TF_CLASS_ENGINEER )
		checkBuildingsValid();

	utils.addUtility(CBotUtility(BOT_UTIL_CAPTURE_FLAG,CTeamFortress2Mod::isMapType(TF_MAP_CTF) && bHasFlag,0.95));

	if ( iClass == TF_CLASS_ENGINEER )
	{
		int iSentryLevel = 0;
		int iDispenserLevel = 0;
		int iAllySentryLevel = 0;
		int iAllyDispLevel = 0;

		float fEntranceDist = 99999.0f;
		float fExitDist = 99999.0f;		

		float fAllyDispenserHealthPercent = 1.0f;
		float fAllySentryHealthPercent = 1.0f;

		float fSentryHealthPercent = 1.0f;
		float fDispenserHealthPercent = 1.0f;
		float fTeleporterEntranceHealthPercent = 1.0f;
		float fTeleporterExitHealthPercent = 1.0f;		

		if ( m_pTeleExit )
		{
			fExitDist = distanceFrom(m_pTeleExit);

			fTeleporterExitHealthPercent = CClassInterface::getHealth(m_pTeleExit)/180;
		}

		if ( m_pTeleEntrance )
		{
			fEntranceDist = distanceFrom(m_pTeleEntrance);

			fTeleporterEntranceHealthPercent = CClassInterface::getHealth(m_pTeleEntrance)/180;
		}

		if ( m_pSentryGun )
		{
			iSentryLevel = CTeamFortress2Mod::getSentryLevel(m_pSentryGun);
			fSentryHealthPercent = CClassInterface::getHealth(m_pSentryGun);

			if ( iSentryLevel == 1 )
				fSentryHealthPercent /= TF2_SENTRY_LEVEL1_HEALTH;		
			else if ( iSentryLevel == 2 )
				fSentryHealthPercent /= TF2_SENTRY_LEVEL2_HEALTH;		
			else if ( iSentryLevel == 3 )
				fSentryHealthPercent /= TF2_SENTRY_LEVEL3_HEALTH;		
		}

		if ( m_pDispenser )
		{
			iDispenserLevel = CTeamFortress2Mod::getDispenserLevel(m_pDispenser);
			fDispenserHealthPercent = CClassInterface::getHealth(m_pDispenser);

			if ( iDispenserLevel == 1 )
				fDispenserHealthPercent /= TF2_DISPENSER_LEVEL1_HEALTH;
			else if ( iDispenserLevel == 2 )
				fDispenserHealthPercent /= TF2_DISPENSER_LEVEL2_HEALTH;		
			else if ( iDispenserLevel == 3 )
				fDispenserHealthPercent /= TF2_DISPENSER_LEVEL3_HEALTH;		
		}

		if ( m_pNearestDisp && (m_pNearestDisp != m_pDispenser) )
		{
			iAllyDispLevel = CTeamFortress2Mod::getDispenserLevel(m_pNearestDisp);
			fAllyDispenserHealthPercent = CClassInterface::getHealth(m_pNearestDisp);

			if ( iAllyDispLevel == 1 )
				fAllyDispenserHealthPercent /= TF2_DISPENSER_LEVEL1_HEALTH;
			else if ( iAllyDispLevel == 2 )
				fAllyDispenserHealthPercent /= TF2_DISPENSER_LEVEL2_HEALTH;		
			else if ( iAllyDispLevel == 3 )
				fAllyDispenserHealthPercent /= TF2_DISPENSER_LEVEL3_HEALTH;	
		}

		if ( m_pNearestAllySentry && (m_pNearestAllySentry != m_pSentryGun) )
		{
			iAllySentryLevel = CTeamFortress2Mod::getSentryLevel(m_pNearestAllySentry);
			fAllySentryHealthPercent = CClassInterface::getHealth(m_pNearestAllySentry);

			if ( iAllySentryLevel == 1 )
				fAllySentryHealthPercent /= TF2_SENTRY_LEVEL1_HEALTH;		
			else if ( iAllySentryLevel == 2 )
				fAllySentryHealthPercent /= TF2_SENTRY_LEVEL2_HEALTH;		
			else if ( iSentryLevel == 3 )
				fAllySentryHealthPercent /= TF2_SENTRY_LEVEL3_HEALTH;	
		}

		utils.addUtility(CBotUtility(BOT_UTIL_BUILDSENTRY,!bHasFlag && !m_pSentryGun && (iMetal>=130),0.9));
		utils.addUtility(CBotUtility(BOT_UTIL_BUILDDISP,!bHasFlag&& m_pSentryGun && !m_pDispenser && (iMetal>=100),0.8 + (((float)(int)bNeedAmmo)*0.12) + (((float)(int)bNeedHealth)*0.12)));
		utils.addUtility(CBotUtility(BOT_UTIL_BUILDTELENT,!bHasFlag&&m_bEntranceVectorValid&&!m_pTeleEntrance&&(iMetal>=125),1.0));
		utils.addUtility(CBotUtility(BOT_UTIL_BUILDTELEXT,!bHasFlag&&!m_pTeleExit&&(iMetal>=125),randomFloat(0.7,0.9)));
		utils.addUtility(CBotUtility(BOT_UTIL_UPGSENTRY,!bHasFlag && m_pSentryGun && (iMetal>=200) && ((iSentryLevel<3)||(fSentryHealthPercent<1.0f)),0.8+((1.0f-fSentryHealthPercent)*0.2)));
		utils.addUtility(CBotUtility(BOT_UTIL_GETAMMODISP,m_pDispenser && isVisible(m_pDispenser) && (iMetal<200),1.0));
		utils.addUtility(CBotUtility(BOT_UTIL_UPGTELENT,m_pTeleEntrance!=NULL && (iMetal>=200) &&  (fTeleporterEntranceHealthPercent<1.0f),((fEntranceDist<fExitDist)) * 0.51 + (0.5-(fTeleporterEntranceHealthPercent*0.5))));
		utils.addUtility(CBotUtility(BOT_UTIL_UPGTELEXT,m_pTeleExit!=NULL && (iMetal>=200) &&  (fTeleporterExitHealthPercent<1.0f),((fExitDist<fEntranceDist) * 0.51) + ((0.5-fTeleporterExitHealthPercent)*0.5)));
		utils.addUtility(CBotUtility(BOT_UTIL_UPGDISP,m_pDispenser!=NULL && (iMetal>=200) && ((iDispenserLevel<3)||(fDispenserHealthPercent<1.0f)),0.7+((1.0f-fDispenserHealthPercent)*0.3)));

		utils.addUtility(CBotUtility(BOT_UTIL_GOTORESUPPLY_FOR_AMMO, !bHasFlag && pWaypointResupply && bNeedAmmo && !m_pAmmo,(fAmmoDist/fResupplyDist)*(200.0f/(iMetal+1))));
		utils.addUtility(CBotUtility(BOT_UTIL_FIND_NEAREST_AMMO,!bHasFlag&&bNeedAmmo&&!m_pAmmo&&pWaypointAmmo,(fResupplyDist/fAmmoDist)*(100.0f/(iMetal+1))));

		utils.addUtility(CBotUtility(BOT_UTIL_ENGI_LOOK_AFTER_SENTRY,m_pSentryGun!=NULL,0.5));

		utils.addUtility(CBotUtility(BOT_UTIL_UPGTMSENTRY,!bHasFlag && m_pNearestAllySentry && (m_pNearestAllySentry!=m_pSentryGun) && (iMetal>=200) && ((iAllySentryLevel<3)||(fAllySentryHealthPercent<1.0f)),0.8+((1.0f-fAllySentryHealthPercent)*0.2)));
		utils.addUtility(CBotUtility(BOT_UTIL_UPGTMDISP,(m_pNearestDisp!=NULL)&&(m_pNearestDisp!=m_pDispenser) && (iMetal>=200) && ((iAllyDispLevel<3)||(fAllyDispenserHealthPercent<1.0f)),0.7+((1.0f-fAllyDispenserHealthPercent)*0.3)));

	}
	else
	{
		utils.addUtility(CBotUtility(BOT_UTIL_GOTORESUPPLY_FOR_AMMO, !bHasFlag && pWaypointResupply && bNeedAmmo && !m_pAmmo,fAmmoDist/fResupplyDist));
		utils.addUtility(CBotUtility(BOT_UTIL_FIND_NEAREST_AMMO,!bHasFlag&&bNeedAmmo&&!m_pAmmo&&pWaypointAmmo,fResupplyDist/fAmmoDist));
	}

	fGetFlagUtility = 0.2+randomFloat(0.0f,0.2f);

	if ( m_iClass == TF_CLASS_SCOUT )
		fGetFlagUtility = 0.6;
	else if ( m_iClass == TF_CLASS_SPY )
		fGetFlagUtility = 0.6;

	fDefendFlagUtility = bot_defrate.GetFloat()/2;

	if ( (m_iClass == TF_CLASS_HWGUY) || (m_iClass == TF_CLASS_DEMOMAN) || (m_iClass == TF_CLASS_SOLDIER) || (m_iClass == TF_CLASS_PYRO) )
		fDefendFlagUtility = bot_defrate.GetFloat() - randomFloat(0.0f,fDefendFlagUtility);

	utils.addUtility(CBotUtility(BOT_UTIL_GOTODISP,m_pNearestDisp && (bNeedAmmo || bNeedHealth),1.0));
	utils.addUtility(CBotUtility(BOT_UTIL_GOTORESUPPLY_FOR_HEALTH, !bHasFlag && pWaypointResupply && bNeedHealth && !m_pHealthkit,fHealthDist/fResupplyDist));

	utils.addUtility(CBotUtility(BOT_UTIL_GETAMMOKIT, bNeedAmmo && m_pAmmo,1.0));
	utils.addUtility(CBotUtility(BOT_UTIL_GETHEALTHKIT, bNeedHealth && m_pHealthkit,1.0));

	utils.addUtility(CBotUtility(BOT_UTIL_GETFLAG, CTeamFortress2Mod::isMapType(TF_MAP_CTF) && !bHasFlag && CTeamFortress2Mod::isMapType(TF_MAP_CTF),fGetFlagUtility));
	utils.addUtility(CBotUtility(BOT_UTIL_GETFLAG_LASTKNOWN, CTeamFortress2Mod::isMapType(TF_MAP_CTF) && !bHasFlag && (m_fLastKnownFlagTime && (m_fLastKnownFlagTime > engine->Time())), fGetFlagUtility+0.1));

	utils.addUtility(CBotUtility(BOT_UTIL_DEFEND_FLAG, CTeamFortress2Mod::isMapType(TF_MAP_CTF) && !bHasFlag, fDefendFlagUtility+0.1));
	utils.addUtility(CBotUtility(BOT_UTIL_DEFEND_FLAG_LASTKNOWN, CTeamFortress2Mod::isMapType(TF_MAP_CTF) && !bHasFlag && (m_fLastKnownTeamFlagTime && (m_fLastKnownTeamFlagTime > engine->Time())), fDefendFlagUtility+(randomFloat(0.0,0.2)-0.1)));
	utils.addUtility(CBotUtility(BOT_UTIL_SNIPE, !bHasFlag && (iClass==TF_CLASS_SNIPER), 0.95));	

	utils.addUtility(CBotUtility(BOT_UTIL_ROAM,true,0.1));
	utils.addUtility(CBotUtility(BOT_UTIL_FIND_NEAREST_HEALTH,!bHasFlag&&bNeedHealth&&!m_pHealthkit&&pWaypointHealth,fResupplyDist/fHealthDist));
	
	// only attack if attack area is > 0
	utils.addUtility(CBotUtility(BOT_UTIL_ATTACK_POINT,(m_iCurrentAttackArea>0) && (CTeamFortress2Mod::isMapType(TF_MAP_CART)||CTeamFortress2Mod::isMapType(TF_MAP_CARTRACE)||CTeamFortress2Mod::isMapType(TF_MAP_ARENA)||CTeamFortress2Mod::isMapType(TF_MAP_KOTH)||CTeamFortress2Mod::isMapType(TF_MAP_CP)||CTeamFortress2Mod::isMapType(TF_MAP_TC)),fGetFlagUtility));
	// only defend if defend area is > 0
	utils.addUtility(CBotUtility(BOT_UTIL_DEFEND_POINT,(m_iCurrentDefendArea>0) && (CTeamFortress2Mod::isMapType(TF_MAP_CART)||CTeamFortress2Mod::isMapType(TF_MAP_CARTRACE)||CTeamFortress2Mod::isMapType(TF_MAP_ARENA)||CTeamFortress2Mod::isMapType(TF_MAP_KOTH)||CTeamFortress2Mod::isMapType(TF_MAP_CP)||CTeamFortress2Mod::isMapType(TF_MAP_TC))&&m_iClass!=TF_CLASS_SCOUT,fDefendFlagUtility));

	if ( CTeamFortress2Mod::isMapType(TF_MAP_CARTRACE) )
	{
		if ( getTeam() == TF2_TEAM_BLUE )
		{
		    m_pDefendPayloadBomb = m_pRedPayloadBomb;
		    m_pPushPayloadBomb = m_pBluePayloadBomb;
			utils.addUtility(CBotUtility(BOT_UTIL_DEFEND_PAYLOAD_BOMB,(m_pDefendPayloadBomb!=NULL),fDefendFlagUtility));
			utils.addUtility(CBotUtility(BOT_UTIL_PUSH_PAYLOAD_BOMB,(m_pPushPayloadBomb!=NULL),fGetFlagUtility));
		}
		else
		{
		    m_pDefendPayloadBomb = m_pBluePayloadBomb;
		    m_pPushPayloadBomb = m_pRedPayloadBomb;
			utils.addUtility(CBotUtility(BOT_UTIL_DEFEND_PAYLOAD_BOMB,(m_pDefendPayloadBomb!=NULL),fDefendFlagUtility));
			utils.addUtility(CBotUtility(BOT_UTIL_PUSH_PAYLOAD_BOMB,(m_pPushPayloadBomb!=NULL),fGetFlagUtility));
		}
	}
	else if ( CTeamFortress2Mod::isMapType(TF_MAP_CART) )
	{
		if ( getTeam() == TF2_TEAM_BLUE )
		{
		    m_pPushPayloadBomb = m_pBluePayloadBomb;
			m_pDefendPayloadBomb = NULL;
			utils.addUtility(CBotUtility(BOT_UTIL_PUSH_PAYLOAD_BOMB,(m_pPushPayloadBomb!=NULL),fGetFlagUtility));
			// Goto Payload bomb
		}
		else
		{
			m_pPushPayloadBomb = NULL;
			m_pDefendPayloadBomb = m_pRedPayloadBomb;
			// Defend Payload bomb
			utils.addUtility(CBotUtility(BOT_UTIL_DEFEND_PAYLOAD_BOMB,(m_pDefendPayloadBomb!=NULL),fDefendFlagUtility));
		}
	}

	/////////////////////////////////////////////////////////
	// Work out utilities
	//////////////////////////////////////////////////////////
	utils.execute();

	while ( (util = utils.nextBest()) != NULL )
	{
		if ( executeAction(util->getId(),pWaypointResupply,pWaypointHealth,pWaypointAmmo) )
		{
			if ( CClients::clientsDebugging() )
			{
				CClients::clientDebugMsg(BOT_DEBUG_UTIL,g_szUtils[util->getId()],this);
			}

			utils.freeMemory();
			return;
		}
	}

	utils.freeMemory();
}

bool CBotTF2::lookAfterBuildings ( float *fTime )
{
	static float prevSentryHealth = 0;
	static float prevDispHealth = 0;
	static float prevTeleExtHealth = 0;
	static float prevTeleEntHealth = 0;

	if ( m_pSentryGun )
	{
		if ( prevSentryHealth > CClassInterface::getHealth(m_pSentryGun) )
			return true;

		prevSentryHealth = CClassInterface::getHealth(m_pSentryGun);

		if ( distanceFrom(m_pSentryGun) > 100 )
			setMoveTo(CBotGlobals::entityOrigin(m_pSentryGun),3);
		else
			stopMoving();

		lookAtEdict(m_pSentryGun);
		setLookAt(LOOK_BUILD);

		if ( *fTime < engine->Time() )
		{
			m_pButtons->tap(IN_ATTACK);
			*fTime = engine->Time() + randomFloat(10.0f,20.0f);
		}

	}
	else
		return false;

	if ( m_pDispenser )
	{
		if ( prevDispHealth > CClassInterface::getHealth(m_pDispenser) )
			return true;

		prevDispHealth = CClassInterface::getHealth(m_pDispenser);
	}
	else
		return false;

	if ( m_pTeleExit )
	{
		if ( prevTeleExtHealth > CClassInterface::getHealth(m_pTeleExit) )
			return true;

		prevTeleExtHealth = CClassInterface::getHealth(m_pTeleExit);
	}
	else
		return false;

	if ( m_pTeleEntrance )
	{
		if ( prevTeleEntHealth > CClassInterface::getHealth(m_pTeleEntrance) )
			return true;

		prevTeleEntHealth = CClassInterface::getHealth(m_pTeleEntrance);
	}
	else
		return false;

	setLookAtTask(LOOK_AROUND,3);


	return false;
}
//
// Execute a given Action
//
bool CBotTF2 :: executeAction ( eBotAction id, CWaypoint *pWaypointResupply, CWaypoint *pWaypointHealth, CWaypoint *pWaypointAmmo )
{
	CWaypoint *pWaypoint;

		switch ( id )
		{
		case BOT_UTIL_DEFEND_PAYLOAD_BOMB:
			{
				pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_DEFEND,getTeam(),m_iCurrentDefendArea,true);

				if ( pWaypoint )
				{
					m_pSchedules->add(new CBotDefendSched(pWaypoint->getOrigin()));
					return true;
				}

				if ( m_pDefendPayloadBomb )
				{
					m_pSchedules->add(new CBotTF2DefendPayloadBombSched(m_pDefendPayloadBomb));
					return true;
				}
			}
			break;
		case BOT_UTIL_PUSH_PAYLOAD_BOMB:
			{
				if ( m_pPushPayloadBomb )
				{
					m_pSchedules->add(new CBotTF2PushPayloadBombSched(m_pPushPayloadBomb));
					return true;
				}
			}
			break;
		case BOT_UTIL_ENGI_LOOK_AFTER_SENTRY:
			{
				m_pSchedules->add(new CBotTFEngiLookAfterSentry(m_pSentryGun));			
				return true;
			}
			break;
		case BOT_UTIL_DEFEND_FLAG:
			// use last known flag position
			{
				CWaypoint *pWaypoint = NULL;
				
				pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_DEFEND,getTeam());
				
				//if ( pWaypoint && randomInt(0,1) )
				//	pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),pWaypoint->getOrigin());

				if ( pWaypoint )
				{
					setLookAt(pWaypoint->getOrigin());
					m_pSchedules->add(new CBotDefendSched(pWaypoint->getOrigin()));
					return true;
				}
			}
			break;
		case BOT_UTIL_DEFEND_FLAG_LASTKNOWN:
			// find our flag waypoint
			{
				setLookAt(m_vLastKnownTeamFlagPoint);
				m_pSchedules->add(new CBotDefendSched(m_vLastKnownTeamFlagPoint));
				return true;
			}
			break;
		case BOT_UTIL_ATTACK_POINT:
			
			/*pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_DEFEND,getTeam(),m_iCurrentAttackArea,true);

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotDefendSched(pWaypoint->getOrigin()));
				return true;
			}*/

			pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT,0,m_iCurrentAttackArea,true);

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotAttackPointSched(pWaypoint->getOrigin(),pWaypoint->getRadius(),pWaypoint->getArea()));
				return true;
			}
			break;
		case BOT_UTIL_DEFEND_POINT:

			pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_DEFEND,getTeam(),m_iCurrentDefendArea,true);

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotDefendSched(pWaypoint->getOrigin()));
				return true;
			}

			pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT,0,m_iCurrentDefendArea,true);

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotDefendPointSched(pWaypoint->getOrigin(),pWaypoint->getRadius(),pWaypoint->getArea()));
				return true;
			}
			break;
		case BOT_UTIL_CAPTURE_FLAG:
			pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT,getTeam());

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
				return true;
			}
			break;
		case BOT_UTIL_BUILDTELENT:
			pWaypoint = CWaypoints::getWaypoint(CWaypointLocations::NearestWaypoint(m_vTeleportEntrance,300,-1,true,false,true,NULL,false,getTeam(),true));

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotTFEngiBuild(ENGI_ENTRANCE,m_vTeleportEntrance));

				return true;
			}
			
			break;
		case BOT_UTIL_BUILDTELEXT:

			if ( m_bTeleportExitVectorValid )
				pWaypoint = CWaypoints::getWaypoint(CWaypointLocations::NearestWaypoint(m_vTeleportExit,150,-1,true,false,true,NULL,false,getTeam(),true));
			else
				pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT,getTeam());//CTeamFortress2Mod::getArea());

			if ( pWaypoint )
			{
				m_bTeleportExitVectorValid = true;
				m_vTeleportExit = pWaypoint->getOrigin();
				m_pSchedules->add(new CBotTFEngiBuild(ENGI_EXIT,m_vTeleportExit));

				return true;
			}

			break;
		case BOT_UTIL_BUILDSENTRY:

			if ( m_bSentryGunVectorValid )
				pWaypoint = CWaypoints::getWaypoint(CWaypointLocations::NearestWaypoint(m_vSentryGun,150,-1,true,false,true,NULL,false,getTeam(),true));
			else
			{
				//if ( CTeamFortress2Mod::isMapType(TF_MAPTYPE_CP) || CTeamFortress2Mod::isMapType(TF_MAPTYPE_PL) )
				//	pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SENTRY,getTeam(),m_iCurrentDefendArea,true);

				//if ( !pWaypoint )
					pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SENTRY,getTeam());
			}

			if ( pWaypoint )
			{
				m_vSentryGun = pWaypoint->getOrigin();
				m_bSentryGunVectorValid = true;
				m_pSchedules->add(new CBotTFEngiBuild(ENGI_SENTRY,m_vSentryGun));
				return true;
			}
			break;
		case BOT_UTIL_BUILDDISP:

			if ( m_bDispenserVectorValid )
				pWaypoint = CWaypoints::getWaypoint(CWaypointLocations::NearestWaypoint(m_vDispenser,150,-1,true,false,true,NULL,false,getTeam(),true));
			else
				pWaypoint = CWaypoints::getWaypoint(CWaypointLocations::NearestWaypoint(CBotGlobals::entityOrigin(m_pSentryGun),150,-1,true,false,true,NULL,false,getTeam(),true));			

			if ( pWaypoint )
			{
				m_vDispenser = pWaypoint->getOrigin();
				m_bDispenserVectorValid = true;
				m_pSchedules->add(new CBotTFEngiBuild(ENGI_DISP,m_vDispenser+Vector(randomFloat(-96,96),randomFloat(-96,96),0)));

				return true;
			}
			break;
		case BOT_UTIL_UPGTMSENTRY:
			m_pSchedules->add(new CBotTFEngiUpgrade(m_pNearestAllySentry));
			return true;
		case BOT_UTIL_UPGTMDISP:
			m_pSchedules->add(new CBotTFEngiUpgrade(m_pNearestDisp));
			return true;
		case BOT_UTIL_UPGSENTRY:
			m_pSchedules->add(new CBotTFEngiUpgrade(m_pSentryGun));
			return true;
		case BOT_UTIL_UPGTELENT:
			m_pSchedules->add(new CBotTFEngiUpgrade(m_pTeleEntrance));
			return true;
		case BOT_UTIL_UPGTELEXT:
			m_pSchedules->add(new CBotTFEngiUpgrade(m_pTeleExit));
			return true;
		case BOT_UTIL_UPGDISP:
			m_pSchedules->add(new CBotTFEngiUpgrade(m_pDispenser));
			return true;
		case BOT_UTIL_GETAMMODISP:
			m_pSchedules->add(new CBotGetMetalSched(CBotGlobals::entityOrigin(m_pDispenser)));
			return true;
		case BOT_UTIL_GOTORESUPPLY_FOR_HEALTH:
			m_pSchedules->add(new CBotTF2GetHealthSched(pWaypointResupply->getOrigin()));
			return true;
		case BOT_UTIL_GOTORESUPPLY_FOR_AMMO:
			m_pSchedules->add(new CBotTF2GetAmmoSched(pWaypointResupply->getOrigin()));
			return true;
		case BOT_UTIL_FIND_NEAREST_HEALTH:
			m_pSchedules->add(new CBotTF2GetHealthSched(pWaypointHealth->getOrigin()));
			return true;
		case BOT_UTIL_FIND_NEAREST_AMMO:
			m_pSchedules->add(new CBotTF2GetAmmoSched(pWaypointAmmo->getOrigin()));
			return true;
		case BOT_UTIL_GOTODISP:
			m_pSchedules->removeSchedule(SCHED_USE_DISPENSER);
			m_pSchedules->addFront(new CBotUseDispSched(m_pNearestDisp));

			m_fPickupTime = engine->Time() + randomFloat(6.0f,20.0f);
			return true;
		case BOT_UTIL_GETHEALTHKIT:
			m_pSchedules->removeSchedule(SCHED_PICKUP);
			m_pSchedules->addFront(new CBotPickupSched(m_pHealthkit));

			m_fPickupTime = engine->Time() + randomFloat(5.0f,10.0f);

			return true;
		case BOT_UTIL_GETAMMOKIT:
			m_pSchedules->removeSchedule(SCHED_PICKUP);
			m_pSchedules->addFront(new CBotPickupSched(m_pAmmo));

			m_fPickupTime = engine->Time() + randomFloat(5.0f,10.0f);
			return true;
		case BOT_UTIL_SNIPE:
			pWaypoint =CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SNIPER,getTeam());

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotTF2SnipeSched(pWaypoint->getOrigin()));
				return true;
			}
			break;
		case BOT_UTIL_GETFLAG_LASTKNOWN:
			pWaypoint = CWaypoints::getWaypoint(CWaypoints::nearestWaypointGoal(-1,m_vLastKnownFlagPoint,512.0,getTeam()));

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotTF2FindFlagSched(m_vLastKnownFlagPoint));
				return true;
			}
			break;
		case BOT_UTIL_GETFLAG:
			pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_FLAG,getTeam());

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotTF2GetFlagSched(pWaypoint->getOrigin()));
				return true;
			}
			break;
		case BOT_UTIL_ROAM:
			// roam
			pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomFlaggedWaypoint(getTeam()));

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
				return true;
			}
			break;
		default:
			break;
		}

		return false;
}

void CBotTF2 :: touchedWpt ( CWaypoint *pWaypoint )
{
	CBot::touchedWpt(pWaypoint);

	if ( canGotoWaypoint (getOrigin(), pWaypoint) )
	{
		if ( pWaypoint->hasFlag(CWaypointTypes::W_FL_ROCKET_JUMP) )
		{
			m_pSchedules->addFront(new CBotSchedule(new CBotTFRocketJump()));
		}
		else if ( pWaypoint->hasFlag(CWaypointTypes::W_FL_DOUBLEJUMP) )
		{
			extern ConVar bot_scoutdj;
			tapButton(IN_JUMP);
			m_fDoubleJumpTime = engine->Time() + bot_scoutdj.GetFloat();
			//m_pSchedules->addFront(new CBotSchedule(new CBotTFDoubleJump()));
		}
	}
}

Vector CBotTF2 :: getAimVector ( edict_t *pEntity )
{
	Vector vAim = CBot::getAimVector(pEntity);
	CBotWeapon *pWp = getCurrentWeapon();
	float fDist = distanceFrom(pEntity);

	if ( pWp )
	{

		if ( m_iClass == TF_CLASS_MEDIC )
		{
			if ( pWp->getID() == TF2_WEAPON_SYRINGEGUN )
				vAim = vAim + Vector(0,0,sqrt(fDist)*2);
		}
		else if ( (m_iClass == TF_CLASS_SOLDIER) || (m_iClass == TF_CLASS_DEMOMAN) )
		{
			switch ( pWp->getID() )
			{
			case TF2_WEAPON_ROCKETLAUNCHER:
			{
				vAim = vAim - Vector(0,0,randomFloat(16.0f,36.0f));
			}
			// fall through
			case TF2_WEAPON_GRENADELAUNCHER:
			{
				CClient *pClient = CClients::get(pEntity);
				Vector vVelocity;

				if ( pClient )
				{
					vVelocity = pClient->getVelocity();
					vAim = vAim + (vVelocity*randomFloat(0.4f,0.9f));
				}

				if ( pWp->getID() == TF2_WEAPON_GRENADELAUNCHER )
					vAim = vAim + Vector(0,0,sqrt(fDist));
			}
			break;
			}
		}
	}

	return vAim;
}

void CBotTF2 :: checkDependantEntities ()
{
	CBotFortress::checkDependantEntities();
	checkEntity(&m_pRedPayloadBomb);
	checkEntity(&m_pBluePayloadBomb);
}

eBotFuncState CBotTF2 :: rocketJump(int *iState,float *fTime)
{
	extern ConVar bot_rj;

	setLookAtTask(LOOK_GROUND,6);

	switch ( *iState )
	{
	case 0:
		{
			if ( (getSpeed() > 100) && (CBotGlobals::playerAngles(m_pEdict).x > 86.0f )  )
			{
				m_pButtons->tap(IN_JUMP);
				*iState = *iState + 1;
				*fTime = engine->Time() + bot_rj.GetFloat();//randomFloat(0.08,0.5);

				return BOT_FUNC_CONTINUE;
			}
		}
		break;
	case 1:
		{
			if ( *fTime < engine->Time() )
			{
				m_pButtons->tap(IN_ATTACK);

				return BOT_FUNC_COMPLETE;
			}
		}
		break;
	}

	return BOT_FUNC_CONTINUE;
}



bool CBotTF2 :: handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy )
{
	/* Handle Spy Attacking Choice here */
	if ( m_iClass == TF_CLASS_SPY )	 
	{
		if ( isDisguised() )
		{
			if ( (m_fFrenzyTime < engine->Time()) && ((distanceFrom(pEnemy)>130) || fabs(CBotGlobals::yawAngleFromEdict(pEnemy,getOrigin())) > 75 ))
				return true;
		}
	}

	if ( pWeapon )
	{
		bool bSecAttack = false;

		clearFailedWeaponSelect();

		if ( pWeapon->isMelee() )
		{
			setMoveTo(CBotGlobals::entityOrigin(pEnemy),5);
			//dontAvoid
			m_fAvoidTime = engine->Time() + 1.0f;
		}

		if ( CTeamFortress2Mod::isRocket(m_pEdict,CTeamFortress2Mod::getEnemyTeam(getTeam())) )
		{
			float fDistance = distanceFrom(pEnemy);

			if ( (fDistance < 400) && pWeapon->canDeflectRockets() && (pWeapon->getAmmo(this) > 25) )
				bSecAttack = true;
			else
				return false;
		}

		if ( (m_iClass == TF_CLASS_SNIPER) && (pWeapon->getID() == TF2_WEAPON_SNIPERRIFLE) ) 
		{
			stopMoving();

			if ( !CTeamFortress2Mod::TF2_IsPlayerZoomed(m_pEdict) )
				secondaryAttack();

			if ( m_fSnipeAttackTime < engine->Time() )
			{
				primaryAttack();
				m_fSnipeAttackTime = engine->Time() + randomFloat(0.5f,3.0f);
			}
		}
		else if ( !bSecAttack )
		{
			if ( pWeapon->mustHoldAttack() )
				primaryAttack(true);
			else
				primaryAttack();
		}
		else
		{
			tapButton(IN_ATTACK2);
		}
	}
	else
		primaryAttack();

	return true;
}

bool CBotTF2 :: upgradeBuilding ( edict_t *pBuilding )
{
	Vector vOrigin = CBotGlobals::entityOrigin(pBuilding);

	CBotWeapon *pWeapon = getCurrentWeapon();

	if ( !pWeapon )
		return false;
	else if ( pWeapon->getID() != TF2_WEAPON_WRENCH )
	{
		if ( !select_CWeapon(CWeapons::getWeapon(TF2_WEAPON_WRENCH)) )
			return false;
	}
	else
	{
		clearFailedWeaponSelect();

		if ( distanceFrom(vOrigin) > 100 )
		{
			setMoveTo(vOrigin,3);			
		}
		else
		{
			duck(true);
			primaryAttack();
		}
	}

	lookAtEdict(pBuilding);
	m_fLookSetTime = 0;
	setLookAtTask(LOOK_EDICT,3);
	m_fLookSetTime = engine->Time() + randomFloat(3.0,8.0);

	return true;
}

void CBotTF2::roundReset(bool bFullReset)
{
	m_pRedPayloadBomb = NULL;
	m_pBluePayloadBomb = NULL;

    m_bEntranceVectorValid = false;
	m_bSentryGunVectorValid = false;
	m_bDispenserVectorValid = false;
	m_bTeleportExitVectorValid = false;
	m_pPrevSpy = NULL;
	m_pHeal = NULL;
	m_pSentryGun = NULL;
	m_pDispenser = NULL;
	m_pTeleEntrance = NULL;
	m_pTeleExit = NULL;
	m_pAmmo = NULL;
	m_pHealthkit = NULL;
	m_pNearestDisp = NULL;
	m_pNearestEnemySentry = NULL;
	m_pNearestAllySentry = NULL;
	m_pFlag = NULL;
	m_pPrevSpy = NULL;

	flagReset();
	teamFlagReset();

	m_pNavigator->freeMapMemory();

	CPoints::getAreas(getTeam(),&m_iCurrentDefendArea,&m_iCurrentAttackArea);

	//m_pPayloadBomb = NULL;
}

/// TO DO : list of areas
void CBotTF2::getDefendArea ( vector<int> *m_iAreas )
{
	CPoints::getAreas(getTeam(),&m_iCurrentDefendArea,&m_iCurrentAttackArea);
	m_iAreas->push_back(m_iCurrentDefendArea);
}

void CBotTF2::getAttackArea ( vector <int> *m_iAreas )
{
	CPoints::getAreas(getTeam(),&m_iCurrentDefendArea,&m_iCurrentAttackArea);
	m_iAreas->push_back(m_iCurrentAttackArea);
}

void CBotTF2::pointCaptured(int iPoint, int iTeam, const char *szPointName)
{
	//m_pPayloadBomb = NULL;
	m_pRedPayloadBomb = NULL;
	m_pBluePayloadBomb = NULL;

	m_pNavigator->freeMapMemory();

	CPoints::getAreas(getTeam(),&m_iCurrentDefendArea,&m_iCurrentAttackArea);
}

// Is Enemy Function
// take a pEdict entity to check if its an enemy
// return TRUE to "OPEN FIRE" (Attack)
// return FALSE to ignore
bool CBotTF2 :: isEnemy ( edict_t *pEdict,bool bCheckWeapons )
{
	int iEnemyTeam;
	bool bIsPipeBomb = false;
	bool bIsRocket = false;
	int bValid = false;

	if ( !CBotGlobals::entityIsAlive(pEdict) )
		return false;

	if ( ENTINDEX(pEdict) <= CBotGlobals::maxClients() )
	{
		if ( CBotGlobals::getTeam(pEdict) != getTeam() )
		{
			if ( m_iClass == TF_CLASS_SPY )	
			{
				if ( !bCheckWeapons )
					return true;
			}	
			
			if ( CClassInterface::getTF2Class(pEdict) == (int)TF_CLASS_SPY )
			{
				int dteam, dclass, dhealth, dindex;

				if ( CClassInterface::getTF2SpyDisguised(pEdict,&dclass,&dteam,&dindex,&dhealth) )
				{
					if ( CTeamFortress2Mod::TF2_IsPlayerCloaked(pEdict) )
						bValid = false;
					else if ( dteam != getTeam() )
					{
						bValid = true;
					}
					else if ( dindex == ENTINDEX(m_pEdict) )
					{
						bValid = true;
					}
					else if ( !isClassOnTeam(dclass,getTeam()) )
						bValid = true;
					else
						bValid = thinkSpyIsEnemy(pEdict);

					if ( bValid && bCheckWeapons )
						foundSpy(pEdict);
				}
				
				//if ( CTeamFortress2Mod::TF2_IsPlayerDisguised(pEdict) || CTeamFortress2Mod::TF2_IsPlayerCloaked(pEdict) )
				//	bValid = false;				
			}
			else
				bValid = true;
		}
	}
	// "FrenzyTime" is the time it takes for the bot to check out where he got hurt
	else if ( (m_iClass != TF_CLASS_SPY) || (m_fFrenzyTime > engine->Time()) )	
	{
		iEnemyTeam = CTeamFortress2Mod::getEnemyTeam(getTeam());

		if ( CTeamFortress2Mod::isSentry(pEdict,iEnemyTeam) || 
			CTeamFortress2Mod::isDispenser(pEdict,iEnemyTeam) || 
			CTeamFortress2Mod::isTeleporterEntrance(pEdict,iEnemyTeam) || 
			CTeamFortress2Mod::isTeleporterExit(pEdict,iEnemyTeam) )
		{
			bValid = true;
		}
		else if ( CTeamFortress2Mod::isPipeBomb ( pEdict, iEnemyTeam ) )
			bIsPipeBomb = bValid = true;
		else if ( CTeamFortress2Mod::isRocket ( pEdict, iEnemyTeam ) )
			bIsRocket = bValid = true;
	}

	if ( bValid )
	{
		if ( bCheckWeapons )
		{
			CBotWeapon *pWeapon = m_pWeapons->getBestWeapon(pEdict);

			if ( pWeapon == NULL )
			{
				return false;
			}
			else
			{
				if ( bIsPipeBomb && !pWeapon->canDestroyPipeBombs() )
					return false;
				else if ( bIsRocket && !pWeapon->canDeflectRockets() )
					return false;
			}
		}

		return true;
	}

	return false;	
}


////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// FORTRESS FOREVER


void CBotFF :: modThink ()
{
// mod specific think code here
	CBotFortress :: modThink();
}

bool CBotFF :: isEnemy ( edict_t *pEdict,bool bCheckWeapons )
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