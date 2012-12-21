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

	CBot::setVisible(pEntity,bVisible);

	if ( bVisible && !(CClassInterface::getEffects(pEntity) & EF_NODRAW) )
	{
		szClassname = pEntity->GetClassName();

		if ( (m_pNearestFlag != pEntity) && CDODMod::m_Flags.isFlag(pEntity) && 
			(!m_pNearestFlag || (distanceFrom(pEntity)<distanceFrom(m_pNearestFlag)) ) )
		{
			m_pNearestFlag = pEntity;
		}
		else if (CClassInterface::getTeam(pEntity) == m_iEnemyTeam) 
		{
			if ( (pEntity!=m_pEnemyGrenade) && (strncmp(szClassname,"grenade",7) == 0 ) && (!m_pEnemyGrenade || (distanceFrom(pEntity)<distanceFrom(m_pEnemyGrenade))))
				m_pEnemyGrenade = pEntity;
			else if ( (pEntity!=m_pEnemyRocket) && (strncmp(szClassname,"rocket",6) == 0 ) && (!m_pEnemyRocket || (distanceFrom(pEntity)<distanceFrom(m_pEnemyRocket))))
				m_pEnemyRocket = pEntity;
		}
	}
	else
	{
		if ( pEntity == m_pNearestFlag )
			m_pNearestFlag = NULL;
		else if ( pEntity == m_pEnemyGrenade )
			m_pEnemyGrenade = NULL;
		else if ( pEntity == m_pEnemyRocket )
			m_pEnemyRocket = NULL;
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
		if ( (m_iDesiredTeam == 2) || (m_iDesiredTeam == 3) )
			m_pPlayerInfo->ChangeTeam(m_iDesiredTeam);
		else
			m_pPlayerInfo->ChangeTeam(randomInt(2,3));

		return false;

	}

	//if ( (m_iDesiredClass >= 0) && (m_iDesiredClass <= 5) && (m_iDesiredClass != CClassInterface::getPlayerClassDOD(m_pEdict)) )
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
		m_pNavigator->belief(getOrigin(),getOrigin(),bot_beliefmulti.GetFloat(),distanceFrom(pVictim),BELIEF_SAFETY);

	return;
}

void CDODBot :: died ( edict_t *pKiller )
{
	spawnInit();

	if ( randomInt(0,1) )
		m_pButtons->attack();

	if ( pKiller )
	{
		if ( CBotGlobals::entityIsValid(pKiller) )
			m_pNavigator->belief(getOrigin(),getOrigin(),bot_beliefmulti.GetFloat(),distanceFrom(pKiller),BELIEF_DANGER);
	}

}

void CDODBot :: seeFriendlyDie ( edict_t *pDied, edict_t *pKiller, CWeapon *pWeapon )
{
	if ( pKiller && !m_pEnemy && !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{
		if ( pWeapon )
		{
			DOD_Class pclass = (DOD_Class)CClassInterface::getPlayerClassDOD(pKiller);

			if ( (pclass == DOD_CLASS_SNIPER) && pWeapon->isZoomable() )
			{
				voiceCommand(DOD_VC_SNIPER);
				m_fCurrentDanger += 100.0f;
			}
			else if ( (pclass == DOD_CLASS_MACHINEGUNNER) && pWeapon->isDeployable() )
			{
				voiceCommand(DOD_VC_MGAHEAD);
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

		CWaypoint *pWpt = CWaypoints::getWaypoint(CWaypointLocations::NearestBlastWaypoint(m_vLastSeeEnemy,getOrigin(),1024.0,-1,true,true,false,false,0,false));
			
		if ( pWpt )
			m_vLastSeeEnemyBlastWaypoint = pWpt->getOrigin();

		updateCondition(CONDITION_CHANGED);
	}
}

void CDODBot :: spawnInit ()
{
	CBot::spawnInit();

	m_pNearestFlag = NULL;

	m_fShoutRocket = 0;
	m_fShoutGrenade = 0;
	m_pEnemyRocket = NULL;
	m_pEnemyGrenade = NULL;

	m_fShootTime = 0;
	m_fProneTime = 0;
	m_fZoomOrDeployTime = 0;

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

	return true;	
}

void CDODBot :: handleWeapons ()
{
	//
	// Handle attacking at this point
	//
	if ( m_pEnemy && !hasSomeConditions(CONDITION_ENEMY_DEAD) && 
		hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && wantToShoot() && 
		isVisible(m_pEnemy) && isEnemy(m_pEnemy) )
	{
		CBotWeapon *pWeapon;

		setMoveLookPriority(MOVELOOK_ATTACK);

		pWeapon = getBestWeapon(m_pEnemy);

		if ( m_bWantToChangeWeapon && (pWeapon != NULL) && (pWeapon != getCurrentWeapon()) && pWeapon->getWeaponIndex() )
		{
			//selectWeaponSlot(pWeapon->getWeaponInfo()->getSlot());
			selectBotWeapon(pWeapon);
		}

		setLookAtTask((LOOK_ENEMY));

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

	if ( m_pEnemy && hasSomeConditions(CONDITION_SEE_CUR_ENEMY) ) 
	{
		m_pNavigator->beliefOne(wptindex,BELIEF_DANGER,distanceFrom(m_pEnemy));
	}
	else
		m_pNavigator->beliefOne(wptindex,BELIEF_SAFETY,0);
}

void CDODBot :: modThink ()
{
	static float fMaxSpeed;

	fMaxSpeed = CClassInterface::getMaxSpeed(m_pEdict);

	m_fIdealMoveSpeed = fMaxSpeed/2;
	
	m_iClass = (DOD_Class)CClassInterface::getPlayerClassDOD(m_pEdict);	
	m_iTeam = getTeam();

	if ( m_iTeam == TEAM_ALLIES ) // im allies
		m_iEnemyTeam = TEAM_AXIS;
	else if ( m_iEnemyTeam == TEAM_AXIS ) // im axis and my enemy team is wrong
		m_iEnemyTeam = TEAM_ALLIES;

	// find weapons and neat stuff
	if ( m_fFixWeaponTime < engine->Time() )
	{
		fixWeapons();
		m_pCurrentWeapon = CClassInterface::getCurrentWeapon(m_pEdict);
		m_fFixWeaponTime = engine->Time() + 1.0f;
	}

	if ( m_pCurrentWeapon )
		CClassInterface::getWeaponClip(m_pCurrentWeapon,&m_iClip1,&m_iClip2);

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
		if ( !m_bProne && ( m_fProneTime < engine->Time() ))
		{
			m_pButtons->tap(IN_ALT1);
			m_fProneTime = engine->Time() + randomFloat(4.0f,8.0f);
		}

		m_fIdealMoveSpeed = fMaxSpeed/4;
	}
	else if ( (m_fCurrentDanger >= 20.0f) && (m_flStamina > 90.f ) && (m_flSprintTime < engine->Time()))
	{
		if ( m_bProne && ( m_fProneTime < engine->Time() ))
		{
			m_pButtons->tap(IN_ALT1);
			m_fProneTime = engine->Time() + randomFloat(4.0f,8.0f);
		}

		m_fIdealMoveSpeed = fMaxSpeed;
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

	if ( m_pEnemyRocket )
	{
		m_fShoutRocket = m_fShoutRocket/2 + 0.5f;

		if ( m_fShoutRocket > 0.95f ) 
		{
			voiceCommand(DOD_VC_BAZOOKA);
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
			voiceCommand(DOD_VC_GRENADE2);
			m_fShoutGrenade = 0.0f;

			if ( !m_pSchedules->isCurrentSchedule(SCHED_GOOD_HIDE_SPOT) )
			{
				// don't interrupt current shedule, just add to front
				m_pSchedules->removeSchedule(SCHED_GOOD_HIDE_SPOT);
				m_pSchedules->addFront(new CGotoHideSpotSched(m_pEnemyGrenade));
			}
		}
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
					else if ( pHasWeapon && (pHasWeapon->getWeaponEntity() != pWeapon) )
						pHasWeapon->setWeaponEntity(pWeapon);
				}
			}
		}
	}
}

void CDODBot ::defending()
{
	// check to go prone or not
}

void CDODBot ::voiceCommand ( eDODVoiceCMD cmd )
{
	// find voice command
	extern ConVar bot_use_vc_commands;
	extern eDODVoiceCommand_t g_DODVoiceCommands[DOD_VC_INVALID];

	if ( bot_use_vc_commands.GetBool() && (m_fLastVoiceCommand < engine->Time() ))
	{
		char scmd[64];
		u_VOICECMD vcmd;

		vcmd.voicecmd = cmd;
		
		sprintf(scmd,"voice_%s",g_DODVoiceCommands[cmd].pcmd);

		helpers->ClientCommand(m_pEdict,scmd);

		m_fLastVoiceCommand = engine->Time() + randomFloat(10.0f,15.0f);
	}
}

void CDODBot :: hearVoiceCommand ( edict_t *pPlayer, byte cmd )
{
	switch ( cmd )
	{
	case DOD_VC_GOGOGO:
		updateCondition(CONDITION_PUSH);
		break;
	case DOD_VC_HOLD:
		removeCondition(CONDITION_PUSH);
		break;
	case DOD_VC_GRENADE2:
	case DOD_VC_BAZOOKA:
		// if I don't see anythign but I see the player calling this, hide!
		if ( !m_pEnemyGrenade && !m_pEnemyRocket && isVisible(pPlayer) && !m_pSchedules->isCurrentSchedule(SCHED_GOOD_HIDE_SPOT) )
		{
			// don't interrupt current shedule, just add to front
			m_pSchedules->removeSchedule(SCHED_GOOD_HIDE_SPOT);
			m_pSchedules->addFront(new CGotoHideSpotSched(pPlayer));
		}
		break;
	case DOD_VC_NEED_BACKUP:
		if ( m_pNearestFlag && isVisible(pPlayer) )
		{
			Vector vPoint = CBotGlobals::entityOrigin(m_pNearestFlag);
			if ( (vPoint - CBotGlobals::entityOrigin(pPlayer)).Length() < 250 )
			{
					CBotSchedule *attack = new CBotSchedule();

					attack->setID(SCHED_ATTACKPOINT);
					attack->addTask(new CFindPathTask(vPoint));
					attack->addTask(new CBotDODAttackPoint(CDODMod::m_Flags.getFlagID(m_pNearestFlag),vPoint,150.0f));
					// add defend task
					m_pSchedules->freeMemory();
					m_pSchedules->add(attack);
			}
		}
		break;
	default:
		break;
	}
}

bool CDODBot :: executeAction ( CBotUtility *util )
{
	switch ( util->getId() )
	{
	case BOT_UTIL_ATTACK_NEAREST_POINT:
		{
			Vector vGoal;
			int iFlagID = util->getIntData();

			vGoal = util->getVectorData();

			CBotSchedule *attack = new CBotSchedule();

			attack->setID(SCHED_ATTACKPOINT);
			attack->addTask(new CFindPathTask(vGoal));
			attack->addTask(new CBotDODAttackPoint(iFlagID,vGoal,150.0f));
			// add defend task
			m_pSchedules->add(attack);
			removeCondition(CONDITION_PUSH);
			
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
				if ( CDODMod::m_Flags.getRandomEnemyControlledFlag(&vGoal,getTeam(),&iFlagID) )
				{
					if ( m_iClass == DOD_CLASS_MACHINEGUNNER )
						pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_MACHINEGUN,m_iTeam,iFlagID,true,this);
					else
						pWaypoint = CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SNIPER,m_iTeam,iFlagID,true,this);
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

				snipe->setID(SCHED_DEFENDPOINT);
				snipe->addTask(new CFindPathTask(pWaypoint->getOrigin()));
				snipe->addTask(new CBotDODSnipe(util->getWeaponChoice(),pWaypoint->getOrigin(),pWaypoint->getAimYaw(),iFlagID!=-1,vGoal.z+48));
				
				m_pSchedules->add(snipe);

				return true;
			}
		}
		break;
	case BOT_UTIL_DEFEND_POINT:
		{
			Vector vGoal;

			if ( CDODMod::m_Flags.getRandomTeamControlledFlag(&vGoal,getTeam()) )
			{
				CWaypoint *pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),vGoal);

				if ( pWaypoint )
				{
					CBotSchedule *defend = new CBotSchedule();

					defend->setID(SCHED_DEFENDPOINT);
					defend->addTask(new CFindPathTask(pWaypoint->getOrigin()));
					defend->addTask(new CBotDefendTask(pWaypoint->getOrigin(),randomFloat(7.5f,12.5f),0,true,vGoal));
					// add defend task
					m_pSchedules->add(defend);
					
					return true;
				}
			}
		}
		break;
	case BOT_UTIL_FIND_LAST_ENEMY:
		{
			Vector vVelocity = Vector(0,0,0);
			CClient *pClient = CClients::get(m_pLastEnemy);
			CBotSchedule *pSchedule = new CBotSchedule();
			
			CFindPathTask *pFindPath = new CFindPathTask(m_vLastSeeEnemy);	

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

				pSched->addTask(pathtask); // 2nd -- hide
				pSched->addTask(new CThrowGrenadeTask(pWeapon,getAmmo(pWeapon->getWeaponInfo()->getAmmoIndex1()),m_vLastSeeEnemyBlastWaypoint)); // first - throw

				m_pSchedules->add(pSched);
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
				CWaypoint *pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),vGoal);

				if ( pWaypoint )
				{
					CBotSchedule *attack = new CBotSchedule();

					attack->setID(SCHED_ATTACKPOINT);
					attack->addTask(new CFindPathTask(pWaypoint->getOrigin()));
					attack->addTask(new CBotDefendTask(pWaypoint->getOrigin(),randomFloat(1.0f,3.0f),0,true,vGoal));
					attack->addTask(new CFindPathTask(vGoal));
					attack->addTask(new CBotDODAttackPoint(iFlagID,vGoal,150.0f));
					// add defend task
					m_pSchedules->add(attack);

					removeCondition(CONDITION_PUSH);
					
					return true;
				}
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
	extern ConVar rcbot_enemyshootfov;

	if ( DotProductFromOrigin(m_vAimVector) < rcbot_enemyshootfov.GetFloat() ) 
		return true; // keep enemy / don't shoot : until angle between enemy is less than 45 degrees

	bAttack = true;
	fDelay = 0;

	if ( pWeapon )
	{
		Vector vEnemyOrigin;

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
				stopMoving();

				fDelay = randomFloat(0.1f,0.3f);

				if ( pWeapon->needsDeployedOrZoomed() ) // && pWeapon->getID() ==...
				{
					//stopMoving();

					if ( pWeapon->isZoomable() && !CClassInterface::isSniperWeaponZoomed(pWeaponEdict) )
					{
						bAttack = false;
					}
					else if ( pWeapon->isDeployable() ) 
					{
						if ( pWeapon->isExplosive() )
							bAttack = CClassInterface::isRocketDeployed(pWeaponEdict);
						else
							bAttack = CClassInterface::isMachineGunDeployed(pWeaponEdict);
					}

					if ( bAttack )
						fDelay = randomFloat(0.5f,1.0f);
					else
					{
						if ( m_fZoomOrDeployTime < engine->Time() )
						{
							secondaryAttack(); // deploy / zoom
							m_fZoomOrDeployTime = engine->Time() + randomFloat(0.5f,1.0f);
						}

						fDelay = randomFloat(0.7f,1.2f);
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

void CDODBot :: getTasks (unsigned int iIgnore)
{
	//CBotUtilities util;
	//CBot::getTasks(iIgnore);

	//ADD_UTILITY(BOT_UTIL_CAPTURE_POINT,,(numFriendlies/numEnemies)/numPlayersOnTeam);
	//ADD_UTILITY();

	static CBotUtilities utils;
	static CBotUtility *next;
	static CBotWeapon *grenade;
	static CBotWeapon *pWeapon;
	static float fAttackUtil;
	static float fDefendUtil;
	static float fDefRate;
	static int numPlayersOnTeam;
	static int numClassOnTeam;

	extern ConVar bot_defrate;

	// if condition has changed or no tasks
	if ( !hasSomeConditions(CONDITION_CHANGED) && !m_pSchedules->isEmpty() )
		return;

	fAttackUtil = 0.5f;
	fDefendUtil = 0.4f;

	numClassOnTeam = CDODMod::numClassOnTeam(m_iTeam,m_iClass);
	numPlayersOnTeam = CBotGlobals::numPlayersOnTeam(m_iTeam,false);

	pWeapon = NULL;

	removeCondition(CONDITION_CHANGED);

	// always able to roam around -- low utility
	ADD_UTILITY(BOT_UTIL_ROAM,true,0.01f);

	// I have an enemy 
	ADD_UTILITY(BOT_UTIL_FIND_LAST_ENEMY,wantToFollowEnemy() && !m_bLookedForEnemyLast && m_pLastEnemy && CBotGlobals::entityIsValid(m_pLastEnemy) && CBotGlobals::entityIsAlive(m_pLastEnemy),getHealthPercent()*(getArmorPercent()+0.1));

	if ( CDODMod::m_Flags.getNumFlags() > 0 )
	{
		static int iFlagID = -1;

		if ( m_pNearestFlag )
			iFlagID = CDODMod::m_Flags.getFlagID(m_pNearestFlag);

		fDefendUtil = (float)CDODMod::m_Flags.getNumFlagsOwned(m_iTeam)/CDODMod::m_Flags.getNumFlags();
		fAttackUtil = 1.0f - fDefendUtil;

		fAttackUtil += randomFloat(-0.2f,0.2f);

		if ( hasSomeConditions(CONDITION_PUSH) )
			fAttackUtil *= 2;

		fDefRate = bot_defrate.GetFloat();
		fDefendUtil += randomFloat(-fDefRate,fDefRate);
		
		ADD_UTILITY(BOT_UTIL_ATTACK_POINT,(m_pNearestFlag==NULL)||!CDODMod::m_Flags.ownsFlag(iFlagID,m_iTeam),fAttackUtil);
		ADD_UTILITY(BOT_UTIL_DEFEND_POINT,(m_pNearestFlag==NULL)||CDODMod::m_Flags.ownsFlag(iFlagID,m_iTeam),fDefendUtil);

		if ( m_pNearestFlag )
		{
			ADD_UTILITY_DATA_VECTOR(BOT_UTIL_ATTACK_NEAREST_POINT,
				(CDODMod::m_Flags.numCappersRequired(iFlagID,m_iTeam)-
				CDODMod::m_Flags.numFriendliesAtCap(iFlagID,m_iTeam))<=1,1.0f,iFlagID,CBotGlobals::entityOrigin(m_pNearestFlag));
		}
	}

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

	if ( !hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && hasSomeConditions(CONDITION_SEE_LAST_ENEMY_POS) && m_pLastEnemy && m_fLastSeeEnemy && ((m_fLastSeeEnemy + 10.0) > engine->Time()) && (m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_US) || m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_GER)) )
	{
		float fDistance = distanceFrom(m_vLastSeeEnemyBlastWaypoint);

		if ( ( fDistance > BLAST_RADIUS ) && ( fDistance < 1500 ) )
		{
			CBotWeapon *pBotWeapon = NULL;

			if ( m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_US) )
				pBotWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_FRAG_US));
			else if ( m_pWeapons->hasWeapon(DOD_WEAPON_FRAG_GER) )
				pBotWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_FRAG_GER));

			ADD_UTILITY_WEAPON(BOT_UTIL_THROW_GRENADE, pBotWeapon && (pBotWeapon->getAmmo(this) > 0) ,1.0f-(getHealthPercent()*0.2),pBotWeapon);
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
		if ( !m_pSchedules->isEmpty() && (m_CurrentUtil != next->getId() ) )
			m_pSchedules->freeMemory();

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
