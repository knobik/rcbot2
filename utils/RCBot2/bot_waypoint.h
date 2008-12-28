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
#ifndef __RCBOT_WAYPOINT_H__
#define __RCBOT_WAYPOINT_H__

#include <stdio.h>

#include "bot.h"
#include "bot_genclass.h"
#include "bot_client.h"
#include "bot_wpt_color.h"

class CWaypointVisibilityTable;
class CClient;

class CWaypointHeader
{
public:
	char szFileType[16];
	char szMapName[64];
	int iVersion;
	int iNumWaypoints;
	int iFlags;
};

#define DRAWTYPE_EFFECTS     0
#define DRAWTYPE_DEBUGENGINE 1

class CWaypoint;

class CWaypointType
{
public:

	CWaypointType ( int iBit, const char *szName, const char *szDescription, WptColor vColour );

	inline const char *getName () { return m_szName; }
	inline const char *getDescription () { return m_szDescription; }

	inline bool isBitsInFlags ( int iFlags ) { return (iFlags & m_iBit)==m_iBit; }
	inline int getBits () { return m_iBit; }
	inline void setMods ( int iMods ){ m_iMods = iMods; }// input bitmask of mods (32 max)
	inline bool forMod ( int iMod ) { return ((1<<iMod)&m_iMods)==(1<<iMod); }
	inline WptColor getColour () { return m_vColour; }

	//virtual void giveTypeToWaypoint ( CWaypoint *pWaypoint );
	//virtual void removeTypeFromWaypoint ( CWaypoint *pWaypoint );

private:
	int m_iMods; // mods allowed
	int m_iBit; // bits used
	char *m_szName; // e.g. "jump"/"ladder"
	char *m_szDescription; // e.g. "will jump here"/"will climb here"
	WptColor m_vColour;
};
/*
class CCrouchWaypointType : public CWaypointType
{
public:
	CCrouchWaypointType();
    
	void giveTypeToWaypoint ( CWaypoint *pWaypoint );
	void removeTypeFromWaypoint ( CWaypoint *pWaypoint );
};*/

#define TEAM_BLUE 3
#define TEAM_RED 2

class CWaypointTypes
{
public:

	static const int W_FL_NONE = 0;
	static const int W_FL_JUMP = 1;
	static const int W_FL_CROUCH = 2;
	static const int W_FL_UNREACHABLE = 4;
	static const int W_FL_LADDER = 8;
	static const int W_FL_FLAG = 16;
	static const int W_FL_CAPPOINT = 32;
	static const int W_FL_NOBLU = 64;
	static const int W_FL_NORED = 128;
	static const int W_FL_HEALTH = 256;
	static const int W_FL_OPENS_LATER = 512;

	static void setup ();

	static void addType ( CWaypointType *type );

	static void printInfo ( CWaypoint *pWpt, edict_t *pPrintTo );

	static void displayTypesMenu ( edict_t *pPrintTo );
	
	static CWaypointType *getType( const char *szType );

	static void showTypesOnConsole( edict_t *pPrintTo );

	static void selectedType ( CClient *pClient );

	static void freeMemory ();

	static WptColor getColour ( int iFlags );
private:
	static vector<CWaypointType*> m_Types;
};

class CWaypoint
{
public:
	//static const int MAX_PATHS = 8;
	// Waypoint flags (up to 32)


	static const int WAYPOINT_HEIGHT = 72;
	static const int WAYPOINT_WIDTH = 8;
	static const int PATHWAYPOINT_WIDTH = 4;

	CWaypoint ()
	{
		m_thePaths.Init();
		init();
//		m_iId = -1;
	}

	CWaypoint ( Vector vOrigin, int iFlags = 0, int iYaw = 0 )
	{
		m_thePaths.Clear();
		init();
		m_iFlags = iFlags;
		m_vOrigin = vOrigin;		
		m_bUsed = true;
		setAim(iYaw);
//		m_iId = iId;
	}

	inline void setAim ( int iYaw )
	{
		m_iAimYaw = iYaw;
	}

	inline float getAimYaw ()
	{
		return (float)m_iAimYaw;
	}

	void init ();

	void addFlag ( int iFlag )
	{
		m_iFlags |= iFlag;
	}

	void removeFlag ( int iFlag )
	{
		m_iFlags &= ~iFlag;
	}

	// removes all waypoint flags
	void removeFlags ()
	{
		m_iFlags = 0;
	}

	bool hasFlag ( int iFlag )
	{
		return (m_iFlags & iFlag) == iFlag;
	}

	// show info to player
	void info ( edict_t *pEdict );

	// methods
    void touched ();

	void draw ( edict_t *pEdict, bool bDrawPaths, unsigned short int iDrawType );

	bool addPathTo ( int iWaypointIndex );
	void removePathTo ( int iWaypointIndex );

	bool isUsed ()
	{
		return m_bUsed;
	}

	//bool touched ( edict_t *pEdict );
	bool touched ( Vector vOrigin );

	void botTouch ( CBot *pBot );

	void freeMapMemory ()
	{
		m_thePaths.Clear();//Destroy();
	}

	void drawPaths ( edict_t *pEdict, unsigned short int iDrawType );

	void drawPathBeam ( CWaypoint *to, unsigned short int iDrawType );

	void setUsed ( bool bUsed )
	{
		m_bUsed = bUsed;
	}

	inline void clearPaths ();

	inline float distanceFrom ( CWaypoint *other )
	{
		return distanceFrom(other->getOrigin());
	}

	float distanceFrom ( Vector vOrigin );

	Vector getOrigin ()
	{
		return m_vOrigin;
	}

	int numPaths ();

	int getPath ( int i );

	void load ( FILE *bfp );

	void save ( FILE *bfp );

	int getFlags ()
	{
		return m_iFlags;
	}

	bool forTeam ( int iTeam )
	{
		if ( iTeam == TEAM_BLUE )
			return (m_iFlags & CWaypointTypes::W_FL_NOBLU)==0;
		else if ( iTeam == TEAM_RED )
			return (m_iFlags & CWaypointTypes::W_FL_NORED)==0;

		return true;	
	}

private:
	Vector m_vOrigin;
	// aim of vector (used with certain waypoint types)
	int m_iAimYaw;
	int m_iFlags;
	// not deleted
	bool m_bUsed;
	// paths to other waypoints
	dataUnconstArray<int> m_thePaths;
};

class CWaypoints
{
public:
	static const int MAX_WAYPOINTS = 1024;
	static const int WAYPOINT_VERSION = 1;

	static const int W_FILE_FL_VISIBILITY = 1;

	static void init ();

	static int getWaypointIndex ( CWaypoint *pWpt )
	{
		return ((int)pWpt - (int)m_theWaypoints)/sizeof(CWaypoint);
	}

	static void drawWaypoints ( CClient *pClient );

	static void addWaypoint ( CClient *pClient );

	static void addWaypoint ( edict_t *pPlayer, Vector vOrigin, int iFlags = CWaypointTypes::W_FL_NONE, bool bAutoPath = false, int iYaw = 0 );

	static void removeWaypoint ( int iIndex );

	static int numWaypoints ();

	static int freeWaypointIndex ();

	static void deletePathsTo ( int iWpt );
	static void deletePathsFrom ( int iWpt );

	static CWaypoint *getWaypoint ( int iIndex );	

	// save waypoints
	static bool save ( bool bVisiblityMade );
	// load waypoints
	static bool load ();

	static bool validWaypointIndex ( int iIndex );

	static void precacheWaypointTexture ();

	static int waypointTexture () { return m_iWaypointTexture; }

	static void deleteWaypoint ( int iIndex );

	static void freeMemory ();

	static int nearestWaypointGoal ( int iFlags, Vector &origin, float fDist, int iTeam = 0 );
	static int randomWaypointGoal ( int iFlags, int iTeam = 0 );
	static int randomFlaggedWaypoint (int iTeam = 0);

	static CWaypointVisibilityTable *getVisiblity () { return m_pVisibilityTable; }
	static void setupVisibility ();
	static CWaypoint *getPinchPointFromWaypoint ( Vector vPlayerOrigin, Vector vPinchOrigin );

private:
	static CWaypoint m_theWaypoints[MAX_WAYPOINTS];	
	static int m_iNumWaypoints;
	static float m_fNextDrawWaypoints;
	static int m_iWaypointTexture;
	static CWaypointVisibilityTable *m_pVisibilityTable;
};

#endif