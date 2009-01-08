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
#ifndef __RCBOT_NAVIGATOR_H__
#define __RCBOT_NAVIGATOR_H__

#include <vector>
#include <queue>
using namespace std;

#include "bot.h"
#include "bot_waypoint.h"

#include "bot_belief.h"
#include "bot_genclass.h"

class CNavMesh;
class CWaypointVisibilityTable;

class IBotNavigator
{
public:
	virtual void init () = 0;

	// returns true when working out route finishes, not if successful
	virtual bool workRoute ( Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false ) = 0;

	virtual void rollBackPosition () = 0;

	virtual bool hasNextPoint () = 0;

	virtual Vector getNextPoint () = 0;

	virtual void updatePosition () = 0;

	virtual void freeMapMemory () = 0;		

	virtual void freeAllMemory () = 0;

	virtual bool routeFound () = 0;

	inline Vector getGoalOrigin () { return m_vGoal; }

	virtual void belief ( Vector origin, Vector facing, float fBelief, float fStrength, BotBelief iType ) = 0;

	// nearest cover position to vOrigin only
	virtual bool getCoverPosition ( Vector vCoverOrigin, Vector *vCover ) = 0;
	// nearest cover postion to both vectors
	virtual bool getHideSpotPosition ( Vector vCoverOrigin, Vector *vCover ) = 0;

	static const int MAX_PATH_TICKS = 200;

protected:
	Vector m_vGoal;
};

#define FL_ASTAR_CLOSED		1
#define FL_ASTAR_PARENT		2
#define FL_ASTAR_OPEN		4
#define FL_HEURISTIC_SET	8

class AStarNode
{
public:
	AStarNode() { memset(this,0,sizeof(AStarNode)); }
	///////////////////////////////////////////////////////
	inline void close () { setFlag(FL_ASTAR_CLOSED); }
	inline void unClose () { removeFlag(FL_ASTAR_CLOSED); }
	inline bool isOpen () { return hasFlag(FL_ASTAR_OPEN); }
	inline void unOpen () { removeFlag(FL_ASTAR_OPEN); }
	inline bool isClosed () { return hasFlag(FL_ASTAR_CLOSED); }
	inline void open () { setFlag(FL_ASTAR_OPEN); }
	//////////////////////////////////////////////////////	
	inline void setHeuristic ( float fHeuristic ) { m_fHeuristic = fHeuristic; setFlag(FL_HEURISTIC_SET); }
	inline bool heuristicSet () { return hasFlag(FL_HEURISTIC_SET); }
	inline const float getHeuristic () { return m_fHeuristic; } const
	
	////////////////////////////////////////////////////////
	inline void setFlag ( int iFlag ) { m_iFlags |= iFlag; }
	inline bool hasFlag ( int iFlag ) { return ((m_iFlags & iFlag) == iFlag); }
	inline void removeFlag ( int iFlag ) { m_iFlags &= ~iFlag; }
	/////////////////////////////////////////////////////////
	inline int getParent () { if ( hasFlag(FL_ASTAR_PARENT) ) return m_iParent; else return -1; }
	inline void setParent ( short int iParent ) 
	{ 
		m_iParent = iParent; 

		if ( m_iParent == -1 )
			removeFlag(FL_ASTAR_PARENT); // no parent
		else
			setFlag(FL_ASTAR_PARENT);
	}
	////////////////////////////////////////////////////////
	inline const float getCost () { return m_fCost; } const
	inline void setCost ( float fCost ) { m_fCost = fCost; }
	////////////////////////////////////////////////////////
	// for comparison
	bool betterCost ( AStarNode *other ) const
	{
		return (m_fCost+m_fHeuristic) < (other->getCost() + other->getHeuristic());
	}
	void setWaypoint ( int iWpt ) { m_iWaypoint = iWpt; }
	inline int getWaypoint () { return m_iWaypoint; }
private:
	float m_fCost;
	float m_fHeuristic;
	unsigned char m_iFlags;
	short int m_iParent;
	int m_iWaypoint;
};

class CWaypointNavigator : public IBotNavigator
{
public:
	CWaypointNavigator ( CBot *pBot ) 
	{ 
		init();
		m_pBot = pBot; 
	}

	void init ();

	bool workRoute ( Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false  );

	Vector getNextPoint ();

	void updatePosition ();

	bool hasNextPoint ();

	void freeMapMemory ();

	void freeAllMemory ();

	bool routeFound ();

	void rollBackPosition ();

	void open ( AStarNode *pNode );

	AStarNode *nextNode ();

	Vector getCoverOrigin ( Vector vCover );

	void clearOpenList ();
	
	void belief ( Vector origin, Vector facing, float fBelief, float fStrength, BotBelief iType );

	// nearest cover position to vOrigin only
	bool getCoverPosition ( Vector vCoverOrigin, Vector *vCover );
	// nearest cover postion to both vectors
	bool getHideSpotPosition ( Vector vCoverOrigin, Vector *vCover );
private:
	CBot *m_pBot;

	//CWaypointVisibilityTable *m_pDangerNodes;

	int m_iCurrentWaypoint;
	int m_iNextWaypoint;
	int m_iGoalWaypoint;
	bool m_bWorkingRoute;

	dataStack<int> m_currentRoute;

	int m_iLastFailedWpt;

	AStarNode paths[CWaypoints::MAX_WAYPOINTS];
	AStarNode *curr;
	AStarNode *succ;

	dataUnconstArray<int> m_iFailedGoals;

	float m_fBelief [CWaypoints::MAX_WAYPOINTS];

	vector<AStarNode*> m_theOpenList;
};

class CNavMeshNavigator : public IBotNavigator
{
public:
	virtual bool workRoute ( Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false  );

	virtual Vector getNextPoint ();

	virtual void updatePosition ();

	void freeMapMemory ();

	void freeAllMemory ();

	bool routeFound ();

	bool hasNextPoint ();

	void rollBackPosition () {};

	void init ();

	void belief ( Vector origin, Vector facing, float fBelief, float fStrength, BotBelief iType );

	//void rememberEnemyPosition ( Vector vOrigin );

	//Vector getEnemyPositionPinchPoint ( Vector vOrigin );
private:
	CNavMesh *m_pNavMesh;
};

#endif