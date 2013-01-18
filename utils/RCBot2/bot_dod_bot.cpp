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
#include "bot_navigator.h"
//#include "vstdlib/random.h" // for random functions

extern ConVar bot_beliefmulti;
extern ConVar bot_max_cc_time;
extern ConVar bot_min_cc_time;
extern ConVar bot_change_class;
extern ConVar rcbot_enemyshootfov;
extern ConVar bot_defrate;
extern ConVar rcbot_smoke_time;

const char *g_DODClassCmd[2][6] = 
{ {"cls_garand","cls_tommy","cls_bar","cls_spring","cls_30cal","cls_bazooka"},
{"cls_k98","cls_mp40","cls_mp44","cls_k98s","cls_mg42","cls_pschreck"} };

void CBroadcastBombPlanted :: execute (CBot *pBot) 
{
	CDODBot *pDODBot = (CDODBot*)pBot;

	pDODBot->bombPlanted(m_iCP,m_iTeam);
}

void CDODBot :: bombPlanted ( int iCP, int iTeam )
{
	//if ( m_iTeam != iTeam )
	updateCondition(CONDITION_CHANGED); // find new task
}

void CDODBot :: init ()
{
	CBot::init();

	m_iSelectedClass = -1;
	m_bCheckClass = false;

}

void CDODBot :: setup ()
{
	CBot::setup();
}

bool CDODBot::canGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint)
{
	if ( CBot::canGotoWaypoint(vPrevWaypoint,pWaypoint) )
	{
		if ( (m_iTeam == TEAM_ALLIES) && pWaypoint->hasFlag(CWaypointTypes::W_FL_NOALLIES) )
		{
			return false;
		}
		else if ( (m_iTeam == TEAM_AXIS) && pWaypoint->hasFlag(CWaypointTypes::W_FL_NOAXIS) )
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
	static const char *szClassname;
	static bool bNoDraw;
	static float fSmokeTime;

	CBot::setVisible(pEntity,bVisible);

	szClassname = pEntity->GetClassName();

	bNoDraw = ((CClassInterface::getEffects(pEntity) & EF_NODRAW) == EF_NODRAW);

	if ( bVisible && !bNoDraw )
	{
		if ( (m_pNearestFlag != pEntity) && CDODMod::m_Flags.isFlag(pEntity) && 
			(!m_pNearestFlag || (distanceFrom(pEntity)<distanceFrom(m_pNearestFlag)) ) )
		{
			m_pNearestFlag = pEntity;
		}
		else if ( (m_pNearestBomb != pEntity) && CDODMod::m_Flags.isBomb(pEntity) && 
			      (!m_pNearestBomb || (distanceFrom(pEntity)<distanceFrom(m_pNearestBomb)) ) )
		{
			m_pNearestBomb = pEntity;
		}
		else if ( (pEntity!=m_pEnemyGrenade) && (strncmp(szClassname,"grenade",7) == 0 ) && 
			((CClassInterface::getGrenadeThrower(pEntity) == m_pEdict) || 
			 (CClassInterface::getTeam(pEntity) == m_iEnemyTeam)) && 
			 (!m_pEnemyGrenade || (distanceFrom(pEntity)<distanceFrom(m_pEnemyGrenade))))
		{
			m_pEnemyGrenade = pEntity;
		}
		else if ( (pEntity!=m_pEnemyRocket) && (strncmp(szClassname,"rocket",6) == 0 ) && 
			(CClassInterface::getTeam(pEntity) == m_iEnemyTeam) && 
			(!m_pEnemyRocket || (distanceFrom(pEntity)<distanceFrom(m_pEnemyRocket))))
		{
			m_pEnemyRocket = pEntity;
		}
	}
	else
	{
		if ( pEntity == m_pNearestFlag ) // forget flag
		{
			if ( distanceFrom(m_pNearestFlag) > 512.0f ) // 'defend' / 'attack' radius
				m_pNearestFlag = NULL;
		}
		else if ( pEntity == m_pEnemyGrenade )
		{
			// remember grenade if it is within radius of blowing me up
			if ( distanceFrom(m_pEnemyGrenade) > (BLAST_RADIUS*2) )
				m_pEnemyGrenade = NULL;
		}
		else if ( pEntity == m_pEnemyRocket )
		{
			// remember rocket if it is within radius of blowing me up
			if ( distanceFrom(m_pEnemyRocket) > (BLAST_RADIUS*2) )
				m_pEnemyRocket = NULL;
		}
		else if ( pEntity == m_pNearestBomb )
		{
			// remember bomb if it is within radius of blowing me up
			if ( distanceFrom(m_pNearestBomb) > (BLAST_RADIUS*2) )
				m_pNearestBomb = NULL;
		}
	}

	if ( !bNoDraw && (pEntity != m_pNearestSmokeToEnemy) && (strncmp(szClassname,"grenade_smoke",13) == 0) )
	{
		fSmokeTime = gpGlobals->curtime - CClassInterface::getSmokeSpawnTime(pEntity);

		if ( (fSmokeTime >= 1.0f) && (fSmokeTime <= 10.0f) )
		{
			if ( !m_pNearestSmokeToEnemy )
			{
				m_pNearestSmokeToEnemy = pEntity;
			}
			else if ( m_pEnemy && hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
			{
				if ( (distanceFrom(pEntity) < distanceFrom(m_pNearestSmokeToEnemy)) || 
					(distanceFrom(pEntity) < (distanceFrom(m_pEnemy)+SMOKE_RADIUS)) )
				{
					// math time - good lord!
					// choose the best smoke that is worthwhile for checking enemy
					if ( m_pNearestSmokeToEnemy && (fabs(DotProductFromOrigin(CBotGlobals::entityOrigin(pEntity))-
							  DotProductFromOrigin(CBotGlobals::entityOrigin(m_pEnemy))) <=
						 fabs(DotProductFromOrigin(CBotGlobals::entityOrigin(m_pNearestSmokeToEnemy))-
							  DotProductFromOrigin(CBotGlobals::entityOrigin(m_pEnemy)))) )
					{
						m_pNearestSmokeToEnemy = pEntity;
					}
				}
			}
		}
	}
	else if ( pEntity == m_pNearestSmokeToEnemy )
	{
		fSmokeTime = gpGlobals->curtime - CClassInterface::getSmokeSpawnTime(pEntity);

		if ( bNoDraw || ((fSmokeTime < 1.0f) || (fSmokeTime > rcbot_smoke_time.GetFloat())) )
			m_pNearestSmokeToEnemy = NULL;
	}
}

void CDODBot :: selectedClass ( int iClass )
{
	m_iSelectedClass = iClass;
}

bool CDODBot :: startGame ()
{
	static int iTeam;

	iTeam = m_pPlayerInfo->GetTeamIndex();

	// not joined a team?
	if ( (iTeam != 2) && (iTeam != 3) )
	{
		if ( (m_iDesiredTeam == 2) || (m_iDesiredTeam == 3) )
			m_pPlayerInfo->ChangeTeam(m_iDesiredTeam);
		else
		{
			// manual -- auto team
			if ( CBotGlobals::numPlayersOnTeam(TEAM_ALLIES,false) <= CBotGlobals::numPlayersOnTeam(TEAM_AXIS,false) )
				m_pPlayerInfo->ChangeTeam(TEAM_ALLIES);
			else
				m_pPlayerInfo->ChangeTeam(TEAM_AXIS);
		}

		return false;
	}

	if ( (m_iDesiredClass < 0) || (m_iDesiredClass > 5) )
		chooseClass();

	// not the correct class? and desired class is valid?
	if ( (m_iDesiredClass >= 0) && (m_iDesiredClass <= 5) && (m_iDesiredClass != CClassInterface::getPlayerClassDOD(m_pEdict)) )
	{
		changeClass();

		return false;
	}

	//else if ( m_pProfile->m_iClass 
	//	engine->ClientCommand(m_pEdict,"joinclass %d\n",m_iDesiredClass); 

	/*if ( CClassInterface::getDesPlayerClassDOD(m_pEdict) == -1 )
	{
		//if ( m_iDesiredClass == 0 )
			engine->ClientCommand(m_pEdict,"joinclass %d",randomInt(0,5)); 
		//else
		//	engine->ClientCommand(m_pEdict,"joinclass %d",m_iDesiredClass);

		/*switch ( m_iDesiredClass )
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		}
		engine->ClientCommand(m_pEdict,"joinclass %d",m_iDesiredClass); 

		return false;
	}*/

	//if ( m_pPlayerInfo->get

	return true;
}

void CDODBot :: killed ( edict_t *pVictim )
{
	if ( pVictim && CBotGlobals::entityIsValid(pVictim) )
		m_pNavigator->belief(CBotGlobals::entityOrigin(pVictim),getEyePosition(),bot_beliefmulti.GetFloat(),distanceFrom(pVictim),BELIEF_SAFETY);

	return;
}

void CDODBot :: died ( edict_t *pKiller )
{
	spawnInit();

	// check if I want to change class
	m_bCheckClass = true;

	if ( randomInt(0,1) )
		m_pButtons->attack();

	if ( pKiller )
	{
		if ( CBotGlobals::entityIsValid(pKiller) )
			m_pNavigator->belief(CBotGlobals::entityOrigin(pKiller),getEyePosition(),bot_beliefmulti.GetFloat(),distanceFrom(pKiller),BELIEF_DANGER);
	}

}

void CDODBot :: seeFriendlyDie ( edict_t *pDied, edict_t *pKiller, CWeapon *pWeapon )
{
	static CWaypoint *pWpt;

	if ( (pKiller != m_pEdict) && pKiller && !m_pEnemy && !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && isEnemy(pKiller,false) )
	{
		if ( pWeapon )
		{
			DOD_Class pclass = (DOD_Class)CClassInterface::getPlayerClassDOD(pKiller);

			if ( (pclass == DOD_CLASS_SNIPER) && pWeapon->isZoomable() )
			{
				if ( (m_LastHearVoiceCommand == DOD_VC_SNIPER) && m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_US) && !m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_GER) )
					addVoiceCommand(DOD_VC_USE_GRENADE);
				else
					addVoiceCommand(DOD_VC_SNIPER);

				updateCondition(CONDITION_COVERT);
				m_fCurrentDanger += 100.0f;
			}
			else if ( (pclass == DOD_CLASS_MACHINEGUNNER) && pWeapon->isDeployable() )
			{
				if ( (m_LastHearVoiceCommand == DOD_VC_MGAHEAD) && m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_US) && !m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_GER) )
					addVoiceCommand(DOD_VC_USE_GRENADE);
				else
					addVoiceCommand(DOD_VC_MGAHEAD);

				updateCondition(CONDITION_COVERT);
				m_fCurrentDanger += 100.0f;
			}
			else
				m_fCurrentDanger += 20.0f;
		}

		// encourage bots to snoop out enemy or throw grenades
		m_fLastSeeEnemy = engine->Time();
		m_pLastEnemy = pKiller;
		m_fLastUpdateLastSeeEnemy = 0;
		m_vLastSeeEnemy = CBotGlobals::entityOrigin(m_pLastEnemy);
		m_vLastSeeEnemyBlastWaypoint = m_vLastSeeEnemy;

		pWpt = CWaypoints::getWaypoint(CWaypointLocations::NearestBlastWaypoint(m_vLastSeeEnemy,getOrigin(),1500.0f,-1,true,true,false,false,0,false));
			
		if ( pWpt )
		{
			m_vLastSeeEnemyBlastWaypoint = pWpt->getOrigin();
			updateCondition(CONDITION_CHANGED);
		}
	}
}


void CDODBot :: seeFriendlyKill ( edict_t *pTeamMate, edict_t *pDied, CWeapon *pWeapon )
{
	static CWaypoint *pWpt;
	static CBotWeapon *pCurrentWeapon;

	if ( (pDied != m_pEdict) && pTeamMate && !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && (CClassInterface::getTeam(pDied)!=m_iTeam) )
	{
		if ( pWeapon )
		{
			pCurrentWeapon = getCurrentWeapon();

			if ( pCurrentWeapon && (pWeapon->getID() == pCurrentWeapon->getID()) )
				m_fCurrentDanger -= 50.0f;
			else
				m_fCurrentDanger -= 20.0f;

			if ( m_fCurrentDanger < 0 )
				m_fCurrentDanger = 0;
		}

		if ( pDied == m_pEnemy )
		{
			if ( ( getHealthPercent() < 0.1f ) && ( randomFloat(0.0,1.0) > 0.75f ) )
				addVoiceCommand(DOD_VC_NICE_SHOT);
		}

		if ( m_pLastEnemy == pDied )
		{
			m_pLastEnemy = NULL;
			m_fLastSeeEnemy = 0;
		}
	}
}

void CDODBot :: spawnInit ()
{
	CBot::spawnInit();

	m_bHasBomb = false;

	m_pNearestBomb = NULL;

	memset(m_CheckSmoke,0,sizeof(smoke_t)*MAX_PLAYERS);

	while ( !m_nextVoicecmd.empty() )
		m_nextVoicecmd.pop();

	m_fNextVoiceCommand = 0;

	m_fDeployMachineGunTime = 0;
	m_pNearestFlag = NULL;

	m_fShoutRocket = 0;
	m_fShoutGrenade = 0;
	m_pEnemyRocket = NULL;
	m_pEnemyGrenade = NULL;

	m_fShootTime = 0;
	m_fProneTime = 0;
	m_fZoomOrDeployTime = 0;

	if ( m_pWeapons )
		m_pWeapons->clearWeapons();

	m_CurrentUtil = BOT_UTIL_MAX;
	// reset objects
	m_flSprintTime = 0;
	m_pCurrentWeapon = NULL;
	m_fFixWeaponTime = 0;
	m_LastHearVoiceCommand = DOD_VC_INVALID;
}

bool CDODBot :: isEnemy ( edict_t *pEdict,bool bCheckWeapons )
{
	extern ConVar rcbot_notarget;
	int entity_index = ENTINDEX(pEdict);

	if ( !entity_index || (entity_index > gpGlobals->maxClients) )
		return false;

	if ( !pEdict )
		return false;

	if ( !pEdict->GetUnknown() )
		return false; // left the server

	// if no target on - listen sever player is a non target
	if ( rcbot_notarget.GetBool() && (entity_index == 1) )
		return false;

	if ( pEdict == m_pEdict )
		return false;

	// not alive -- false
	if ( !CBotGlobals::entityIsAlive(pEdict) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	if ( bCheckWeapons && m_pNearestSmokeToEnemy )
	{
		return isVisibleThroughSmoke(m_pNearestSmokeToEnemy,pEdict);
	}

	return true;	
}

void CDODBot :: handleWeapons ()
{
	//
	// Handle attacking at this point
	//
	if ( m_pEnemy && !hasSomeConditions(CONDITION_ENEMY_DEAD) && 
		hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && wantToShoot() && 
		isVisible(m_pEnemy) && isEnemy(m_pEnemy,true) )
	{
		CBotWeapon *pWeapon;

		pWeapon = getBestWeapon(m_pEnemy);

		if ( m_bWantToChangeWeapon && (pWeapon != NULL) && (pWeapon != getCurrentWeapon()) && pWeapon->getWeaponIndex() )
		{
			//selectWeaponSlot(pWeapon->getWeaponInfo()->getSlot());
			selectBotWeapon(pWeapon);
		}

		setLookAtTask(LOOK_ENEMY);

		if ( !handleAttack ( pWeapon, m_pEnemy ) )
		{
			m_pEnemy = NULL;
			m_pOldEnemy = NULL;
			wantToShoot(false);
		}
	}
}

void CDODBot :: touchedWpt ( CWaypoint *pWaypoint )
{
	static int wptindex;

	CBot::touchedWpt(pWaypoint);

	wptindex = CWaypoints::getWaypointIndex(pWaypoint);

	if ( pWaypoint->hasFlag(CWaypointTypes::W_FL_BOMBS_HERE) )
		m_bHasBomb = true;
	else if ( m_pEnemy && hasSomeConditions(CONDITION_SEE_CUR_ENEMY) ) 
	{
		m_pNavigator->beliefOne(wptindex,BELIEF_DANGER,distanceFrom(m_pEnemy));
	}
	else
		m_pNavigator->beliefOne(wptindex,BELIEF_SAFETY,0);
}

void CDODBot :: changeClass ()
{
	int iTeam = getTeam();
	// change class
	//selectClass();
	helpers->ClientCommand(m_pEdict,g_DODClassCmd[iTeam-2][m_iDesiredClass]);

	m_fChangeClassTime = engine->Time() + randomFloat(bot_min_cc_time.GetFloat(),bot_max_cc_time.GetFloat());
}

void CDODBot :: chooseClass ()
{
	float fClassFitness[6]; // 6 classes
	float fTotalFitness = 0;
	float fRandom;

	short int i = 0;
	
	int iTeam = getTeam();
	int iClass;
	edict_t *pPlayer;

	for ( i = 1; i < 6; i ++ )
		fClassFitness[i] = 1.0f;

	if ( (m_iClass >= 0) && (m_iClass < 6) )
		fClassFitness[m_iClass] = 0.1f;

	for ( i = 1; i <= gpGlobals->maxClients; i ++ )
	{
		pPlayer = INDEXENT(i);
		
		if ( CBotGlobals::entityIsValid(pPlayer) && (CClassInterface::getTeam(pPlayer) == iTeam))
		{
			iClass = CClassInterface::getPlayerClassDOD(pPlayer);

			if ( (iClass >= 0) && (iClass < 6) )
				fClassFitness [iClass] *= 0.6f; 
		}
	}

	for ( int i = 0; i < 6; i ++ )
		fTotalFitness += fClassFitness[i];

	fRandom = randomFloat(0,fTotalFitness);

	fTotalFitness = 0;

	m_iDesiredClass = 0;

	for ( int i = 0; i < 6; i ++ )
	{
		fTotalFitness += fClassFitness[i];

		if ( fRandom <= fTotalFitness )
		{
			m_iDesiredClass = i;
			break;
		}
	}

}

void CDODBot :: modThink ()
{
	static float fMaxSpeed;
	static CBotWeapon *pWeapon;

	// when respawned -- check if I should change class
	if ( m_bCheckClass && !m_pPlayerInfo->IsDead())
	{
		m_bCheckClass = false;

		if ( bot_change_class.GetBool() && (m_fChangeClassTime < engine->Time()) )
		{
				// get score for this class
				float scoreValue = CDODMod::getScore(m_pEdict);

				// if I think I could do better
				if ( randomFloat(0.0f,1.0f) > (scoreValue / CDODMod::getHighestScore()) )
				{
					chooseClass();
					changeClass();
				}
		}
	}

	m_fFov = BOT_DEFAULT_FOV;
	fMaxSpeed = CClassInterface::getMaxSpeed(m_pEdict);

	setMoveSpeed(fMaxSpeed*0.75f);
	
	m_iClass = (DOD_Class)CClassInterface::getPlayerClassDOD(m_pEdict);	
	m_iTeam = getTeam();

	if ( m_iTeam == TEAM_ALLIES ) // im allies
		m_iEnemyTeam = TEAM_AXIS;
	else if ( m_iEnemyTeam == TEAM_AXIS ) // im axis and my enemy team is wrong
		m_iEnemyTeam = TEAM_ALLIES;

	// find weapons and neat stuff
	//if ( m_fFixWeaponTime < engine->Time() )
	//{
	fixWeapons(); // update weapons
	m_pCurrentWeapon = CClassInterface::getCurrentWeapon(m_pEdict);
	pWeapon = getCurrentWeapon();

	//m_fFixWeaponTime = engine->Time() + 1.0f;
	//}

	if ( m_pCurrentWeapon )
	{
		CClassInterface::getWeaponClip(m_pCurrentWeapon,&m_iClip1,&m_iClip2);

		if ( pWeapon && pWeapon->isDeployable() && CClassInterface::isMachineGunDeployed(m_pCurrentWeapon) )
			m_fDeployMachineGunTime = engine->Time();

		if ( pWeapon && pWeapon->isZoomable() && CClassInterface::isSniperWeaponZoomed(m_pCurrentWeapon) )
		{
			m_fFov = 45.0f;
		}
	}

	if ( CClassInterface::isMoveType(m_pEdict,MOVETYPE_LADDER) )
	{
		setMoveLookPriority(MOVELOOK_OVERRIDE);
		setLookAtTask(LOOK_WAYPOINT);
		m_pButtons->holdButton(IN_FORWARD,0,1,0);
		setMoveLookPriority(MOVELOOK_MODTHINK);
	}

	CClassInterface::getPlayerInfoDOD(m_pEdict,&m_bProne,&m_flStamina);

	if ( m_fCurrentDanger >= 50.0f )
	{
		// not sniper rifle or machine gun but can look down the sights
		if ( !hasSomeConditions(CONDITION_RUN) && hasSomeConditions(CONDITION_COVERT) && m_pCurrentWeapon && pWeapon && (( pWeapon->getID() == DOD_WEAPON_K98 ) || (pWeapon->getID() == DOD_WEAPON_GARAND) ))
		{
			bool bZoomed = false;

			if ( pWeapon->getID() == DOD_WEAPON_K98 )
				bZoomed = CClassInterface::isK98Zoomed(m_pCurrentWeapon);
			else
				bZoomed = CClassInterface::isGarandZoomed(m_pCurrentWeapon);

			if ( !bZoomed && (m_fZoomOrDeployTime < engine->Time()) )
			{
				secondaryAttack(); // deploy / zoom
				m_fZoomOrDeployTime = engine->Time() + randomFloat(0.1f,0.2f);
			}
		}
		
		if ( !hasSomeConditions(CONDITION_RUN) && (m_fCurrentDanger >= 80.0f) && !m_bProne && ( m_fProneTime < engine->Time() ))
		{
			m_pButtons->tap(IN_ALT1);
			m_fProneTime = engine->Time() + randomFloat(4.0f,8.0f);
		}

		setMoveSpeed(fMaxSpeed/4);

		// slow down - be careful
	}
	else if ( hasSomeConditions(CONDITION_RUN) || ((m_fCurrentDanger >= 20.0f) && (m_flStamina > 90.0f ) && (m_flSprintTime < engine->Time())) )
	{
		// unprone
		if ( m_bProne && ( m_fProneTime < engine->Time() ))
		{
			m_pButtons->tap(IN_ALT1);
			m_fProneTime = engine->Time() + randomFloat(4.0f,8.0f);
		}

		setMoveSpeed(fMaxSpeed);
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

	if ( m_pEnemyRocket )
	{
		m_fShoutRocket = m_fShoutRocket/2 + 0.5f;

		if ( m_fShoutRocket > 0.95f ) 
		{
			addVoiceCommand(DOD_VC_BAZOOKA);

			updateCondition(CONDITION_RUN);

			m_fShoutRocket = 0.0f;

			if ( !m_pSchedules->isCurrentSchedule(SCHED_GOOD_HIDE_SPOT) )
			{
				// don't interrupt current shedule, just add to front
				m_pSchedules->removeSchedule(SCHED_GOOD_HIDE_SPOT);
				m_pSchedules->addFront(new CGotoHideSpotSched(m_pEnemyRocket));
			}
		}
	}

	if ( m_pEnemyGrenade )
	{
		m_fShoutGrenade = m_fShoutGrenade/2 + 0.5f;

		if ( m_fShoutGrenade > 0.95f ) 
		{
			if ( distanceFrom(m_pEnemyGrenade) < (BLAST_RADIUS*2) )
			{
				if ( CClassInterface::getGrenadeThrower(m_pEnemyGrenade) != m_pEdict )
				{
					addVoiceCommand(DOD_VC_GRENADE2);
					m_fShoutGrenade = 0.0f;
				}

				updateCondition(CONDITION_RUN);

				if ( !m_pSchedules->isCurrentSchedule(SCHED_GOOD_HIDE_SPOT) )
				{
					// don't interrupt current shedule, just add to front
					m_pSchedules->removeSchedule(SCHED_GOOD_HIDE_SPOT);
					m_pSchedules->addFront(new CGotoHideSpotSched(m_pEnemyGrenade));
				}
			}
		}
	}

	if ( m_pNearestBomb )
	{
		int iBombID = CDODMod::m_Flags.getBombID(m_pNearestBomb);

		if ( CDODMod::m_Flags.canDefuseBomb(m_iTeam,iBombID) )
		{
			if ( !m_pSchedules->hasSchedule(SCHED_BOMB) )
			{
					CBotSchedule *attack = new CBotSchedule();

					attack->setID(SCHED_BOMB);
					attack->addTask(new CFindPathTask(m_pNearestBomb));//,LOOK_AROUND));
					attack->addTask(new CBotDODBomb(DOD_BOMB_DEFUSE,iBombID,m_pNearestBomb,CBotGlobals::entityOrigin(m_pNearestBomb),-1));
					// add defend task
					m_pSchedules->freeMemory();
					m_pSchedules->add(attack);

					removeCondition(CONDITION_PUSH);
					updateCondition(CONDITION_RUN);
			}
		}
		else if ( !m_pSchedules->hasSchedule(SCHED_BOMB) && 
			!m_pSchedules->hasSchedule(SCHED_GOOD_HIDE_SPOT) &&
			!CDODMod::m_Flags.isBombBeingDefused(iBombID) &&
			CDODMod::m_Flags.isBombPlanted(iBombID) &&
			CDODMod::m_Flags.isBombExplodeImminent(iBombID) )
		{
			if ( distanceFrom(m_pNearestBomb) < (BLAST_RADIUS*2) )
			{
				updateCondition(CONDITION_RUN);
				// don't interrupt current shedule, just add to front
				m_pSchedules->removeSchedule(SCHED_GOOD_HIDE_SPOT);
				m_pSchedules->addFront(new CGotoHideSpotSched(m_pNearestBomb));
			}
		}
	}
}

void CDODBot :: fixWeapons ()
{
	
	m_Weapons = CClassInterface::getWeaponList(m_pEdict);

	if ( m_pWeapons )
		m_pWeapons->update();
	/*
	if ( m_pWeapons && m_Weapons ) 
	{
		//m_pWeapons->clearWeapons();

		// loop through the weapons array in the CBaseCombatCharacter
		for ( unsigned int i = 0; i < MAX_WEAPONS; i ++ )
		{
			// get the information from the bot 
			const char *classname;
			CWeapon *pBotWeapon;
			CBotWeapon *pHasWeapon;
			int iWeaponState;

			edict_t *pWeapon = INDEXENT(m_Weapons[i].GetEntryIndex());

			if ( pWeapon && CBotGlobals::entityIsValid(pWeapon) )
			{
				classname = pWeapon->GetClassName();

				// find this weapon in our initialized array in the code
				pBotWeapon = CWeapons::getWeapon(classname);

				if ( pBotWeapon )
				{
					iWeaponState = CClassInterface::getWeaponState(pWeapon);
					// see if the bot has this weapon or not already
					pHasWeapon = m_pWeapons->getWeapon(pBotWeapon);

					// if the bot doesn't have it or the state has changed, update
					if ( pHasWeapon && pHasWeapon->hasWeapon() && (iWeaponState == WEAPON_NOT_CARRIED) )
						pHasWeapon->setHasWeapon(false);
					else if ( !pHasWeapon || !pHasWeapon->hasWeapon() )
						m_pWeapons->addWeapon(pBotWeapon->getID(),pWeapon);
					else if ( pHasWeapon && (pHasWeapon->getWeaponEntity() != pWeapon) )
						pHasWeapon->setWeaponEntity(pWeapon);
				}
			}
		}
	}*/
}

void CDODBot ::defending()
{
	// check to go prone or not
}

void CDODBot ::voiceCommand ( int cmd )
{
	// find voice command
	extern eDODVoiceCommand_t g_DODVoiceCommands[DOD_VC_INVALID];

	char scmd[64];
	u_VOICECMD vcmd;

	vcmd.voicecmd = cmd;
	
	sprintf(scmd,"voice_%s",g_DODVoiceCommands[cmd].pcmd);

	helpers->ClientCommand(m_pEdict,scmd);
}

void CDODBot :: hearVoiceCommand ( edict_t *pPlayer, byte cmd )
{
	switch ( cmd )
	{
	case DOD_VC_USE_GRENADE:
		if ( isVisible(pPlayer) )
		{
			updateCondition(CONDITION_GREN);
		}
		break;
	case DOD_VC_GOGOGO:
		if ( isVisible(pPlayer) )
		{
			updateCondition(CONDITION_PUSH);
			updateCondition(CONDITION_RUN);

			if ( randomFloat(0.0f,1.0f) > 0.75f )
				addVoiceCommand(DOD_VC_YES);
		}
		else if ( randomFloat(0.0f,1.0f) > 0.9f )
			addVoiceCommand(DOD_VC_NO);
		break;
	case DOD_VC_HOLD:
		if ( isVisible(pPlayer) )
		{
			updateCondition(CONDITION_COVERT);
			removeCondition(CONDITION_PUSH);
			removeCondition(CONDITION_RUN);

			if ( randomFloat(0.0f,1.0f) > 0.75f )
				addVoiceCommand(DOD_VC_YES);
		}
		else if ( randomFloat(0.0f,1.0f) > 0.9f )
			addVoiceCommand(DOD_VC_NO);
		break;
	case DOD_VC_SMOKE:
		if ( isVisible(pPlayer) )
		{
			updateCondition(CONDITION_COVERT);

			if ( m_pWeapons->hasWeapon(DOD_WEAPON_SMOKE_US) || m_pWeapons->hasWeapon(DOD_WEAPON_SMOKE_GER) )
			{
				if ( randomFloat(0.0f,1.0f) > 0.75f )
					addVoiceCommand(DOD_VC_YES);
			}
		}
		else if ( (!m_pWeapons->hasWeapon(DOD_WEAPON_SMOKE_US) && !m_pWeapons->hasWeapon(DOD_WEAPON_SMOKE_GER)) && (randomFloat(0.0f,1.0f) > 0.75f) )
		{
			addVoiceCommand(DOD_VC_NO);
		}
		break;
	case DOD_VC_SNIPER:
	case DOD_VC_MGAHEAD:
		if ( !m_pEnemy && !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && isVisible(pPlayer) )
		{
			updateCondition(CONDITION_COVERT);
			m_fCurrentDanger += 50.0f;
		}
		break;
	case DOD_VC_GRENADE2:
	case DOD_VC_BAZOOKA:
		// if I don't see anythign but I see the player calling this, hide!
		if ( !m_pEnemyGrenade && !m_pEnemyRocket && isVisible(pPlayer) && !m_pSchedules->isCurrentSchedule(SCHED_GOOD_HIDE_SPOT) )
		{
			updateCondition(CONDITION_COVERT);
			updateCondition(CONDITION_RUN);

			// don't interrupt current shedule, just add to front
			m_pSchedules->removeSchedule(SCHED_GOOD_HIDE_SPOT);
			m_pSchedules->addFront(new CGotoHideSpotSched(pPlayer));
		}

		break;
	case DOD_VC_NEED_BACKUP:
		if ( m_pNearestFlag && isVisible(pPlayer) )
		{
			Vector vPoint = CBotGlobals::entityOrigin(m_pNearestFlag);
			Vector vPlayer = CBotGlobals::entityOrigin(pPlayer);

			if ( (vPoint - vPlayer).Length() < 250 )
			{
					CBotSchedule *attack = new CBotSchedule();

					updateCondition(CONDITION_RUN);

					if ( CDODMod::isBombMap() )
					{
						CWaypoint *pWpt = CWaypoints::getWaypoint(CWaypointLocations::NearestBlastWaypoint(vPoint,vPlayer,1000.0,-1,true,false,true,false,m_iTeam,true));

						attack->setID(SCHED_DEFENDPOINT);
						attack->addTask(new CFindPathTask(pWpt->getOrigin()));
						attack->addTask(new CBotDefendTask(pWpt->getOrigin(),randomFloat(6.0f,12.0f),0,true,vPoint));
						// add defend task
						m_pSchedules->freeMemory();
						m_pSchedules->add(attack);
					}
					else
					{
						attack->setID(SCHED_ATTACKPOINT);
						attack->addTask(new CFindPathTask(vPoint));
						attack->addTask(new CBotDODAttackPoint(CDODMod::m_Flags.getFlagID(m_pNearestFlag),vPoint,150.0f));
						// add defend task
						m_pSchedules->freeMemory();
						m_pSchedules->add(attack);

					}

					if ( randomFloat(0.0f,1.0f) > 0.75f )
						addVoiceCommand(DOD_VC_YES);
			}
		}
		break;
	default:
		break;
	}

	m_LastHearVoiceCommand = (eDODVoiceCMD)cmd;

	// don't say the same command for a second or two
	if ( cmd < MAX_VOICE_CMDS )
		m_fLastVoiceCommand[cmd] = engine->Time() + randomFloat(1.0f,3.0f);
}

bool CDODBot :: executeAction ( CBotUtility *util )
{
	int iBombType = 0;
	int id = -1;
	Vector vGoal;
	edict_t *pBombTarget = NULL;

	switch ( util->getId() )
	{
	case BOT_UTIL_MESSAROUND:
		{
			// find a nearby friendly
			int i = 0;
			edict_t *pEdict;
			edict_t *pNearby = NULL;
			float fMaxDistance = 500;
			float fDistance;

			for ( i = 1; i <= CBotGlobals::maxClients(); i ++ )
			{
				pEdict = INDEXENT(i);

				if ( CBotGlobals::entityIsValid(pEdict) )
				{
					if ( CClassInterface::getTeam(pEdict) == getTeam() )
					{
						if ( (fDistance=distanceFrom(pEdict)) < fMaxDistance )
						{
							if ( isVisible(pEdict) )
							{
								// add a little bit of randomness
								if ( !pNearby || randomInt(0,1) )
								{
									pNearby = pEdict;
									fMaxDistance = fDistance;
								}
							}
						}
					}
				}
			}

			if ( pNearby )
			{
				// this will work in DOD too
				m_pSchedules->add(new CBotTF2MessAroundSched(pNearby));
				return true;
			}

			return false;
		}
	case BOT_UTIL_DEFEND_NEAREST_POINT:
		{
			Vector vGoal;
			int id = util->getIntData();
			bool defend_wpt = true;

			vGoal = util->getVectorData();

			CWaypoint *pWaypoint;

			pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_DEFEND,m_iTeam,id,true,this);

			if ( pWaypoint == NULL )
			{			
				defend_wpt = false;

				if ( distanceFrom(vGoal) > 1024 ) // outside waypoint bucket of goal
					pWaypoint = CWaypoints::getPinchPointFromWaypoint(vGoal,vGoal);
				else
					pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),vGoal);
			}

			if ( pWaypoint )
			{

				if ( randomFloat(0.0f,1.0f) > 0.9f )
					addVoiceCommand(DOD_VC_ENEMY_BEHIND);
				
				CBotSchedule *defend = new CBotSchedule();
				CBotTask *findpath = new CFindPathTask(pWaypoint->getOrigin());//,LOOK_AROUND);
				CBotTask *deftask = new CBotDefendTask(pWaypoint->getOrigin(),randomFloat(7.5f,12.5f),0,true,vGoal,defend_wpt ? LOOK_SNIPE : LOOK_AROUND);

				removeCondition(CONDITION_PUSH); 
				findpath->setCompleteInterrupt(CONDITION_PUSH);
				deftask->setCompleteInterrupt(CONDITION_PUSH);

				defend->setID(SCHED_DEFENDPOINT);
				defend->addTask(findpath);
				defend->addTask(deftask);
				// add defend task
				m_pSchedules->add(defend);

				return true;

			}
		}
		break;
	case BOT_UTIL_ATTACK_NEAREST_POINT:
		{
			Vector vGoal;
			int iFlagID = util->getIntData();

			vGoal = util->getVectorData();

			CBotSchedule *attack = new CBotSchedule();

			attack->setID(SCHED_ATTACKPOINT);
			attack->addTask(new CFindPathTask(vGoal));//,LOOK_AROUND));
			attack->addTask(new CBotDODAttackPoint(iFlagID,vGoal,150.0f));
			// add defend task
			m_pSchedules->add(attack);
			removeCondition(CONDITION_PUSH);

			if ( CDODMod::m_Flags.getNumFlagsOwned(m_iTeam) == (CDODMod::m_Flags.getNumFlags()-1) )
			{
				addVoiceCommand(DOD_VC_GOGOGO);
			}

			return true;
		}
		break;
	case BOT_UTIL_SNIPE: // find snipe or machine gun waypoint
		{
			int iFlagID = -1;
			Vector vGoal;
			CWaypoint *pWaypoint = NULL;

			if ( util->getIntData() ) // attack
			{
				if ( !CDODMod::isBombMap() || !CDODMod::isCommunalBombPoint() )
				{
					if ( CDODMod::m_Flags.getRandomEnemyControlledFlag(&vGoal,getTeam(),&iFlagID) )
					{
						if ( m_iClass == DOD_CLASS_MACHINEGUNNER )
							pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_MACHINEGUN,m_iTeam,iFlagID,true,this);
						else
							pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SNIPER,m_iTeam,iFlagID,true,this);
					}
				}
				else
				{
					// attack the bomb point -- less chance if owned many bomb points already
					if ( randomFloat(0.0f,1.0f) < 
						((float)CDODMod::m_Flags.getNumPlantableBombs(m_iTeam)/
						 CDODMod::m_Flags.getNumBombsOnMap(m_iTeam)) ) 
					{
						if ( m_iClass == DOD_CLASS_MACHINEGUNNER )
							pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_MACHINEGUN,m_iTeam,CDODMod::getBombPointArea(m_iTeam),true,this);
						else
							pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SNIPER,m_iTeam,CDODMod::getBombPointArea(m_iTeam),true,this);
					}
					else
					{
						// attack a point
						if ( CDODMod::m_Flags.getRandomEnemyControlledFlag(&vGoal,getTeam(),&iFlagID) )
						{
							if ( m_iClass == DOD_CLASS_MACHINEGUNNER )
								pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_MACHINEGUN,m_iTeam,iFlagID,true,this);
							else
								pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SNIPER,m_iTeam,iFlagID,true,this);
						}
					}
				}

				if ( pWaypoint ) // attack position -- pushing
					removeCondition(CONDITION_PUSH);
			}
			else // defend
			{
				if ( CDODMod::m_Flags.getRandomTeamControlledFlag(&vGoal,getTeam(),&iFlagID) )
				{
					if ( m_iClass == DOD_CLASS_MACHINEGUNNER )
						pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_MACHINEGUN,m_iTeam,iFlagID,true,this);
					else
						pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SNIPER,m_iTeam,iFlagID,true,this);
				}
			}

			if ( pWaypoint == NULL )
			{
				if ( m_iClass == DOD_CLASS_MACHINEGUNNER )
					pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_MACHINEGUN,m_iTeam);
				else if ( pWaypoint == NULL )
					pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SNIPER,m_iTeam);
			}

			if ( pWaypoint )
			{
				CBotSchedule *snipe = new CBotSchedule();
				CBotTask *findpath = new CFindPathTask(pWaypoint->getOrigin());
				CBotTask *snipetask;
				
				// find Z for goal if no flag id
				if ( (iFlagID == -1) && (pWaypoint->getArea() > 0) && CDODMod::isBombMap() && CDODMod::isCommunalBombPoint() )
				{
					CWaypoint *pBombs = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_BOMBS_HERE,m_iTeam,pWaypoint->getArea(),true);

					if ( pBombs )
					{
						iFlagID = pWaypoint->getArea();
						vGoal = pBombs->getOrigin();
					}
				}

				snipetask = new CBotDODSnipe(util->getWeaponChoice(),pWaypoint->getOrigin(),pWaypoint->getAimYaw(),iFlagID!=-1,vGoal.z+48);

				findpath->setCompleteInterrupt(CONDITION_PUSH);
				snipetask->setCompleteInterrupt(CONDITION_PUSH);

				snipe->setID(SCHED_DEFENDPOINT);
				snipe->addTask(findpath);
				snipe->addTask(snipetask);
				
				m_pSchedules->add(snipe);

				return true;
			}
		}
		break;
	case BOT_UTIL_DEFEND_NEAREST_BOMB:
		vGoal = util->getVectorData();
	case BOT_UTIL_DEFEND_BOMB: // fall through -- no break
		if ( util->getId() == BOT_UTIL_DEFEND_BOMB )
			CDODMod::m_Flags.getRandomBombToDefend(&vGoal,m_iTeam,&pBombTarget,&id);
	case BOT_UTIL_DEFEND_POINT:
		{
			int id = -1;
			bool defend_wpt = true;

			if ( util->getId() == BOT_UTIL_DEFEND_POINT )
			{
				if ( !CDODMod::m_Flags.getRandomTeamControlledFlag(&vGoal,getTeam(),&id) )
					return false;
			}

			CWaypoint *pWaypoint;

			pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_DEFEND,m_iTeam,id,true,this);

			if ( pWaypoint == NULL )
			{			
				defend_wpt = false;

				if ( distanceFrom(vGoal) > 1024 ) // outside waypoint bucket of goal
					pWaypoint = CWaypoints::getPinchPointFromWaypoint(vGoal,vGoal);
				else
					pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),vGoal);
			}

			if ( pWaypoint )
			{
				CBotSchedule *defend = new CBotSchedule();
				CBotTask *findpath = new CFindPathTask(pWaypoint->getOrigin());//,LOOK_AROUND);
				CBotTask *deftask = new CBotDefendTask(pWaypoint->getOrigin(),randomFloat(7.5f,12.5f),0,true,vGoal,defend_wpt ? LOOK_SNIPE : LOOK_AROUND);

				removeCondition(CONDITION_PUSH); 
				findpath->setCompleteInterrupt(CONDITION_PUSH);
				deftask->setCompleteInterrupt(CONDITION_PUSH);

				defend->setID(SCHED_DEFENDPOINT);
				defend->addTask(findpath);
				defend->addTask(deftask);
				// add defend task
				m_pSchedules->add(defend);

				return true;
			}
		}
		break;
	// no breaking -- defusing/planting bomb stuff
	case BOT_UTIL_DEFUSE_NEAREST_BOMB:
		iBombType = DOD_BOMB_DEFUSE;
	case BOT_UTIL_PLANT_NEAREST_BOMB:
		id = util->getIntData();
		vGoal = util->getVectorData();
		pBombTarget = m_pNearestBomb;

		if ( util->getId() == BOT_UTIL_PLANT_NEAREST_BOMB )
			iBombType = DOD_BOMB_PLANT;
	case BOT_UTIL_DEFUSE_BOMB:
		if ( util->getId() == BOT_UTIL_DEFUSE_BOMB )
		{
			if ( !CDODMod::m_Flags.getRandomBombToDefuse(&vGoal,m_iTeam,&pBombTarget,&id) )
				return false;

			iBombType = DOD_BOMB_DEFUSE;
		}
	case BOT_UTIL_PLANT_BOMB:
		{
			if ( util->getId() == BOT_UTIL_PLANT_BOMB )
			{
				if ( !CDODMod::m_Flags.getRandomBombToPlant(&vGoal,m_iTeam,&pBombTarget,&id) )
					return false;

				iBombType = DOD_BOMB_PLANT;
			}
					
			CWaypoint *pWaypoint;

			if ( distanceFrom(vGoal) > 1024 ) // outside waypoint bucket of goal
				pWaypoint = CWaypoints::getPinchPointFromWaypoint(vGoal,vGoal);
			else
				pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),vGoal);

			CBotSchedule *attack = new CBotSchedule();

			attack->setID(SCHED_BOMB);

			if ( pWaypoint && (iBombType == DOD_BOMB_PLANT) && (randomFloat(0.0f,200.0f) < m_pNavigator->getBelief(CWaypoints::getWaypointIndex(pWaypoint))) )
			{
				attack->addTask(new CFindPathTask(pWaypoint->getOrigin()));//,LOOK_AROUND));
				attack->addTask(new CBotInvestigateTask(pWaypoint->getOrigin(),250,randomFloat(3.0f,5.0f),CONDITION_SEE_CUR_ENEMY));
			}
			attack->addTask(new CFindPathTask(vGoal));
			attack->addTask(new CBotDODBomb(iBombType,id,pBombTarget,vGoal,-1));
			// add defend task
			m_pSchedules->add(attack);

			if ( iBombType == DOD_BOMB_DEFUSE ) 
				updateCondition(CONDITION_RUN);

			removeCondition(CONDITION_PUSH);
			
			return true;
		
		}
		break;
	case BOT_UTIL_PICKUP_BOMB:
		{
			CWaypoint *pWaypoint;

			pWaypoint = CWaypoints::getWaypoint(CWaypoints::nearestWaypointGoal(CWaypointTypes::W_FL_BOMBS_HERE,getOrigin(),8192.0,m_iTeam));

			if ( pWaypoint )
			{
				if ( CDODMod::isCommunalBombPoint() )
				{
					CWaypoint *pWaypointPinch = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_DEFEND|CWaypointTypes::W_FL_SNIPER|CWaypointTypes::W_FL_MACHINEGUN,m_iTeam,pWaypoint->getArea(),true,this);

					if ( !pWaypointPinch )
					{
						if ( distanceFrom(pWaypoint->getOrigin()) > 1024.0 )
							pWaypointPinch = CWaypoints::getPinchPointFromWaypoint(pWaypoint->getOrigin(),pWaypoint->getOrigin());
						else
							pWaypointPinch = CWaypoints::getPinchPointFromWaypoint(getOrigin(),pWaypoint->getOrigin());
					}

					if ( pWaypointPinch && (distanceFrom(pWaypointPinch->getOrigin()) < distanceFrom(pWaypoint->getOrigin())) )
					{
						CBotSchedule *defend = new CBotSchedule();
						CBotTask *findpath1 = new CFindPathTask(pWaypointPinch->getOrigin());//,LOOK_AROUND);
						CBotTask *findpath2 = new CFindPathTask(pWaypoint->getOrigin());//,LOOK_AROUND);
						CBotTask *deftask = new CBotDefendTask(pWaypointPinch->getOrigin(),randomFloat(3.5f,6.5f),0,true,pWaypoint->getOrigin(),LOOK_SNIPE);

						removeCondition(CONDITION_PUSH); 
						deftask->setCompleteInterrupt(CONDITION_PUSH);

						defend->setID(SCHED_DEFENDPOINT);
						defend->addTask(findpath1); // find a spot to look out for enemies
						defend->addTask(deftask);   // look out for enemies
						defend->addTask(findpath2); // pickup bomb
						// add defend task
						m_pSchedules->add(defend);
					}
					else
						m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
				}
				else
					m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));

				return true;
			}
		}
		break;
	case BOT_UTIL_FIND_LAST_ENEMY:
		{
			Vector vVelocity = Vector(0,0,0);
			CClient *pClient = CClients::get(m_pLastEnemy);
			CBotSchedule *pSchedule = new CBotSchedule();
			
			CFindPathTask *pFindPath = new CFindPathTask(m_vLastSeeEnemy,LOOK_LAST_ENEMY);	

			pFindPath->setCompleteInterrupt(CONDITION_SEE_CUR_ENEMY);
			
			if ( !CClassInterface::getVelocity(m_pLastEnemy,&vVelocity) )
			{
				if ( pClient )
					vVelocity = pClient->getVelocity();
			}

			pSchedule->addTask(pFindPath);
			pSchedule->addTask(new CFindLastEnemy(m_vLastSeeEnemy,vVelocity));

			//////////////
			pFindPath->setNoInterruptions();

			m_pSchedules->add(pSchedule);

			m_bLookedForEnemyLast = true;

			return true;
		}
	case BOT_UTIL_THROW_GRENADE:
		{
		// find hide waypoint
			CWaypoint *pWaypoint = CWaypoints::getWaypoint(CWaypointLocations::GetCoverWaypoint(getOrigin(),m_vLastSeeEnemy,NULL));

			if ( pWaypoint )
			{
				CBotWeapon *pWeapon = util->getWeaponChoice();
				CBotSchedule *pSched = new CBotSchedule();
				CFindPathTask *pathtask = new CFindPathTask(pWaypoint->getOrigin());

				pathtask->setNoInterruptions();

				pSched->addTask(new CThrowGrenadeTask(pWeapon,getAmmo(pWeapon->getWeaponInfo()->getAmmoIndex1()),m_vLastSeeEnemyBlastWaypoint)); // first - throw
				pSched->addTask(pathtask); // 2nd -- hide

				m_pSchedules->add(pSched);

				removeCondition(CONDITION_COVERT);
				removeCondition(CONDITION_GREN);

				return true;
			}

			return true;
		}
		break;
	case BOT_UTIL_ATTACK_POINT:
		{
			Vector vGoal;
			int iFlagID;

			if ( CDODMod::m_Flags.getRandomEnemyControlledFlag(&vGoal,getTeam(),&iFlagID) )
			{
				CWaypoint *pWaypoint;

				if ( distanceFrom(vGoal) > 1024 ) // outside waypoint bucket of goal
					pWaypoint = CWaypoints::getPinchPointFromWaypoint(vGoal,vGoal);
				else
					pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),vGoal);

				CBotSchedule *attack = new CBotSchedule();

				attack->setID(SCHED_ATTACKPOINT);

				if ( pWaypoint && (randomFloat(0.0f,MAX_BELIEF) < m_pNavigator->getBelief(CWaypoints::getWaypointIndex(pWaypoint))) )
				{
					attack->addTask(new CFindPathTask(pWaypoint->getOrigin()));//,LOOK_AROUND));
					attack->addTask(new CBotInvestigateTask(pWaypoint->getOrigin(),250,randomFloat(3.0f,5.0f),CONDITION_SEE_CUR_ENEMY));
				}

				attack->addTask(new CFindPathTask(vGoal));
				attack->addTask(new CBotDODAttackPoint(iFlagID,vGoal,150.0f));
				// add defend task
				m_pSchedules->add(attack);

				// last flag
				//if ( CDODMod::m_Flags.getNumFlagsOwned(m_iTeam) == (CDODMod::m_Flags.getNumFlags()-1) )
				//	addVoiceCommand(DOD_VC_GOGOGO);

				removeCondition(CONDITION_PUSH);
				
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

void CDODBot :: reachedCoverSpot ()
{
	// reached cover
	// dont need to run there any more
	removeCondition(CONDITION_RUN);

	if ( !m_pEnemy.get() && !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
		m_pButtons->tap(IN_RELOAD);
}

bool CDODBot:: checkStuck ()
{
	if ( CBot::checkStuck() )
	{
		CBotWeapon *pWeapon = getCurrentWeapon();

		if ( pWeapon )
		{
			if ( pWeapon->isZoomable() && CClassInterface::isSniperWeaponZoomed(pWeapon->getWeaponEntity()) )
				secondaryAttack();
			else if ( pWeapon->isDeployable() )
			{
				if ( pWeapon->isExplosive() && CClassInterface::isRocketDeployed(pWeapon->getWeaponEntity()) )
					secondaryAttack();
				else if ( !pWeapon->isExplosive() && CClassInterface::isMachineGunDeployed(pWeapon->getWeaponEntity()) )
					secondaryAttack();
			}
		}
	}

	return false;
}

bool CDODBot :: handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy )
{
	static bool bAttack;
	static float fDelay; // delay to reduce recoil

	bAttack = true;
	fDelay = 0;

	if ( pWeapon )
	{
		Vector vEnemyOrigin;

		if ( pWeapon->isExplosive() && (DotProductFromOrigin(m_vAimVector) < 0.98f) ) 
			return true; // keep enemy / don't shoot : until angle between enemy is less than 45 degrees

		clearFailedWeaponSelect();

		if ( pWeapon->isMelee() )
		{
			setMoveTo(CBotGlobals::entityOrigin(pEnemy));
			//setLookAt(m_vAimVector);
			setLookAtTask((LOOK_ENEMY));
			// dontAvoid my enemy
			m_fAvoidTime = engine->Time() + 1.0f;
			fDelay = 0;

			vEnemyOrigin = CBotGlobals::entityOrigin(pEnemy);
	// enemy below me!
			if ( pWeapon->isMelee() && (vEnemyOrigin.z < (getOrigin().z - 8)) && (vEnemyOrigin.z > (getOrigin().z-128))  )
				duck();
		}
		else
		{
			edict_t *pWeaponEdict = pWeapon->getWeaponEntity();

			if ( m_iClip1 == 0 )
			{
				if ( !m_pSchedules->isCurrentSchedule(SCHED_GOOD_HIDE_SPOT) )
				{
					if ( pWeapon->isZoomable() && !CClassInterface::isSniperWeaponZoomed(pWeaponEdict) )
						secondaryAttack();

					m_pSchedules->freeMemory();
					m_pSchedules->add(new CGotoHideSpotSched(pEnemy));
				}

				m_pButtons->tap(IN_RELOAD);

				return false;
			}
			else
			{
				fDelay = randomFloat(0.05f,0.2f);

				if ( !hasSomeConditions(CONDITION_RUN) )
					stopMoving();

				if ( pWeapon->needsDeployedOrZoomed() ) // && pWeapon->getID() ==...
				{
					//stopMoving();

					if ( pWeapon->isZoomable() ) 
					{
						if ( !CClassInterface::isSniperWeaponZoomed(pWeaponEdict) )
							bAttack = false;
						else
							fDelay = randomFloat(0.5f,1.0f);
					}
					else if ( pWeapon->isDeployable() ) 
					{
						if ( pWeapon->isExplosive() )
							bAttack = CClassInterface::isRocketDeployed(pWeaponEdict);
						else
						{
							bAttack = CClassInterface::isMachineGunDeployed(pWeaponEdict);
						}

						if ( !bAttack )
							fDelay = randomFloat(0.7f,1.2f);
						//else
						//	fDelay = 0;
					}

					if ( !bAttack && (m_fZoomOrDeployTime < engine->Time()) )
					{
						secondaryAttack(); // deploy / zoom
						m_fZoomOrDeployTime = engine->Time() + randomFloat(0.5f,1.0f);
					}
				} 
				else if ( pWeapon->isDeployable() )
				{
					if ( !CClassInterface::isMachineGunDeployed(pWeaponEdict) )
					{
						if ( m_fZoomOrDeployTime < engine->Time() )
						{
							secondaryAttack(); // deploy / zoom
							m_fZoomOrDeployTime = engine->Time() + randomFloat(0.5f,1.0f);
						}

						// not deployed for a while -- go prone to deploy
						if ( !hasSomeConditions(CONDITION_RUN) && (m_fDeployMachineGunTime + 1.0f) < engine->Time() )
							m_pButtons->holdButton(IN_ALT1,0,1.0f,0);

						fDelay = randomFloat(0.7f,1.2f);
					}
				}
				else if ( ((pWeapon->getID()==DOD_WEAPON_GARAND)||(pWeapon->getID()==DOD_WEAPON_K98)) && ( distanceFrom(pEnemy) > 1000 ))
				{
					if ( !CClassInterface::isK98Zoomed(pWeaponEdict) && !CClassInterface::isGarandZoomed(pWeaponEdict) )
					{
						if ( m_fZoomOrDeployTime < engine->Time() )
						{
							secondaryAttack(); // deploy / zoom
							m_fZoomOrDeployTime = engine->Time() + randomFloat(0.1f,0.2f);
							fDelay = randomFloat(0.2f,0.4f);
						}
					}
				}
			}
		}

		if ( bAttack && (m_fShootTime < engine->Time()) )
		{
			primaryAttack(); // shoot
			m_fShootTime = engine->Time() + fDelay;
		}
	}
	else
		primaryAttack();

	return true;
}

#define BOT_DEFEND 0
#define BOT_ATTACK 1

void CDODBot :: getTasks (unsigned int iIgnore)
{
	static CBotUtilities utils;
	static CBotUtility *next;
	static CBotWeapon *grenade;
	static CBotWeapon *pWeapon;
	static float fAttackUtil;
	static float fDefendUtil;
	static float fDefRate;
	static int numPlayersOnTeam;
	static int numClassOnTeam;
	static bool bCheckCurrent;
	static int iFlagID;
	static int iFlagsOwned;
	static int iEnemyFlagsOwned;
	static int iNumFlags;
	static int iNumBombsToPlant;
	static int iNumBombsOnMap;
	static int iNumEnemyBombsOnMap;
	static int iNumEnemyBombsStillToPlant;
	static int iNumBombsToDefuse;
	static int iNumBombsToDefend;
	static float fDefendBombUtil;
	static float fDefuseBombUtil;
	static float fPlantUtil; 
	static int iEnemyTeam;
	// if condition has changed or no tasks
	if ( !hasSomeConditions(CONDITION_CHANGED) && !m_pSchedules->isEmpty() )
		return;

	bCheckCurrent = true;
	m_iTeam = getTeam(); // update team

	fAttackUtil = 0.5f;
	fDefendUtil = 0.4f;

	numClassOnTeam = CDODMod::numClassOnTeam(m_iTeam,m_iClass);
	numPlayersOnTeam = CBotGlobals::numPlayersOnTeam(m_iTeam,false);

	pWeapon = NULL;

	removeCondition(CONDITION_CHANGED);

	// always able to roam around -- low utility
	ADD_UTILITY(BOT_UTIL_ROAM,true,0.01f);

	// I had an enemy a minute ago
	ADD_UTILITY(BOT_UTIL_FIND_LAST_ENEMY,wantToFollowEnemy() && !m_bLookedForEnemyLast && m_pLastEnemy && CBotGlobals::entityIsValid(m_pLastEnemy) && CBotGlobals::entityIsAlive(m_pLastEnemy),getHealthPercent()*0.81f);

	// flag capture map
	if ( CDODMod::isFlagMap() && (CDODMod::m_Flags.getNumFlags() > 0) )
	{
		/*
		int iAttackOrDefend; 
		float fAttackRatio;

		float fFlagRatio;
		float fDefRatio;
		iFlagID = -1;

		iFlagsOwned = CDODMod::m_Flags.getNumFlagsOwned(m_iTeam);
		iEnemyFlagsOwned = CDODMod::m_Flags.getNumFlagsOwned(m_iTeam == TEAM_ALLIES ? TEAM_AXIS : TEAM_ALLIES);
		iNumFlags = CDODMod::m_Flags.getNumFlags();
		fDefRate = bot_defrate.GetFloat();

		fFlagRatio = (float)iEnemyFlagsOwned/iNumFlags;
		fDefRatio = (float)iFlagsOwned/iNumFlags;

		fAttackRatio = fFlagRatio 

		if ( m_pNearestFlag )
			iFlagID = CDODMod::m_Flags.getFlagID(m_pNearestFlag);

		fAttackUtil = 0.3f + (randomFloat(0.0f,fFlagRatio)*(m_pProfile->m_fBraveness*0.5f));
		fDefendUtil = 0.3f + (randomFloat(0.0f,fDefRatio)*(0.5f - (m_pProfile->m_fBraveness*0.5f)));
*/
		if ( (m_pNearestFlag==NULL)||CDODMod::m_Flags.ownsFlag(iFlagID,m_iTeam) )
		{
			if ( CDODMod::shouldAttack(m_iTeam) )
			{
				fAttackUtil = 0.6f;
				fDefendUtil = 0.3f;
			}
			else
			{
				fAttackUtil = 0.3f;
				fDefendUtil = 0.5f;
			}

			if ( hasSomeConditions(CONDITION_PUSH) )
				fAttackUtil *= 2;

			// if we see a flag and we own it, don't worry about it
			ADD_UTILITY(BOT_UTIL_ATTACK_POINT,true,fAttackUtil);
			ADD_UTILITY(BOT_UTIL_DEFEND_POINT,true,fDefendUtil);
		}

		if ( m_pNearestFlag )
		{
			iFlagID = CDODMod::m_Flags.getFlagID(m_pNearestFlag);

			// attack the flag if I've reached the last one
			ADD_UTILITY_DATA_VECTOR(BOT_UTIL_ATTACK_NEAREST_POINT,
				!CDODMod::m_Flags.ownsFlag(iFlagID,m_iTeam) && (CDODMod::m_Flags.numCappersRequired(iFlagID,m_iTeam)-
				CDODMod::m_Flags.numFriendliesAtCap(iFlagID,m_iTeam))<=1,(iFlagsOwned == (iNumFlags-1)) ? 0.9f : 0.75f,iFlagID,CBotGlobals::entityOrigin(m_pNearestFlag));

			ADD_UTILITY_DATA_VECTOR(BOT_UTIL_DEFEND_NEAREST_POINT,
				CDODMod::m_Flags.ownsFlag(iFlagID,m_iTeam) && ((CDODMod::m_Flags.numEnemiesAtCap(iFlagID,m_iTeam)>0)||hasEnemy()),
				(iFlagsOwned == 1) ? 0.9f : 0.75f,iFlagID,CBotGlobals::entityOrigin(m_pNearestFlag));

		}
	}
	// bomb map
	else if ( CDODMod::isBombMap() && (CDODMod::m_Flags.getNumFlags() > 0) )
	{
		// same thing as above except with bombs
		iFlagID = -1;
		iEnemyTeam = m_iTeam==TEAM_ALLIES ? TEAM_AXIS : TEAM_ALLIES;
		iFlagsOwned = CDODMod::m_Flags.getNumFlagsOwned(m_iTeam);
		iNumFlags = CDODMod::m_Flags.getNumFlags();

		iNumEnemyBombsOnMap = CDODMod::m_Flags.getNumBombsOnMap(iEnemyTeam);
		iNumBombsOnMap = CDODMod::m_Flags.getNumBombsOnMap(m_iTeam);
		iNumBombsToPlant = CDODMod::m_Flags.getNumBombsToPlant(m_iTeam);
		iNumBombsToDefuse = CDODMod::m_Flags.getNumBombsToDefuse(m_iTeam);
		iNumBombsToDefend = CDODMod::m_Flags.getNumBombsToDefend(m_iTeam);
		iNumEnemyBombsStillToPlant = CDODMod::m_Flags.getNumBombsToPlant(iEnemyTeam);

		// different defend util here
		fDefendUtil = 0.8f - ((float)iNumEnemyBombsStillToPlant/iNumEnemyBombsOnMap)*0.4f;

		fPlantUtil = 0.4f + (((float)iNumBombsToPlant/iNumBombsOnMap)*0.4f);
		fDefuseBombUtil = fDefendUtil * 2;
		fDefendBombUtil = 0.8f - (((float)iNumBombsToDefend/iNumBombsOnMap)*0.8f);

		
		fPlantUtil += randomFloat(-0.25f,0.25f); // add some fuzz
		fAttackUtil = fPlantUtil;
		fDefRate = bot_defrate.GetFloat();
		fDefendUtil += randomFloat(-fDefRate,fDefRate);

		// bot is ... go go go!
		if ( hasSomeConditions(CONDITION_PUSH) )
		{
			fPlantUtil *= 2.0f;
			fDefuseBombUtil *= 2.0f;
			fDefendBombUtil *= 0.75f;
			fDefendUtil *= 0.75f;
		}

		ADD_UTILITY(BOT_UTIL_PLANT_BOMB,m_bHasBomb && (iNumBombsToPlant>0),fPlantUtil );
		ADD_UTILITY(BOT_UTIL_DEFUSE_BOMB,(iNumBombsToDefuse>0), fDefuseBombUtil);
		ADD_UTILITY(BOT_UTIL_DEFEND_BOMB,(iNumBombsToDefend>0), fDefendBombUtil);
		ADD_UTILITY(BOT_UTIL_PICKUP_BOMB,!m_bHasBomb && (iNumBombsToPlant>0),fPlantUtil);

		if ( iNumEnemyBombsOnMap > 0 )
		{
			ADD_UTILITY(BOT_UTIL_DEFEND_POINT,(iFlagsOwned>0)&&(m_pNearestFlag==NULL)||CDODMod::m_Flags.ownsFlag(iFlagID,m_iTeam),fDefendUtil);

			/*if ( m_pNearestFlag )
			{
				// attack the flag if I've reached the last one
				ADD_UTILITY_DATA_VECTOR(BOT_UTIL_ATTACK_NEAREST_POINT,
					!CDODMod::m_Flags.ownsFlag(iFlagID,m_iTeam) && (CDODMod::m_Flags.numCappersRequired(iFlagID,m_iTeam)-
					CDODMod::m_Flags.numFriendliesAtCap(iFlagID,m_iTeam))<=1,(iFlagsOwned == (iNumFlags-1)) ? 0.9f : 0.75f,iFlagID,CBotGlobals::entityOrigin(m_pNearestFlag));
			}*/
		}

		if ( m_pNearestBomb )
		{
			Vector vBomb = CBotGlobals::entityOrigin(m_pNearestBomb);

			iFlagID = CDODMod::m_Flags.getBombID(m_pNearestBomb);

			// attack the flag if I've reached the last one
			ADD_UTILITY_DATA_VECTOR(BOT_UTIL_PLANT_NEAREST_BOMB,
				CDODMod::m_Flags.canPlantBomb(m_iTeam,iFlagID),fPlantUtil+randomFloat(-0.05f,0.1f),iFlagID,vBomb);
// attack the flag if I've reached the last one
			ADD_UTILITY_DATA_VECTOR(BOT_UTIL_DEFEND_NEAREST_BOMB,
				CDODMod::m_Flags.canDefendBomb(m_iTeam,iFlagID),fDefendBombUtil+randomFloat(-0.05f,0.1f),iFlagID,vBomb);
// attack the flag if I've reached the last one
			ADD_UTILITY_DATA_VECTOR(BOT_UTIL_DEFUSE_NEAREST_BOMB,
				CDODMod::m_Flags.canDefuseBomb(m_iTeam,iFlagID),fDefuseBombUtil+randomFloat(-0.05f,0.1f),iFlagID,vBomb);

		}
	}
	else
	{
		ADD_UTILITY(BOT_UTIL_MESSAROUND,(getHealthPercent()>0.75f), fAttackUtil );
	}

	// sniping or machinegunning
	switch ( m_iClass )
	{
	case DOD_CLASS_SNIPER:
		{
			pWeapon = m_pWeapons->hasWeapon(DOD_WEAPON_K98_SCOPED) ? m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_K98_SCOPED)) : m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_SPRING));

			if ( pWeapon )
				ADD_UTILITY_WEAPON_DATA(BOT_UTIL_SNIPE,pWeapon->getAmmo(this)>0,getHealthPercent() * randomFloat(0.75f,1.0f),pWeapon,fAttackUtil>fDefendUtil);
		}
		break;
	case DOD_CLASS_MACHINEGUNNER:
		{
			pWeapon = m_pWeapons->hasWeapon(DOD_WEAPON_20CAL) ? m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_20CAL)) : m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_MG42));

			if ( pWeapon )
				ADD_UTILITY_WEAPON_DATA(BOT_UTIL_SNIPE,pWeapon->getAmmo(this)>0,getHealthPercent(),pWeapon,fAttackUtil>fDefendUtil);
		}
		break;
	}

	// grenades
	if ( !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && hasSomeConditions(CONDITION_SEE_LAST_ENEMY_POS) && m_pLastEnemy && m_fLastSeeEnemy && ((m_fLastSeeEnemy + 10.0) > engine->Time()) && 
		(m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_US) || m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_GER) || m_pWeapons->hasWeapon(DOD_WEAPON_SMOKE_US) || m_pWeapons->hasWeapon(DOD_WEAPON_SMOKE_GER)) )
	{
		float fDistance = distanceFrom(m_vLastSeeEnemyBlastWaypoint);
		float fGrenUtil =  0.85f + ( (1.0f - getHealthPercent()) * 0.15f);

		CBotWeapon *pBotWeapon = NULL;
		CBotWeapon *pBotSmokeGren = m_pWeapons->hasWeapon(DOD_WEAPON_SMOKE_US) ? m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_SMOKE_US)) : m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_SMOKE_GER));

		if ( (hasSomeConditions(CONDITION_COVERT)||(m_fCurrentDanger >= 25.0f)) && pBotSmokeGren && pBotSmokeGren->hasWeapon() )
			pBotWeapon = pBotSmokeGren;
		else if ( m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_US) )
			pBotWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_FRAG_US));
		else if ( m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_GER) )
			pBotWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_FRAG_GER));
				
		// if within throw distance and outside balst radius, I can throw it
		if ( pBotWeapon && (!pBotWeapon->isExplosive() || (fDistance > BLAST_RADIUS)) && ( fDistance < 1500 ) )
		{
			ADD_UTILITY_WEAPON(BOT_UTIL_THROW_GRENADE, pBotWeapon && (pBotWeapon->getAmmo(this) > 0) ,hasSomeConditions(CONDITION_GREN) ? fGrenUtil*2 : fGrenUtil,pBotWeapon);
		}
	}


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
		if ( !m_pSchedules->isEmpty() && bCheckCurrent )
		{
			if ( m_CurrentUtil != next->getId() )
				m_pSchedules->freeMemory();
			else
				break;
		} 

		bCheckCurrent = false;

		if ( executeAction(next) )
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

	sprintf(cmd,"use %s\n",pWeapon->getWeaponName());

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

		sprintf(cmd,"use %s\n",pSelect->getWeaponName());

		helpers->ClientCommand(m_pEdict,cmd);

		return true;
	}
	else
		failWeaponSelect();

	return false;
}

Vector CDODBot :: getAimVector ( edict_t *pEntity )
{
	static Vector vAim;
	static short int iSlot;
	static smoke_t *smokeinfo;
	static bool bProne;
	static float fStamina;

	vAim = CBot::getAimVector(pEntity);

	// for some reason, prone does not change collidable size
	CClassInterface::getPlayerInfoDOD(pEntity,&bProne,&fStamina);
	// .. so update 
	if ( bProne )
	{
		vAim.z = vAim.z - randomFloat(0.0,8.0f);
	}

	if ( m_pNearestSmokeToEnemy )
	{
		iSlot = ENTINDEX(pEntity)-1;

		if (( iSlot >= 0 ) && ( iSlot < MAX_PLAYERS ))
		{
			smokeinfo = &(m_CheckSmoke[iSlot]);

			if ( smokeinfo->bInSmoke ) 
			{
				return vAim = vAim + Vector(randomFloat(-SMOKE_RADIUS,SMOKE_RADIUS)*smokeinfo->fProb,randomFloat(-SMOKE_RADIUS,SMOKE_RADIUS)*smokeinfo->fProb,randomFloat(-SMOKE_RADIUS,SMOKE_RADIUS)*smokeinfo->fProb);
			}
		}
	}

	return vAim;
}

bool CDODBot :: isVisibleThroughSmoke ( edict_t *pSmoke, edict_t *pCheck )
{
	//if ( isVisible(pCheck) )
//{
	static float fSmokeDist,fDist;
	static smoke_t *smokeinfo;
	static float fTime, fProb;
	static Vector vSmoke;
	static Vector vCheckComp;

	static short int iSlot;

	iSlot = ENTINDEX(pCheck)-1;

	// if pCheck is a player
	if (( iSlot >= 0 ) && ( iSlot < MAX_PLAYERS ))
	{
		smokeinfo = &(m_CheckSmoke[iSlot]);

		// last time i checked was long enough ago
		if ( smokeinfo->fLastTime < engine->Time() )
		{
			smokeinfo->bVisible = true;
			smokeinfo->bInSmoke = false;
			//fTime = gpGlobals->curtime - CClassInterface::getSmokeSpawnTime(pSmoke);

			//if ( (fTime > 1.0f) && (fTime < 10.0f) )
			//{
				vSmoke = CBotGlobals::entityOrigin(pSmoke);
				fSmokeDist = distanceFrom(vSmoke);

				// I'm outside smoke -- but maybe the enemy is inside the smoke
				if ( fSmokeDist > SMOKE_RADIUS )
				{
					fDist = (CBotGlobals::entityOrigin(pCheck) - vSmoke).Length();

					// enemy outside the smoke radius
					if ( fDist > SMOKE_RADIUS )
					{
						// check if enemy is behind the smoke from my perspective
						vCheckComp = CBotGlobals::entityOrigin(pCheck) - getOrigin();
						vCheckComp = (vCheckComp / vCheckComp.Length())*fSmokeDist;

						fDist = (vSmoke - (getOrigin() + vCheckComp)).Length();

						//if ( fDist > SMOKE_RADIUS )
						// visible
					}
				}
				else
					fDist = fSmokeDist;

				if ( fDist <= SMOKE_RADIUS ) 
					// smoke gets heavy at 1.0 seconds and diminishes at 10 secs
				{
					smokeinfo->fProb = 1.0f-(fDist/SMOKE_RADIUS);
					smokeinfo->bVisible = (randomFloat(0.0f,0.33f) > smokeinfo->fProb ); 
					// smoke gets pretty heavy half way into the smoke grenade
					smokeinfo->bInSmoke = true;
				}
				
					
			//}
			//else
			//	smokeinfo->bInSmoke = false;

			#ifdef _DEBUG
				if ( CClients::clientsDebugging(BOT_DEBUG_THINK) )
					CClients::clientDebugMsg(this,BOT_DEBUG_THINK,"Smoke Test (%s to %s) = %s",m_szBotName,engine->GetPlayerNetworkIDString(pCheck),smokeinfo->bVisible ? "visible" : "invisible");
			#endif

		}

		// check again soon (typical reaction time delay)
		smokeinfo->fLastTime = engine->Time() + randomFloat(0.15f,0.3f);

		return smokeinfo->bVisible;

	}

	return true;
//}

	//return false;
}