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

	if ( pBot )
		pBot->hurt(pAttacker,pEvent->getInt("health"));

	pBot = CBots::getBotPointer(pAttacker);

	if ( pBot )
		pBot->shot(m_pActivator);
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
void CTF2BuiltObjectEvent :: execute ( IBotEventInterface *pEvent )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);

	if ( pBot && pBot->isTF() )
	{
		((CBotFortress*)pBot)->engiBuildSuccess((eEngiBuild)pEvent->getInt("object"));
	}
}

void CFlagEvent :: execute ( IBotEventInterface *pEvent )
{
	int type = pEvent->getInt("eventtype");

	int player = pEvent->getInt("player");

	edict_t *pPlayer = INDEXENT(player);

	CBot *pBot = CBots::getBotPointer(pPlayer);

	switch ( type )
	{
	case 1: // pickup
		if ( pBot && pBot->isTF() )
			((CBotTF2*)pBot)->pickedUpFlag();
		break;
	case 2: // captured
		{
			IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pPlayer);

			if ( p )
			{
				CBroadcastFlagCaptured *captured = new CBroadcastFlagCaptured(p->GetTeamIndex());
				CBots::botFunction(captured);
				delete captured;
			}

			if ( pBot && pBot->isTF() )
			{
				((CBotTF2*)pBot)->capturedFlag();	
				((CBotTF2*)pBot)->droppedFlag();	
			}
		
			

			
		}
		break;
	case 4: // drop
		{
			IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pPlayer);

			if ( p )
			{
				CBroadcastFlagDropped *dropped = new CBroadcastFlagDropped(p->GetTeamIndex(),CBotGlobals::entityOrigin(pPlayer));
				CBots::botFunction(dropped);
				delete dropped;
			}

			if ( pBot && pBot->isTF() )
				((CBotTF2*)pBot)->droppedFlag();

			
			
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
	addEvent(new CTF2BuiltObjectEvent());
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
