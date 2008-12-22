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
#include "bot_client.h"
#include "bot_waypoint_locations.h"
#include "bot_accessclient.h"
#include "bot_commands.h"
#include "bot_globals.h"

// setup static client array
CClient CClients::m_Clients[MAX_PLAYERS];
CClient *CClients::m_pListenServerClient = NULL;
bool CClients::m_bClientsDebugging = false;

void CClient :: init ()
{
	m_pPlayer = NULL;
	m_szSteamID = NULL;
	m_bWaypointOn = false;
	m_iCurrentWaypoint = -1;
	m_iAccessLevel = 0;
	m_bAutoPaths = true;
	m_iPathFrom = -1;
	m_iPathTo = -1;
	m_bPathWaypointOn = false;
	m_iDebugLevels = 0;
	m_pPlayerInfo = NULL;
}

void CClient :: setEdict ( edict_t *pPlayer )
{
	m_pPlayer = pPlayer;
	m_pPlayerInfo = playerinfomanager->GetPlayerInfo(pPlayer);
}

// called each frame
void CClient :: think ()
{
	if ( isWaypointOn() )
		CWaypoints::drawWaypoints(this);
}

const char *CClient :: getName ()
{
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo( m_pPlayer );

	if ( playerinfo )
		return playerinfo->GetName();

	return NULL;
}

void CClient :: clientActive ()
{
	// get steam id
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo( m_pPlayer );

	m_szSteamID = NULL;

	if ( playerinfo )
	{
		// store steam id
		m_szSteamID = (char*)playerinfo->GetNetworkIDString();
	
		// check my access levels
		CAccessClients::checkClientAccess(this);
	}
}
// this player joins with pPlayer edict
void CClient :: clientConnected ( edict_t *pPlayer )
{
	init();
	// set player edict
	setEdict(pPlayer);

	if ( !engine->IsDedicatedServer() )
	{
		if ( CClients::noListenServerClient() )
		{
			// give listenserver client all access to bot commands
			CClients::setListenServerClient(this);		
			setAccessLevel(CMD_ACCESS_ALL);
		}
	}
}

void CClient :: updateCurrentWaypoint ()
{
	setWaypoint(CWaypointLocations::NearestWaypoint(getOrigin(),50,-1,false,true));
}
// this player disconnects
void CClient :: clientDisconnected ()
{
	// is bot?
	CBot *pBot = CBots::getBotPointer(m_pPlayer);

	if ( pBot != NULL )
	{
		// free bots memory and other stuff
		pBot->freeAllMemory();
	}

	if ( !engine->IsDedicatedServer() )
	{
		if ( CClients::isListenServerClient(this) )
		{
			CClients::setListenServerClient(NULL);
		}
	}

	init();
}

int CClient :: accessLevel ()
{
	return m_iAccessLevel;
}

bool CClient :: isUsed ()
{
	return (m_pPlayer != NULL);
}

Vector CClient :: getOrigin ()
{
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo( m_pPlayer );

	if ( playerinfo )
	{
		return playerinfo->GetAbsOrigin() + Vector(0,0,32);
	}

	return CBotGlobals::entityOrigin(m_pPlayer);//m_pPlayer->GetCollideable()->GetCollisionOrigin();
}

void CClients :: clientActive ( edict_t *pPlayer )
{
	CClient *pClient = &m_Clients[slotOfEdict(pPlayer)];

	pClient->clientActive();
}

void CClients :: clientConnected ( edict_t *pPlayer )
{
	CClient *pClient = &m_Clients[slotOfEdict(pPlayer)];

	pClient->clientConnected(pPlayer);
}

void CClients :: init ( edict_t *pPlayer )
{
	m_Clients[slotOfEdict(pPlayer)].init();
}

void CClients :: clientDisconnected ( edict_t *pPlayer )
{
	CClient *pClient = &m_Clients[slotOfEdict(pPlayer)];

	pClient->clientDisconnected();
}

void CClients :: clientThink ()
{
	static CClient *pClient;

	m_bClientsDebugging = false;

	for ( int i = 0; i < MAX_PLAYERS; i ++ )
	{
		pClient = &m_Clients[i];

		if ( !pClient->isUsed() )
			continue;
		if ( !m_bClientsDebugging && pClient->isDebugging() )
			m_bClientsDebugging = true;
	
		pClient->think();
	}
}

CClient *CClients :: findClientBySteamID ( char *szSteamID )
{
	CClient *pClient;

	for ( int i = 0; i < MAX_PLAYERS; i ++ )
	{
		pClient = &m_Clients[i];

		if ( pClient->isUsed() )
		{
			if ( FStrEq(pClient->getSteamID(),szSteamID) )
				return pClient;
		}
	}	

	return NULL;
}

void CClients :: clientDebugMsg ( int iLev, const char *szMsg )
{
	CClient *pClient;

	char *szDebugLev;

	switch ( iLev )
	{
	case BOT_DEBUG_GAME_EVENT:
		szDebugLev = "game_event";
		break;
	default:
		szDebugLev = "unknown";
		break;
	}

	for ( int i = 0; i < MAX_PLAYERS; i ++ )
	{
		pClient = &m_Clients[i];

		if ( !pClient->isUsed() )
			continue;
		if ( !pClient->isDebugOn(iLev) )
			continue;		

		CBotGlobals::botMessage(pClient->getPlayer(),0,"[DEBUG %s] %s",szDebugLev,szMsg);
	}
}

	// get index in array
int CClients :: slotOfEdict ( edict_t *pPlayer )
{
	return ENTINDEX(pPlayer)-1;
}

bool CClients :: clientsDebugging ()
{
	return m_bClientsDebugging;
}