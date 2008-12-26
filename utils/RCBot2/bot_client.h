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
#ifndef __RCBOT_CLIENT_H__
#define __RCBOT_CLIENT_H__

#include "bot_const.h"

struct edict_t;
class CBot;

class CClient
{
public:
	void init ();

	int accessLevel ();
	// this player joins with pPlayer edict
	void clientConnected ( edict_t *pPlayer );
	// this player disconnects
	void clientDisconnected ();	

	bool isUsed ();

	Vector getOrigin ();

	void setEdict ( edict_t *pPlayer );

	edict_t *getPlayer () { return m_pPlayer; }

	inline bool isPlayer ( edict_t *pPlayer ) { return m_pPlayer == pPlayer; }

	inline bool isWaypointOn () { return m_bWaypointOn; }
	inline void setWaypointOn ( bool bOn ) { m_bWaypointOn = bOn; }
	inline void setWaypoint ( int iWpt ) { m_iCurrentWaypoint = iWpt; }
	inline int currentWaypoint () { return m_iCurrentWaypoint; }

	inline void setAccessLevel ( int iLev ) { m_iAccessLevel = iLev; }

	inline bool isAutoPathOn () { return m_bAutoPaths; }
	inline void setAutoPath ( bool bOn ) { m_bAutoPaths = bOn; }
	inline bool isPathWaypointOn () { return m_bPathWaypointOn; }
	inline void setPathWaypoint ( bool bOn ) { m_bPathWaypointOn = bOn; }

	inline void setPathFrom ( int iWpt ) { m_iPathFrom = iWpt; }
	inline void setPathTo ( int iWpt ) { m_iPathTo = iWpt; }

	inline int getPathFrom () { return m_iPathFrom; }
	inline int getPathTo () { return m_iPathTo; }

	inline const char *getSteamID () { return m_szSteamID; }
	const char *getName ();

	void updateCurrentWaypoint ();

	void clientActive ();

	void setDebug ( int iLevel, bool bSet ) { if ( bSet ) { m_iDebugLevels |= iLevel; } else { m_iDebugLevels &= ~iLevel; } }
	bool isDebugOn ( int iLevel ) { return (m_iDebugLevels & iLevel)>0; }
	void clearDebug ( ) { m_iDebugLevels = 0; }
	bool isDebugging () { return (m_iDebugLevels != 0); }

	inline void setDebugBot ( CBot *pBot ) { m_pDebugBot = pBot; }	
	inline bool isDebuggingBot ( CBot *pBot ) { return m_pDebugBot == pBot; }

	void think ();

	inline void setDrawType ( unsigned short int iType ) { m_iWaypointDrawType = iType; }
	inline unsigned short int getDrawType () { return m_iWaypointDrawType; }
private:
	edict_t *m_pPlayer;
	// steam id
	char *m_szSteamID;
	// is drawing waypoints ON for this player
	bool m_bWaypointOn;
	// player editing this waypoint
	int m_iCurrentWaypoint;

	int m_iPathFrom;
	int m_iPathTo;

	int m_iAccessLevel;

	// auto path waypointing
	bool m_bAutoPaths;
	bool m_bPathWaypointOn;
	unsigned short int m_iWaypointDrawType;

	unsigned int m_iDebugLevels;

	IPlayerInfo *m_pPlayerInfo;

	CBot *m_pDebugBot;

	// TODO: tooltips queue
	// vector<CToolTip*> tooltips
};

class CClients
{
public:
	// called when player joins
	static void clientConnected ( edict_t *pPlayer );
	static void clientDisconnected ( edict_t *pPlayer );
	// player starts game
	static void clientActive ( edict_t *pPlayer );
	// get index in array
	static int slotOfEdict ( edict_t *pPlayer );
	static void init ( edict_t *pPlayer );
	static CClient *get ( int iIndex ) { return &m_Clients[iIndex]; }
	static CClient *get ( edict_t *pPlayer ) { return &m_Clients[slotOfEdict(pPlayer)]; }
	static void setListenServerClient ( CClient *pClient ) { m_pListenServerClient = pClient; }
	static bool isListenServerClient ( CClient *pClient ) { return m_pListenServerClient == pClient; }
	static bool noListenServerClient () { return m_pListenServerClient == NULL; }
	static void clientThink ();
	static bool clientsDebugging ();
	static void clientDebugMsg ( int iLev, const char *szMsg, CBot *pBot = NULL );
	static CClient *findClientBySteamID ( char *szSteamID );

private:
	static CClient m_Clients[MAX_PLAYERS];
	static CClient *m_pListenServerClient;
	static bool m_bClientsDebugging;
};
#endif