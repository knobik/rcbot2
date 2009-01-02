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

#include "vstdlib/random.h" // for random functions


void CBroadcastFlagDropped :: execute ( CBot *pBot )
{
	if ( pBot->getTeam() == m_iTeam )
		((CBotTF2*)pBot)->flagDropped(m_vOrigin);
}

void CBroadcastFlagCaptured :: execute ( CBot *pBot )
{
	if ( pBot->getTeam() == m_iTeam )
		((CBotTF2*)pBot)->flagReset();
}

CBotFortress :: CBotFortress()
{ 
	CBot(); 
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
	if ( m_pPlayerInfo->GetTeamIndex() == 0 )
	{
		selectTeam();
	}
	else if ( m_iClass == TF_CLASS_UNDEFINED )
	{
		selectClass();
	}
	else
		return true;

	return false;
}

void CBotFortress :: checkHealingValid ()
{
	if ( m_pHeal )
	{
		if ( !CBotGlobals::entityIsValid(m_pHeal) || !CBotGlobals::entityIsAlive(m_pHeal) )
			m_pHeal = NULL;
		else if ( !isVisible(m_pHeal) )
			m_pHeal = NULL;
	}
}

void CBotFortress :: setVisible ( edict_t *pEntity, bool bVisible )
{
	if ( CTeamFortress2Mod::isFlag(pEntity,getTeam()) )
	{
		if ( bVisible )
			m_pFlag = pEntity;
		else
			m_pFlag = NULL;
	}

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
						if ( m_pHeal )
						{
							if ( m_pHeal != pEntity )
							{
								IPlayerInfo *p1 = playerinfomanager->GetPlayerInfo(m_pHeal);
								IPlayerInfo *p2 = playerinfomanager->GetPlayerInfo(pEntity);

								if ( ((float)p2->GetHealth()/p2->GetMaxHealth()) < ((float)p1->GetHealth()/p1->GetMaxHealth()) )
									m_pHeal = pEntity;
							}
						}
						else
							m_pHeal = pEntity;
					}
				}
			}
		}else if ( m_pHeal == pEntity )
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

	if ( RandomInt(0,1) )
		m_pButtons->attack();
}

void CBotFortress :: spawnInit ()
{
	CBot::spawnInit();
}

void CBotFortress :: setClass ( TF_Class _class )
{
	m_iClass = _class;
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

	checkHealingValid();
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
	TF_Class _class;

	if ( m_iDesiredClass == 0 )
		_class = (TF_Class)RandomInt(1,9);
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

/////////////////////////////////////////////////////////////////////////////
// TEAM FORTRESS 2

void CBotTF2 :: spawnInit()
{
	CBotFortress::spawnInit();

	m_pHeal = NULL;

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
		m_fTauntTime = engine->Time() + RandomFloat(40.0,100.0);
		m_fTaunting = engine->Time() + 4.0;
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

}

void CBotTF2 :: killed ( edict_t *pVictim )
{
	taunt();
}

void CBotTF2 :: capturedFlag ()
{
	taunt();
}

void CBotTF2 :: capturedPoint ()
{
	taunt();
}

void CBotTF2 :: spyDisguise ( int iTeam, int iClass )
{
	char cmd[256];

	sprintf(cmd,"disguise %d %d",iClass,iTeam);

	helpers->ClientCommand(m_pEdict,cmd);
}
// Test
bool CBotTF2 :: isCloaked ()
{
	return m_pController->IsEFlagSet(EF_NODRAW);
}
// Test
bool CBotTF2 :: isDisguised ()
{
	return (m_pEdict->GetIServerEntity()->GetModelIndex() != 0);
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
}

void CBotFortress :: callMedic ()
{
	helpers->ClientCommand (m_pEdict,"saveme");
}

bool CBotFortress :: canGotoWaypoint (Vector vPrevWaypoint, CWaypoint *pWaypoint)
{
	if ( CBot::canGotoWaypoint(vPrevWaypoint,pWaypoint) )
	{
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
	// mod specific think code here
	CBotFortress :: modThink();

	if ( getClass() == TF_CLASS_ENGINEER )
		checkBuildingsValid();

	if ( m_fTaunting > engine->Time() )
		stopMoving(3);

	m_fIdealMoveSpeed = CTeamFortress2Mod::TF2_GetPlayerSpeed(m_pEdict,m_iClass);
}

void CBotTF2 :: getTasks ( unsigned int iIgnore )
{
	// look for tasks
	if ( !hasFlag() && m_pHeal && (m_iClass == TF_CLASS_MEDIC) )
	{		
		if ( !m_pSchedules->isCurrentSchedule(SCHED_HEAL) )
		{
			m_pSchedules->removeSchedule(SCHED_HEAL);
			m_pSchedules->addFront(new CBotTF2HealSched());
		}
	}
	else if ( m_pEnemy && hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{		
		if ( !m_pSchedules->isCurrentSchedule(SCHED_ATTACK) )
		{
			m_pSchedules->removeSchedule(SCHED_ATTACK);
			m_pSchedules->addFront(new CBotAttackSched(m_pEnemy));

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
		CWaypoint *pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT,getTeam()));

		if ( pWaypoint )
		{
			m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
		}
		else
		{
			// roam
			CWaypoint *pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomFlaggedWaypoint(getTeam()));

			if ( pWaypoint )
				m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
		}
	}
	else
	{
		CWaypoint *pWaypoint = NULL;
		// Find enemy flag or defend flag or roam

		// Pickup health
		if ( ((float)m_pPlayerInfo->GetHealth()/m_pPlayerInfo->GetMaxHealth()) < 0.6f )
		{
			pWaypoint = CWaypoints::getWaypoint(CWaypoints::nearestWaypointGoal(CWaypointTypes::W_FL_HEALTH,getOrigin(),4096.0,getTeam()));

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotTF2GetHealthSched(pWaypoint->getOrigin()));
				return;
			}
		}

		if ( getClass() == TF_CLASS_ENGINEER )
		{		
			CBotWeapon *pWeapon = m_pWeapons->getWeapon(CWeapons::getWeapon(TF2_WEAPON_WRENCH));

			checkBuildingsValid();

			if ( pWeapon )
			{
				if ( pWeapon->getAmmo(this) < 200 )
				{
					CWaypoint *pDisp = NULL;
					CWaypoint *pWaypointAmmo = CWaypoints::getWaypoint(CWaypoints::nearestWaypointGoal(CWaypointTypes::W_FL_AMMO,getOrigin(),4096,getTeam()));
					CWaypoint *pWaypointResupply = CWaypoints::getWaypoint(CWaypoints::nearestWaypointGoal(CWaypointTypes::W_FL_RESUPPLY,getOrigin(),4096,getTeam()));

					if ( m_pDispenser )
					{
						pDisp = CWaypoints::getWaypoint(CWaypointLocations::NearestWaypoint(CBotGlobals::entityOrigin(m_pDispenser),150,-1,true,false,true,NULL,false,getTeam()));
					}

					if ( pDisp )
					{
						// to do : Get ammo from Disp Schedule
						m_pSchedules->add(new CBotGetMetalSched(CBotGlobals::entityOrigin(m_pDispenser)));
							return;
					}
					else if ( pWaypointAmmo && pWaypointResupply )
					{
						if ( distanceFrom(pWaypointResupply->getOrigin()) < 1000 ) // not too far
						{
							m_pSchedules->add(new CBotGetMetalSched(pWaypointAmmo->getOrigin()));
							return;
						}
						else
						{
							m_pSchedules->add(new CBotGetMetalSched(pWaypointResupply->getOrigin()));
							return;
						}
					}
					else if ( pWaypointAmmo )
					{
						m_pSchedules->add(new CBotGetMetalSched(pWaypointAmmo->getOrigin()));
						return;
					}
					else if ( pWaypointResupply )
					{
						m_pSchedules->add(new CBotGetMetalSched(pWaypointResupply->getOrigin()));
						return;
					}
					// Get ammo
				}
				else if ( !m_pSentryGun )
				{
					pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SENTRY,getTeam()));

					if ( pWaypoint )
					{
						m_pSchedules->add(new CBotTFEngiBuild(ENGI_SENTRY,pWaypoint->getOrigin()));
						return;
					}
				}
				else if ( !m_pDispenser )
				{
					pWaypoint = CWaypoints::getWaypoint(CWaypointLocations::NearestWaypoint(CBotGlobals::entityOrigin(m_pSentryGun),150,-1,true,false,true,NULL,false,getTeam()));

					if ( pWaypoint )
					{
						m_pSchedules->add(new CBotTFEngiBuild(ENGI_DISP,pWaypoint->getOrigin()+Vector(RandomFloat(-96,96),RandomFloat(-96,96),0)));
						return;
					}
				}
				else
				{
					if ( CTeamFortress2Mod::getSentryLevel(m_pSentryGun) < 3 )
					{
						// upgrade it !
						
						pWaypoint = CWaypoints::getWaypoint(CWaypointLocations::NearestWaypoint(CBotGlobals::entityOrigin(m_pSentryGun),150,-1,true,false,true,NULL,false,getTeam()));

						if ( pWaypoint )
						{
							m_pSchedules->add(new CBotTFEngiUpgrade(m_pSentryGun));
							return;
						}
					}
				}

			}
		}
		else if ( getClass() == TF_CLASS_SNIPER )
		{
			pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_SNIPER,getTeam()));

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotTF2SnipeSched(pWaypoint->getOrigin()));
				return;
			}
		}

		if ( !pWaypoint && m_fLastKnownFlagTime && (m_fLastKnownFlagTime > engine->Time()) )
		{
			pWaypoint = CWaypoints::getWaypoint(CWaypoints::nearestWaypointGoal(-1,m_vLastKnownFlagPoint,512.0,getTeam()));

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotTF2FindFlagSched(m_vLastKnownFlagPoint));
				return;
			}
		}


		if ( CTeamFortress2Mod::isMapType(TF_MAP_CTF) )
		{
			// roam
			pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomWaypointGoal(CWaypointTypes::W_FL_FLAG,getTeam()));

			if ( pWaypoint )
			{
				m_pSchedules->add(new CBotTF2GetFlagSched(pWaypoint->getOrigin()));
				return;
			}
		}

		if ( !pWaypoint )
		{
			// roam
			pWaypoint = CWaypoints::getWaypoint(CWaypoints::randomFlaggedWaypoint(getTeam()));
		}

		if ( pWaypoint )
			m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
	}

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
			m_pSchedules->addFront(new CBotSchedule(new CBotTFDoubleJump()));
		}
	}
}

bool CBotTF2 :: handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy )
{
	if ( pWeapon )
	{
		bool bSecAttack = false;

		if ( pWeapon->isMelee() )
			setMoveTo(CBotGlobals::entityOrigin(m_pEdict),4);

		if ( CTeamFortress2Mod::isRocket(m_pEdict,CTeamFortress2Mod::getEnemyTeam(getTeam())) )
		{
			float fDistance = distanceFrom(CBotGlobals::entityOrigin(pEnemy));

			if ( (fDistance < 400) && pWeapon->canDeflectRockets() && (pWeapon->getAmmo(this) > 25) )
				bSecAttack = true;
			else
				return false;
		}

		if ( !bSecAttack )
		{
			if ( pWeapon->mustHoldAttack() )
				primaryAttack(true);
			else
				primaryAttack();
		}
		else
		{
			secondaryAttack();
		}
	}
	else
		primaryAttack();

	return true;
}

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
		if (  CBotGlobals::getTeam(pEdict) != getTeam() )
		{
			// spy?
			if ( CTeamFortress2Mod::TF2_IsPlayerDisguised(pEdict) || CTeamFortress2Mod::TF2_IsPlayerCloaked(pEdict) )
				bValid = false;
			else
				bValid = true;
		}


	}
	else
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