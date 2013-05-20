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
#include "bot_waypoint.h"
#include "ndebugoverlay.h"
#include "bot_menu.h"

// setup static client array
CClient CClients::m_Clients[MAX_PLAYERS];
CClient *CClients::m_pListenServerClient = NULL;
bool CClients::m_bClientsDebugging = false;

extern IVDebugOverlay *debugoverlay;

void CClient :: init ()
{
	m_iWaypointShowFlags = 0;
	m_fSpeed = 0;
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
	m_iWptArea = 0;
	m_iWaypointDrawType = 3;
	m_bShowMenu = false;
	m_fUpdatePos = 0.0f;
	m_pDebugBot = NULL;
	m_fCopyWptRadius = 0.0f;
	m_iCopyWptFlags = 0;
	m_iCopyWptArea = 0;
	m_pMenu = NULL;
	m_iMenuCommand = -1;
	m_fNextUpdateMenuTime = 0.0f;
}

bool CClient :: needToRenderMenu () 
{ 
	return m_fNextUpdateMenuTime < engine->Time(); 
}

void CClient :: updateRenderMenuTime () 
{ 
	extern ConVar rcbot_menu_update_time2;
	m_fNextUpdateMenuTime = engine->Time() + rcbot_menu_update_time2.GetFloat(); 
}

void CClient :: setEdict ( edict_t *pPlayer )
{
	m_pPlayer = pPlayer;
	m_pPlayerInfo = playerinfomanager->GetPlayerInfo(pPlayer);
}
	
void CClient :: setupMenuCommands ()
{
	/*engine->ClientCommand(m_pPlayer,"alias \"rcbot_setup\" \"bind 0 menuselect0\"");
	engine->ClientCommand(m_pPlayer,"rcbot_setup");bind 2 \"menuselect 2\"");*/
	engine->ClientCommand(m_pPlayer,"bind 1 \"menuselect 1\"");
	engine->ClientCommand(m_pPlayer,"bind 2 \"menuselect 2\"");
	engine->ClientCommand(m_pPlayer,"bind 3 \"menuselect 3\"");
	engine->ClientCommand(m_pPlayer,"bind 4 \"menuselect 4\"");
	engine->ClientCommand(m_pPlayer,"bind 5 \"menuselect 5\"");
	engine->ClientCommand(m_pPlayer,"bind 6 \"menuselect 6\"");
	engine->ClientCommand(m_pPlayer,"bind 7 \"menuselect 7\"");
	engine->ClientCommand(m_pPlayer,"bind 8 \"menuselect 8\"");
	engine->ClientCommand(m_pPlayer,"bind 9 \"menuselect 9\"");
	engine->ClientCommand(m_pPlayer,"bind 0 \"menuselect 0\"");
}
	
void CClient :: resetMenuCommands ()
{
	/*engine->ClientCommand(m_pPlayer,"alias \"rcbot_reset\" \"bind 0 slot10\"");
	engine->ClientCommand(m_pPlayer,"rcbot_reset");bind 2 \"menuselect 2\"");*/
	engine->ClientCommand(m_pPlayer,"bind 1 \"slot1\"");
	engine->ClientCommand(m_pPlayer,"bind 2 \"slot2\"");
	engine->ClientCommand(m_pPlayer,"bind 3 \"slot3\"");
	engine->ClientCommand(m_pPlayer,"bind 4 \"slot4\"");
	engine->ClientCommand(m_pPlayer,"bind 5 \"slot5\"");
	engine->ClientCommand(m_pPlayer,"bind 6 \"slot6\"");
	engine->ClientCommand(m_pPlayer,"bind 7 \"slot7\"");
	engine->ClientCommand(m_pPlayer,"bind 8 \"slot8\"");
	engine->ClientCommand(m_pPlayer,"bind 9 \"slot9\"");
	engine->ClientCommand(m_pPlayer,"bind 0 \"slot10\"");
}

// called each frame
void CClient :: think ()
{
	if ( m_bShowMenu )
	{
		m_bShowMenu = false;
		engine->ClientCommand(m_pPlayer,"cancelselect");
	}

	if ( m_pMenu != NULL )
	{
		if ( needToRenderMenu() )
			m_pMenu->render(this);
		//CBotMenuList::render(pClient);
	}

	if ( isWaypointOn() )
		CWaypoints::drawWaypoints(this);

	if ( m_fUpdatePos < engine->Time() )
	{
		m_fUpdatePos = engine->Time() + 1.0f;
		m_vVelocity = (getOrigin()-m_vLastPos);
		m_fSpeed = m_vVelocity.Length();
		m_vLastPos = getOrigin();
	}

	if ( isDebugging() )
	{
		IPlayerInfo *p = playerinfomanager->GetPlayerInfo(m_pPlayer);


		if ( isDebugOn(BOT_DEBUG_SPEED) )
		{
			CBotGlobals::botMessage(m_pPlayer,0,"speed = %0.0f",m_fSpeed);
		}

		if ( isDebugOn(BOT_DEBUG_USERCMD) )
		{

			if ( p )
			{
				CBotCmd cmd = p->GetLastUserCommand();

				CBotGlobals::botMessage(m_pPlayer,0,"Btns = %d, cmd_no = %d, impulse = %d, weapselect = %d, weapsub = %d",cmd.buttons,cmd.command_number,cmd.impulse,cmd.weaponselect,cmd.weaponsubtype);

			}
		}


		if ( (m_pDebugBot!=NULL) && isDebugOn(BOT_DEBUG_HUD) )
		{
			if ( m_fNextPrintDebugInfo < engine->Time() )
			{
				char msg[1024];

				QAngle eyes = p->GetLastUserCommand().viewangles;
				Vector vForward;
				// in fov? Check angle to edict
				AngleVectors(eyes,&vForward);

				vForward = vForward / vForward.Length(); // normalize
				Vector vLeft = (vForward-p->GetAbsOrigin()).Cross(Vector(0,0,1));
				vLeft = vLeft/vLeft.Length();
				
				Vector vDisplay = p->GetAbsOrigin() + vForward*300.0f; 
				vDisplay = vDisplay - vLeft*300.0f;

				// get debug message
				m_pDebugBot->debugBot(msg);

#ifndef __linux__
				int i = 0; 
				int n = 0;
				char line[256];
				int linenum = 0;
				int iIndex = ENTINDEX(m_pDebugBot->getEdict());

				do
				{
					while ( (msg[i]!=0) && (msg[i]!='\n') ) 
						line[n++] = msg[i++];

					line[n]=0;
					debugoverlay->AddEntityTextOverlay(iIndex,linenum++,1.0f,255,255,255,255,line);
					n = 0;

					if ( msg[i] == 0 )
						break;
					i++;
				}while ( 1 ) ;
				//int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char *format, ...
			//	debugoverlay->AddEntityTextOverlay();
#endif
				m_fNextPrintDebugInfo = engine->Time() + 1.0f;
			}
		}
			//this->cm_pDebugBot->getTaskDebug();
		//m_pDebugBot->canAvoid();
	}
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
}

void CClient :: updateCurrentWaypoint ()
{
	setWaypoint(CWaypointLocations::NearestWaypoint(getOrigin(),50,-1,false,true,false,NULL,false,0,false,false,Vector(0,0,0),m_iWaypointShowFlags));
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
		return  playerinfo->GetAbsOrigin() + Vector(0,0,32);
	}

	return CBotGlobals::entityOrigin(m_pPlayer) + Vector(0,0,32);//m_pPlayer->GetCollideable()->GetCollisionOrigin();
}

void CClients :: clientActive ( edict_t *pPlayer )
{
	CClient *pClient = &m_Clients[slotOfEdict(pPlayer)];

	pClient->clientActive();
}

CClient *CClients :: clientConnected ( edict_t *pPlayer )
{
	CClient *pClient = &m_Clients[slotOfEdict(pPlayer)];

	pClient->clientConnected(pPlayer);

	return pClient;
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

	edict_t *pPlayer;

	m_bClientsDebugging = false;

	for ( int i = 0; i < MAX_PLAYERS; i ++ )
	{
		pClient = &m_Clients[i];

		if ( !pClient->isUsed() )
			continue;
		if ( !m_bClientsDebugging && pClient->isDebugging() )
			m_bClientsDebugging = true;

		pPlayer = pClient->getPlayer();
	
		if ( pPlayer && pPlayer->GetIServerEntity() )
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

void CClients::clientDebugMsg(CBot *pBot, int iLev, const char *fmt, ... )
{
	va_list argptr; 
	static char string[1024];

	va_start (argptr, fmt);
	vsprintf (string, fmt, argptr); 
	va_end (argptr); 

	clientDebugMsg(iLev,string,pBot);
}

const char *g_szDebugTags[14] =
{
"GAME_EVENT",
"NAV",
"SPEED",
"VIS",
"TASK",
"BUTTONS",
"USERCMD",
"UTIL",
"PROFILE",
"EDICTS",
"THINK",
"LOOK",
"HUD",
"AIM"
};


void CClients :: clientDebugMsg ( int iLev, const char *szMsg, CBot *pBot )
{
	CClient *pClient;

	for ( int i = 0; i < MAX_PLAYERS; i ++ )
	{
		pClient = &m_Clients[i];

		if ( !pClient->isUsed() )
			continue;
		if ( !pClient->isDebugOn(iLev) )
			continue;		
		if ( pBot && !pClient->isDebuggingBot(pBot) )
			continue;

		CBotGlobals::botMessage(pClient->getPlayer(),0,"[DEBUG %s] %s",g_szDebugTags[iLev],szMsg);
	}
}

	// get index in array
int CClients :: slotOfEdict ( edict_t *pPlayer )
{
	return ENTINDEX(pPlayer)-1;
}

bool CClients :: clientsDebugging (int iLev)
{
	if ( iLev == 0 )
		return m_bClientsDebugging;
	else if ( m_bClientsDebugging )
	{
		int i;
		CClient *pClient;

		for ( i = 0; i < MAX_PLAYERS; i ++ )
		{
			pClient = CClients::get(i);

			if ( pClient->isUsed() )
			{
				if ( pClient->isDebugOn(iLev) )
					return true;
			}
		}
	}

	return false;
}

void CClient :: setWaypointCut (CWaypoint *pWaypoint)
{
	if ( pWaypoint )
	{
		register int i = 0;

		setWaypointCopy(pWaypoint);

		m_WaypointCutPaths.clear();

		for ( i = 0; i < pWaypoint->numPaths(); i ++ )
		{
			m_WaypointCutPaths.push_back(pWaypoint->getPath(i));
		}

		m_WaypointCopyType = WPT_COPY_CUT;
	}
}

void CClient :: setWaypointCopy (CWaypoint *pWaypoint) 
{
	if (pWaypoint) 
	{ 
		m_fCopyWptRadius = pWaypoint->getRadius();
		m_iCopyWptFlags = pWaypoint->getFlags();
		m_iCopyWptArea = pWaypoint->getArea();
		m_WaypointCopyType = WPT_COPY_COPY;
	} 
}