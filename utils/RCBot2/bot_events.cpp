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
#include "igameevents.h"
#include "bot.h"
#include "bot_event.h"
#include "bot_strings.h"
#include "bot_globals.h"
#include "bot_fortress.h"
#include "bot_script.h"

vector<CBotEvent*> CBotEvents :: m_theEvents;
///////////////////////////////////////////////////////

void CRoundStartEvent :: execute ( IBotEventInterface *pEvent )
{
	CBots::roundStart();
}

void CPlayerHurtEvent :: execute ( IBotEventInterface *pEvent )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);
	edict_t *pAttacker = CBotGlobals::playerByUserId(pEvent->getInt("attacker"));

	if ( pAttacker && (!pAttacker->m_pNetworkable || !pAttacker->m_NetworkSerialNumber) )
		pAttacker = NULL;

	if ( pBot )
		pBot->hurt(pAttacker,pEvent->getInt("health"));

	pBot = CBots::getBotPointer(pAttacker);

	if ( pBot )
		pBot->shot(m_pActivator);

	//CBots::botFunction()
}

void CPlayerDeathEvent :: execute ( IBotEventInterface *pEvent )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);

	edict_t *pAttacker = CBotGlobals::playerByUserId(pEvent->getInt("attacker"));

	if ( pBot )
		pBot->died(pAttacker);

	pBot = CBots::getBotPointer(pAttacker);

	if ( pBot )
		pBot->killed(m_pActivator);
}

void CBombPickupEvent :: execute ( IBotEventInterface *pEvent )
{
}

void CPlayerFootstepEvent :: execute ( IBotEventInterface *pEvent )
{
}

void CBombDroppedEvent :: execute ( IBotEventInterface *pEvent )
{
}

void CWeaponFireEvent :: execute ( IBotEventInterface *pEvent )
{
}

void CPlayerSpawnEvent :: execute ( IBotEventInterface *pEvent )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);

	if ( pBot )
		pBot->spawnInit();
}

void CBulletImpactEvent :: execute ( IBotEventInterface *pEvent )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);

	if ( pBot )
	{
		pBot->shotmiss();
	}
}
/////////////////////////////////////////

/*
[RCBot] [DEBUG game_event] player_sapped_object
[RCBot] [DEBUG game_event] userid = 2
[RCBot] [DEBUG game_event] ownerid = 4
[RCBot] [DEBUG game_event] object = 2
[RCBot] [DEBUG game_event] sapperid = 400
*/
void CTF2ObjectSapped :: execute ( IBotEventInterface *pEvent )
{
	int owner = pEvent->getInt("ownerid",-1);
	int building = pEvent->getInt("object",-1);
	int sapperid = pEvent->getInt("sapperid",-1);

	if ( m_pActivator && (owner>=0) && (building>=0) && (sapperid>=0) )
	{
		edict_t *pSpy = m_pActivator;
		edict_t *pOwner = CBotGlobals::playerByUserId(owner);
		edict_t *pSapper = INDEXENT(sapperid);
		CBotTF2 *pBot = (CBotTF2*)CBots::getBotPointer(pOwner);
		
		if ( pBot )
		{
			pBot->buildingSapped((eEngiBuild)building,pSapper,pSpy);
		}

		CTeamFortress2Mod::sapperPlaced(pOwner,(eEngiBuild)building,pSapper);

		CBroadcastSpySap spysap = CBroadcastSpySap(pSpy);

		CBots::botFunction(&spysap);

	}
}
/*
[RCBot] [DEBUG game_event] object_destroyed
[RCBot] [DEBUG game_event] userid = 2
[RCBot] [DEBUG game_event] attacker = 4
[RCBot] [DEBUG game_event] weapon = wrench
[RCBot] [DEBUG game_event] weapon_logclassname = wrench
[RCBot] [DEBUG game_event] weaponid = 10
[RCBot] [DEBUG game_event] priority = 6
[RCBot] [DEBUG game_event] objecttype = 3
[RCBot] [DEBUG game_event] index = 436
[RCBot] [DEBUG game_event] was_building = 0
*/
void CTF2ObjectDestroyed :: execute ( IBotEventInterface *pEvent )
{
	int type = pEvent->getInt("objecttype",-1);
	int index = pEvent->getInt("index",-1);
	int engi = pEvent->getInt("attacker",-1);
	int was_building = pEvent->getInt("was_building",0);

	if ( (engi>=0) && m_pActivator && (type>=0) && (index>=0) && (was_building>=0) )
	{
		if ( !was_building )
		{ // could be a sapper
			if ( (eEngiBuild)type == ENGI_SAPPER )
			{
				edict_t *pOwner = CBotGlobals::playerByUserId(engi);
				edict_t *pSapper = INDEXENT(index);
				CBotTF2 *pBot = (CBotTF2*)CBots::getBotPointer(pOwner);

				if ( pBot )
					pBot->sapperDestroyed(pSapper);

				CTeamFortress2Mod::sapperDestroyed(pOwner,(eEngiBuild)type,pSapper);
			}
		}
	}


}

void CTF2BuiltObjectEvent :: execute ( IBotEventInterface *pEvent )
{
	eEngiBuild type = (eEngiBuild)pEvent->getInt("object");
	int index = pEvent->getInt("index");
	edict_t *pBuilding = INDEXENT(index);
	CBot *pBot = CBots::getBotPointer(m_pActivator);

	if ( type == ENGI_TELE )
		CTeamFortress2Mod::teleporterBuilt(m_pActivator,type,pBuilding);
	if ( type == ENGI_SENTRY )
		CTeamFortress2Mod::sentryBuilt(m_pActivator,type,pBuilding);
	if ( type == ENGI_DISP )
		CTeamFortress2Mod::dispenserBuilt(m_pActivator,type,pBuilding);

	if ( pBot && pBot->isTF() )
	{
		((CBotFortress*)pBot)->engiBuildSuccess((eEngiBuild)pEvent->getInt("object"),pEvent->getInt("index"));
	}
}

void CTF2ChangeClass :: execute ( IBotEventInterface *pEvent )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);

	if ( pBot && pBot->isTF() )
	{

		int _class = pEvent->getInt("class");

		((CBotFortress*)pBot)->setClass((TF_Class)_class);

	}
}

void CTF2RoundStart :: execute ( IBotEventInterface *pEvent )
{
	// 04/07/09 : add full reset

	  CBroadcastRoundStart roundstart = CBroadcastRoundStart(pEvent->getInt("full_reset") == 1);
	  
	  if ( pEvent->getInt("full_reset") == 1 )
		CPoints::resetPoints();
	  // MUST BE AFTER RESETPOINTS
	  CBots::botFunction(&roundstart);

	  CTeamFortress2Mod::resetSetupTime();
	
}

void CTF2PointStartCapture :: execute ( IBotEventInterface *pEvent )
{/*
 [RCBot] [DEBUG game_event] teamplay_point_startcapture
[RCBot] [DEBUG game_event] cp = 0
[RCBot] [DEBUG game_event] cpname = #Dustbowl_cap_1_A
[RCBot] [DEBUG game_event] team = 2
[RCBot] [DEBUG game_event] capteam = 3
[RCBot] [DEBUG game_event] captime = 64.134995
[RCBot] [DEBUG game_event] cappers = 
[RCBot] [DEBUG game_event] priority = 7
*/
    int capteam = pEvent->getInt("capteam",0);
//	const char *cpname = pEvent->getString("cpname","");
//	int cp = pEvent->getInt("cp",0);

	CBotTF2FunctionEnemyAtIntel *function = new CBotTF2FunctionEnemyAtIntel(capteam,Vector(0,0,0),EVENT_CAPPOINT);

	CBots::botFunction(function);

	delete function;
	
}

void CTF2PointCaptured :: execute ( IBotEventInterface *pEvent )
{
	CBroadcastCapturedPoint cap = CBroadcastCapturedPoint(pEvent->getInt("cp"),pEvent->getInt("team"),pEvent->getString("cpname"));
	
	CPoints::pointCaptured(pEvent->getInt("team"),pEvent->getString("cpname"));
    // MUST BE AFTER POINTCAPTURED
    CBots::botFunction(&cap);
}

/* Flag has been picked up or dropped */
#define FLAG_PICKUP		1
#define FLAG_CAPTURED	2
#define FLAG_DEFEND		3
#define FLAG_DROPPED	4
#define FLAG_RETURN		5

void CFlagEvent :: execute ( IBotEventInterface *pEvent )
{
	// dropped / picked up ID
	int type = pEvent->getInt("eventtype");
	// player id
	int player = pEvent->getInt("player");

	edict_t *pPlayer = INDEXENT(player);

	CBot *pBot = CBots::getBotPointer(pPlayer);

	switch ( type )
	{
	case FLAG_PICKUP: // pickup
		if ( pBot && pBot->isTF() )
		{
			((CBotTF2*)pBot)->pickedUpFlag();
		}

		if ( pPlayer )
			CTeamFortress2Mod::flagPickedUp(CTeamFortress2Mod::getTeam(pPlayer),pPlayer);
		break;
	case FLAG_CAPTURED: // captured
		{
			IPlayerInfo *p = NULL;
			
			if( pPlayer )
				playerinfomanager->GetPlayerInfo(pPlayer);

			if ( p )
			{
				CBroadcastFlagCaptured captured = CBroadcastFlagCaptured(p->GetTeamIndex());
				CBots::botFunction(&captured);
			}

			if ( pBot && pBot->isTF() )
			{
				((CBotTF2*)pBot)->capturedFlag();	
				((CBotTF2*)pBot)->droppedFlag();	
			}
		
			if ( pPlayer )
				CTeamFortress2Mod::flagDropped(CTeamFortress2Mod::getTeam(pPlayer));

			
		}
		break;
	case FLAG_DROPPED: // drop
		{
			IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pPlayer);

			if ( p )
			{
				CBroadcastFlagDropped dropped = CBroadcastFlagDropped(p->GetTeamIndex(),CBotGlobals::entityOrigin(pPlayer));
				CBots::botFunction(&dropped);
			}

			if ( pBot && pBot->isTF() )
				((CBotTF2*)pBot)->droppedFlag();

			
			if ( pPlayer )
				CTeamFortress2Mod::flagDropped(CTeamFortress2Mod::getTeam(pPlayer));
		}
		break;
	default:	
		break;
	}

}

void CFlagCaptured :: execute ( IBotEventInterface *pEvent )
{

}

///////////////////////////////////////////////////////

void CBotEvent :: setType ( char *szType )
{
	m_szType = CStrings::getString(szType);
}
bool CBotEvent :: forCurrentMod ()
{
	return ((m_iModId == MOD_ANY) || (CBotGlobals::isMod(m_iModId)));
}
// should we execute this ??
bool CBotEvent :: isType ( const char *szType )
{
	return forCurrentMod() && FStrEq(m_szType,szType);
}

///////////////////////////////////////////////////////
void CBotEvents :: setupEvents ()
{
	addEvent(new CRoundStartEvent());
	addEvent(new CPlayerHurtEvent());
	addEvent(new CPlayerDeathEvent());
	addEvent(new CBombPickupEvent());
	addEvent(new CPlayerFootstepEvent());
	addEvent(new CBombDroppedEvent());
	addEvent(new CWeaponFireEvent());
	addEvent(new CBulletImpactEvent());
	addEvent(new CFlagEvent());
	addEvent(new CPlayerSpawnEvent());
	////////////// tf2
	addEvent(new CTF2BuiltObjectEvent());
	addEvent(new CTF2ChangeClass());
	addEvent(new CTF2RoundStart());
	addEvent(new CTF2PointCaptured());
	addEvent(new CTF2PointStartCapture());
	addEvent(new CTF2ObjectSapped());
	addEvent(new CTF2ObjectDestroyed());
}

void CBotEvents :: addEvent ( CBotEvent *pEvent )
{
	extern IGameEventManager2 *gameeventmanager;
	extern CRCBotPlugin g_RCBOTServerPlugin;

	if ( gameeventmanager )
		gameeventmanager->AddListener( g_RCBOTServerPlugin.getEventListener(), pEvent->getName(), true );

	m_theEvents.push_back(pEvent);
}

void CBotEvents :: freeMemory ()
{
	for ( unsigned int i = 0; i < m_theEvents.size(); i ++ )
	{
		delete m_theEvents[i];
		m_theEvents[i] = NULL;	
	}
	m_theEvents.clear();
}

void CBotEvents :: executeEvent( void *pEvent, eBotEventType iType )
{
	CBotEvent *pFound;
	int iEventId = -1; 
	bool bFound;

	IBotEventInterface *pInterface = NULL;

	if ( iType == TYPE_KEYVALUES )
		pInterface = new CGameEventInterface1((KeyValues*)pEvent);
	else if ( iType == TYPE_IGAMEEVENT )
		pInterface = new CGameEventInterface2((IGameEvent*)pEvent);

	if ( pInterface == NULL )
		return;

	if ( iType != TYPE_IGAMEEVENT )
		iEventId = pInterface->getInt("eventid");

	for ( unsigned int i = 0; i < m_theEvents.size(); i ++ )
	{
		pFound = m_theEvents[i];

		// if it has an pEvent id stored just check that
		//if ( ( iType != TYPE_IGAMEEVENT ) && pFound->hasEventId() )
		//	bFound = pFound->isEventId(iEventId);
		//else
		bFound = pFound->isType(pInterface->getName());

		if ( bFound )	
		{
			// set pEvent id for quick checking
			pFound->setEventId(iEventId);

			pFound->setActivator(CBotGlobals::playerByUserId(pInterface->getInt("userid")));

			pFound->execute(pInterface);

			break;
		}
	}

	delete pInterface;
}
