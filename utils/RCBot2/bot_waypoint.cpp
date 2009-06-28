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
#include <vector>
using namespace std;

#include "filesystem.h"
#include "interface.h"
#include "engine/iserverplugin.h"
#include "iplayerinfo.h"
#include "vplane.h"
#include "eiface.h"
#include "convar.h"
#include "ndebugoverlay.h"

#include "bot.h"
#include "bot_globals.h"
#include "bot_client.h"
#include "bot_navigator.h"
#include "bot_waypoint.h"
#include "bot_waypoint_locations.h"
#include "bot_waypoint_visibility.h"
#include "bot_wpt_color.h"
#include "bot_profile.h"
#include "bot_schedule.h"
#include "bot_fortress.h"

#include "bot_wpt_dist.h"

int CWaypoints::m_iNumWaypoints = 0;
CWaypoint CWaypoints::m_theWaypoints[CWaypoints::MAX_WAYPOINTS];
float CWaypoints::m_fNextDrawWaypoints = 0;
int CWaypoints::m_iWaypointTexture = 0;
CWaypointVisibilityTable * CWaypoints::m_pVisibilityTable = NULL;
vector<CWaypointType*> CWaypointTypes::m_Types;

extern IVDebugOverlay *debugoverlay;

///////////////////////////////////////////////////////////////
// initialise
void CWaypointNavigator :: init ()
{
	m_pBot = NULL;

	m_vOffset = Vector(0,0,0);
	m_bOffsetApplied = false;

	m_iCurrentWaypoint = -1;
	m_iNextWaypoint = -1;
	m_iGoalWaypoint = -1;

	m_currentRoute.Destroy();

	m_iLastFailedWpt = -1;
	m_bWorkingRoute = false;

	Q_memset(m_fBelief,0,sizeof(int)*CWaypoints::MAX_WAYPOINTS);

	m_iFailedGoals.Destroy();//.clear();//Destroy();
}

// get the covering waypoint vector vCover
bool CWaypointNavigator :: getCoverPosition ( Vector vCoverOrigin, Vector *vCover )
{
	int iWpt;

	iWpt = CWaypointLocations::GetCoverWaypoint(m_pBot->getOrigin(),vCoverOrigin,NULL);

	CWaypoint *pWaypoint = CWaypoints::getWaypoint(iWpt);
	
	if ( pWaypoint == NULL )
		return false;
	
	*vCover = pWaypoint->getOrigin();

	return true;
}
#define MAX_BELIEF 1024.0f
// get belief nearest to current origin using waypoints to store belief
void CWaypointNavigator :: belief ( Vector vOrigin, Vector facing, float fBelief, float fStrength, BotBelief iType )
{
	dataUnconstArray<int> m_iVisibles;
	//int m_iVisiblePoints[CWaypoints::MAX_WAYPOINTS]; // make searching quicker

	CWaypointLocations::GetAllVisible(vOrigin,vOrigin,&m_iVisibles);
	CWaypointLocations::GetAllVisible(vOrigin,m_pBot->getEyePosition(),&m_iVisibles);

	for ( int i = 0; i < m_iVisibles.Size(); i ++ )
	{
		CWaypoint *pWpt = CWaypoints::getWaypoint(m_iVisibles[i]);
		int iWptIndex = CWaypoints::getWaypointIndex(pWpt);

		if ( iType == BELIEF_SAFETY )
		{
			if ( m_fBelief[iWptIndex] > -MAX_BELIEF)
				m_fBelief[iWptIndex] -= (fStrength / (sqrt((vOrigin-pWpt->getOrigin()).LengthSqr())))*fBelief;
		}
		else if ( iType == BELIEF_DANGER )
		{
			if ( m_fBelief[iWptIndex] < MAX_BELIEF )
				m_fBelief[iWptIndex] += (fStrength / (sqrt((vOrigin-pWpt->getOrigin()).LengthSqr())))*fBelief;
		}
	}
}
// get the hide spot position (vCover) from origin vCoverOrigin
bool CWaypointNavigator :: getHideSpotPosition ( Vector vCoverOrigin, Vector *vCover )
{
	int iWpt;

	if ( m_pBot->hasGoal() )
		iWpt = CWaypointLocations::GetCoverWaypoint(m_pBot->getOrigin(),vCoverOrigin,NULL,m_pBot->getGoalOrigin());
	else
		iWpt = CWaypointLocations::GetCoverWaypoint(m_pBot->getOrigin(),vCoverOrigin,NULL);

	CWaypoint *pWaypoint = CWaypoints::getWaypoint(iWpt);
	
	if ( pWaypoint == NULL )
		return false;
	
	*vCover = pWaypoint->getOrigin();

	return true;
}
// AStar Algorithm : open a waypoint
void CWaypointNavigator :: open ( AStarNode *pNode )
{ 
	if ( !pNode->isOpen() )
	{
		pNode->open();
		m_theOpenList.push_back(pNode);
	}
}
// AStar Algorithm : get the waypoint with lowest cost
AStarNode *CWaypointNavigator :: nextNode ()
{
	AStarNode *pNode = NULL;

	if ( !m_theOpenList.empty() )
	{
		// Find node with least cost
		float mincost = 0;
		float cost;
		unsigned int i;
		AStarNode *pTemp;

		// Safest method, but takes some cpu
		for ( i = 0; i < m_theOpenList.size(); i ++ )
		{
			pTemp = m_theOpenList[i];

			cost = pTemp->getCost() + pTemp->getHeuristic();

			if ( !pNode || (cost < mincost) )
			{
				pNode = pTemp;
				mincost = cost;
			}
		}

		if ( pNode )
		{
			
			vector<AStarNode*> temp;

			pNode->unOpen();

			for ( i = 0; i < m_theOpenList.size(); i ++ )
			{
				if ( m_theOpenList[i] != pNode )
					temp.push_back(m_theOpenList[i]);
			}

			m_theOpenList.clear();

			m_theOpenList = temp;
			
		}
	}

	return pNode;
}

// clears the AStar open list
void CWaypointNavigator :: clearOpenList ()
{
	for ( unsigned int i = 0; i < m_theOpenList.size(); i ++ )
		m_theOpenList[i]->unOpen();

	m_theOpenList.clear();
}

void CWaypointNavigator :: failMove ()
{
	m_iLastFailedWpt = m_iCurrentWaypoint;

	if ( !m_iFailedGoals.IsMember(m_iGoalWaypoint) )
	{
		m_iFailedGoals.Add(m_iGoalWaypoint);
		m_fNextClearFailedGoals = engine->Time() + randomFloat(8.0f,30.0f);
	}
}

float CWaypointNavigator :: distanceTo ( Vector vOrigin )
{
	int iGoal;

	if ( m_iCurrentWaypoint == -1 )
		m_iCurrentWaypoint = CWaypointLocations::NearestWaypoint(m_pBot->getOrigin(),CWaypointLocations::REACHABLE_RANGE,-1,true,false,true,NULL,false,m_pBot->getTeam());
	
	if ( m_iCurrentWaypoint != -1 )
	{
		iGoal = CWaypointLocations::NearestWaypoint(vOrigin,CWaypointLocations::REACHABLE_RANGE,-1,true,false,true,NULL,false,m_pBot->getTeam());

		if ( iGoal != -1 )
			return CWaypointDistances::getDistance(m_iCurrentWaypoint,iGoal);
	}
		
	return m_pBot->distanceFrom(vOrigin);
}

float CWaypointNavigator :: distanceTo ( CWaypoint *pWaypoint )
{
	return distanceTo(pWaypoint->getOrigin());
}

// find route using A* algorithm
bool CWaypointNavigator :: workRoute ( Vector vFrom, Vector vTo, bool *bFail, bool bRestart, bool bNoInterruptions )
{
	if ( bRestart )
	{
		*bFail = false;

		m_bWorkingRoute = true;
		m_iGoalWaypoint = CWaypointLocations::NearestWaypoint(vTo,CWaypointLocations::REACHABLE_RANGE,m_iLastFailedWpt,true,false,true,&m_iFailedGoals,false,m_pBot->getTeam());

		if ( m_iGoalWaypoint == -1 )
		{
			*bFail = true;
			m_bWorkingRoute = false;
			return true;
		}
		
		m_vPreviousPoint = vFrom;
		m_iCurrentWaypoint = CWaypointLocations::NearestWaypoint(vFrom,CWaypointLocations::REACHABLE_RANGE,m_iLastFailedWpt,true,false,true,NULL,false,m_pBot->getTeam());

		if ( m_iCurrentWaypoint == -1 )
		{
			*bFail = true;
			m_bWorkingRoute = false;
			return true;
		}

		// reset
		m_iLastFailedWpt = -1;

		clearOpenList();
		Q_memset(paths,0,sizeof(AStarNode)*CWaypoints::MAX_WAYPOINTS);

		AStarNode *curr = &paths[m_iCurrentWaypoint];
		curr->setWaypoint(m_iCurrentWaypoint);
		curr->setHeuristic(m_pBot->distanceFrom(vTo));
		open(curr);
	}
/////////////////////////////////
	if ( m_iGoalWaypoint == -1 )
	{
		*bFail = true;
		m_bWorkingRoute = false;
		return true;
	}
	if ( m_iCurrentWaypoint == -1 )
	{
		*bFail = true;
		m_bWorkingRoute = false;
		return true;
	}
///////////////////////////////

	int iLoops = 0;
	int iMaxLoops = this->m_pBot->getProfile()->getPathTicks();//IBotNavigator::MAX_PATH_TICKS;
	
	if ( bNoInterruptions )
		iMaxLoops *= 2; // "less" interruptions, however dont want to hang, or use massive cpu

	int iCurrentNode; // node selected

	bool bFoundGoal = false;

	CWaypoint *currWpt;
	CWaypoint *succWpt;

	float fCost;
	float fOldCost;

	Vector vOrigin;

	int iPath;
	int iMaxPaths;
	int iSucc;

	int iLastNode = -1;

	while ( !bFoundGoal && !m_theOpenList.empty() && (iLoops < iMaxLoops) )
	{
		iLoops ++;

		curr = this->nextNode();

		if ( !curr )
			break;

		iCurrentNode = curr->getWaypoint();
		
		bFoundGoal = (iCurrentNode == m_iGoalWaypoint);

		if ( bFoundGoal )
			break;

		// can get here now
		m_iFailedGoals.Remove(iCurrentNode);//.Remove(iCurrentNode);

		currWpt = CWaypoints::getWaypoint(iCurrentNode);

		vOrigin = currWpt->getOrigin();

		iMaxPaths = currWpt->numPaths();

		succ = NULL;

		for ( iPath = 0; iPath < iMaxPaths; iPath ++ )
		{
			iSucc = currWpt->getPath(iPath);

			if ( iSucc == iLastNode )
				continue;
			if ( iSucc == iCurrentNode ) // argh?
				continue;			

			succ = &paths[iSucc];
			succWpt = CWaypoints::getWaypoint(iSucc);

			if ( !m_pBot->canGotoWaypoint(vOrigin,succWpt) )
				continue;

			fCost = curr->getCost()+(succWpt->distanceFrom(vOrigin));

			if ( succ->isOpen() || succ->isClosed() )
			{
				if ( succ->getParent() != -1 )
				{
					fOldCost = succ->getCost();

					if ( fCost >= fOldCost )
						continue; // ignore route
				}
				else
					continue;
			}

			succ->unClose();

			if ( !succ->isOpen() )
			{
				open(succ);
			}

			succ->setParent(iCurrentNode);
			succ->setCost(fCost+m_fBelief[iSucc]);	
			succ->setWaypoint(iSucc);

			if ( !succ->heuristicSet() )		
				succ->setHeuristic(m_pBot->distanceFrom(succWpt->getOrigin())+succWpt->distanceFrom(vTo));			
		}

		curr->close(); // close chosen node

		iLastNode = iCurrentNode;		
	}
	/////////
	if ( iLoops == iMaxLoops )
	{
		//*bFail = true;
		
		return false; // not finished yet, wait for next iteration
	}

	m_bWorkingRoute = false;
	
	clearOpenList(); // finished

	if ( !bFoundGoal )
	{
		*bFail = true;

		if ( !m_iFailedGoals.IsMember(m_iGoalWaypoint) )
		{
			m_iFailedGoals.Add(m_iGoalWaypoint);
			m_fNextClearFailedGoals = engine->Time() + randomFloat(8.0f,30.0f);
		}

		return true; // waypoint not found but searching is complete
	}

	iCurrentNode = m_iGoalWaypoint;

	m_currentRoute.Destroy();

	iLoops = 0;

	int iNumWaypoints = CWaypoints::numWaypoints();
	float fDistance = 0.0;
	int iParent;

	while ( (iCurrentNode != -1) && (iCurrentNode != m_iCurrentWaypoint ) && (iLoops <= iNumWaypoints) )
	{
		iLoops++;

		m_currentRoute.Push(iCurrentNode);

		iParent = paths[iCurrentNode].getParent();

		// crash bug fix
		if ( iParent != -1 )
			fDistance += (CWaypoints::getWaypoint(iCurrentNode)->getOrigin() - CWaypoints::getWaypoint(iParent)->getOrigin()).Length();

		iCurrentNode = iParent;
	}

	CWaypointDistances::setDistance(m_iCurrentWaypoint,m_iGoalWaypoint,fDistance);

	// erh??
	if ( iLoops > iNumWaypoints )
	{
		m_currentRoute.Destroy();
		*bFail = true;
	}
	else
		m_vGoal = CWaypoints::getWaypoint(m_iGoalWaypoint)->getOrigin();

    return true; 
}
// if bot has a current position to walk to return the boolean
bool CWaypointNavigator :: hasNextPoint ()
{
	return m_iCurrentWaypoint != -1;
}
// return the vector of the next point
Vector CWaypointNavigator :: getNextPoint ()
{
	//CWaypoint *pWpt = CWaypoints::getWaypoint(*m_currentRoute.GetHeadInfoPointer());
	
	//return pWpt->getOrigin();

	return CWaypoints::getWaypoint(m_iCurrentWaypoint)->getOrigin();
}

bool CWaypointNavigator :: getNextRoutePoint ( Vector *point )
{
	if ( !m_currentRoute.IsEmpty() )
	{
		int *head = m_currentRoute.GetHeadInfoPointer();

		if ( head && (*head!= -1))
		{
			*point = CWaypoints::getWaypoint(*head)->getOrigin();
			return true;
		}
	}

	return false;
}

bool CWaypointNavigator :: canGetTo ( Vector vOrigin )
{
	int iwpt = CWaypointLocations::NearestWaypoint(vOrigin,100,-1,true,false,true,NULL,false,m_pBot->getTeam());

	if ( iwpt >= 0 )
	{
		if ( m_iFailedGoals.IsMember(iwpt) )
			return false;
	}
	else
		return false;

	return true;
}

void CWaypointNavigator :: rollBackPosition ()
{
	m_vPreviousPoint = m_pBot->getOrigin();
	m_iCurrentWaypoint = CWaypointLocations::NearestWaypoint(m_vPreviousPoint,CWaypointLocations::REACHABLE_RANGE,m_iLastFailedWpt,true,false,true,NULL,false,m_pBot->getTeam());

	while ( !m_currentRoute.IsEmpty() ) // reached goal!!
	{		
		if ( m_iCurrentWaypoint == m_currentRoute.Pop() )
		{
			if ( !m_currentRoute.IsEmpty() )
				m_iCurrentWaypoint = m_currentRoute.Pop();
		}
	}
	// find waypoint in route
}
// update the bots current walk vector
void CWaypointNavigator :: updatePosition ()
{
	static Vector vWptOrigin;
	static float fRadius;

	QAngle aim;
	Vector vaim;

	if ( m_iCurrentWaypoint == -1 ) // invalid
	{
		m_pBot->stopMoving();	
		m_bOffsetApplied = false;
		return;
	}

	CWaypoint *pWaypoint = CWaypoints::getWaypoint(m_iCurrentWaypoint);

	if ( pWaypoint == NULL )
	{
		m_bOffsetApplied = false;
		return;
	}

	aim = QAngle(0,pWaypoint->getAimYaw(),0);
	AngleVectors(aim,&vaim);

	fRadius = pWaypoint->getRadius();

	vWptOrigin = pWaypoint->getOrigin();

	if ( !m_bWorkingRoute )
	{
		if ( pWaypoint->touched(m_pBot->getOrigin(),m_vOffset) )
		{
			m_pBot->touchedWpt(pWaypoint);

			m_bOffsetApplied = false;

			if ( m_currentRoute.IsEmpty() ) // reached goal!!
			{
				m_vPreviousPoint = m_pBot->getOrigin();
				m_iCurrentWaypoint = -1;

				if ( m_pBot->getSchedule()->hasSchedule(SCHED_RUN_FOR_COVER) )
					m_pBot->reachedCoverSpot();
			}
			else
			{
				m_vPreviousPoint = m_pBot->getOrigin();
				m_iCurrentWaypoint = m_currentRoute.Pop();

				if ( m_iCurrentWaypoint != -1 )
				{
					vWptOrigin = CWaypoints::getWaypoint(m_iCurrentWaypoint)->getOrigin();
				}
			}
		}
	}
	else
		m_bOffsetApplied = false;

	if ( pWaypoint->hasFlag(CWaypointTypes::W_FL_CROUCH) )
	{
		m_pBot->duck(true);
	}

	if ( !m_bOffsetApplied )
	{
		if ( fRadius > 0 )
			m_vOffset = Vector(randomFloat(-fRadius,fRadius),randomFloat(-fRadius,fRadius),0);
		else
			m_vOffset = Vector(0,0,0);

		m_bOffsetApplied = true;
	}

	// fix for bots not finding goals
	if ( m_fNextClearFailedGoals && ( m_fNextClearFailedGoals < engine->Time() ) )
	{
		m_iFailedGoals.Destroy();
		m_fNextClearFailedGoals = 0;
	}

	m_pBot->setMoveTo(vWptOrigin+m_vOffset);
	m_pBot->setAiming(vWptOrigin+(vaim*1024));
}

// free up memory
void CWaypointNavigator :: freeMapMemory ()
{
	m_currentRoute.Destroy();
	m_iFailedGoals.Destroy();//.clear();//Destroy();
}

void CWaypointNavigator :: freeAllMemory ()
{
	freeMapMemory();
}

bool CWaypointNavigator :: routeFound ()
{
	return !m_currentRoute.IsEmpty();
}

/////////////////////////////////////////////////////////

// draw paths from this waypoint (if waypoint drawing is on)
void CWaypoint :: drawPaths ( edict_t *pEdict, unsigned short int iDrawType )
{
	int iPaths;
	int iWpt;
	CWaypoint *pWpt;

	iPaths = numPaths();

	for ( int i = 0; i < iPaths; i ++ )
	{
		iWpt = getPath(i);

		pWpt = CWaypoints::getWaypoint(iWpt);

		drawPathBeam(pWpt,iDrawType);
	}
}
// draws one path beam
void CWaypoint :: drawPathBeam ( CWaypoint *to, unsigned short int iDrawType )
{
	switch ( iDrawType )
	{
	case DRAWTYPE_EFFECTS:
		g_pEffects->Beam( m_vOrigin, to->getOrigin(), CWaypoints::waypointTexture(), 
		0, 0, 1,
		1, PATHWAYPOINT_WIDTH, PATHWAYPOINT_WIDTH, 255, 
		1, 200, 200, 200, 200, 10);	
		break;
	case DRAWTYPE_DEBUGENGINE:
		debugoverlay->AddLineOverlay (m_vOrigin, to->getOrigin(), 200,200,200, false, 1);
		break;
	}
}
/*
bool CWaypoint :: touched ( edict_t *pEdict )
{
	return touched(pEdict->m_pNetworkable->GetPVSInfo()->
}*/
// checks if a waypoint is touched
bool CWaypoint :: touched ( Vector vOrigin, Vector vOffset)
{
	if ( (vOrigin-(m_vOrigin+vOffset)).Length2D() <= 40 )
	{
		return fabs(vOrigin.z-m_vOrigin.z) < 48;
	}

	return false;
}
// get the colour of this waypoint in WptColor format
WptColor CWaypointTypes ::getColour ( int iFlags )
{
	WptColor colour = WptColor(0,0,255); // normal waypoint

	bool bNoColour = true;

	for ( unsigned int i = 0; i < m_Types.size(); i ++ )
	{
		if ( m_Types[i]->isBitsInFlags(iFlags) )
		{
			if ( bNoColour )
			{
				colour = m_Types[i]->getColour();
				bNoColour = false;
			}
			else
				colour.mix(m_Types[i]->getColour());
		}
	}

	return colour;
}
// draw this waypoint
void CWaypoint :: draw ( edict_t *pEdict, bool bDrawPaths, unsigned short int iDrawType )
{
	float fHeight = WAYPOINT_HEIGHT;

	QAngle qAim;
	Vector vAim;
	
	WptColor colour = CWaypointTypes::getColour(m_iFlags);

	//////////////////////////////////////////

	unsigned char r = (unsigned char)colour.r;
	unsigned char g = (unsigned char)colour.g;
	unsigned char b = (unsigned char)colour.b;
	unsigned char a = (unsigned char)colour.a;

	qAim = QAngle(0,m_iAimYaw,0);

	AngleVectors(qAim,&vAim);


	// top + bottom heights = fHeight
	fHeight /= 2;

	switch ( iDrawType )
	{
	case DRAWTYPE_DEBUGENGINE:
		// draw waypoint
		debugoverlay->AddLineOverlay (m_vOrigin - Vector(0,0,fHeight), m_vOrigin + Vector(0,0,fHeight), r,g,b, false, 1);

		// draw aim
		debugoverlay->AddLineOverlay (m_vOrigin + Vector(0,0,fHeight/2), m_vOrigin + Vector(0,0,fHeight/2) + vAim*48, r,g,b, false, 1);

		// draw radius
		if ( m_fRadius )
		{
			debugoverlay->AddBoxOverlay(m_vOrigin,Vector(-m_fRadius,-m_fRadius,-fHeight),Vector(m_fRadius,m_fRadius,fHeight),QAngle(0,0,0),255,255,255,50,1);
		}
		break;
	case DRAWTYPE_EFFECTS:
		g_pEffects->Beam( m_vOrigin - Vector(0,0,fHeight), m_vOrigin + Vector(0,0,fHeight), CWaypoints::waypointTexture(), 
			0, 0, 1,
			1, WAYPOINT_WIDTH, WAYPOINT_WIDTH, 255, 
			1, r, g, b, a, 10);//*/

		/*g_pEffects->Beam( m_vOrigin + Vector(0,0,fHeight/2), m_vOrigin + Vector(0,0,fHeight/2) + vAim*48 CWaypoints::waypointTexture(), 
			0, 0, 1,
			1, WAYPOINT_WIDTH/2, WAYPOINT_WIDTH/2, 255, 
			1, r, g, b, a, 10);//*/
		break;
	}

	/*
( const Vector &Start, const Vector &End, int nModelIndex, 
		int nHaloIndex, unsigned char frameStart, unsigned char frameRate,
		float flLife, unsigned char width, unsigned char endWidth, unsigned char fadeLength, 
		unsigned char noise, unsigned char red, unsigned char green,
		unsigned char blue, unsigned char brightness, unsigned char speed) = 0;
*/

	if ( bDrawPaths )
		drawPaths ( pEdict,iDrawType );
}
// clear the waypoints possible paths
void CWaypoint :: clearPaths ()
{
	m_thePaths.Clear();
}
// get the distance from this waypoint from vector position vOrigin
float CWaypoint :: distanceFrom ( Vector vOrigin )
{
	return VectorDistance((m_vOrigin - vOrigin));//.Length();
}
/////////////////////////////////////////////////////////////////////////////////////
// save waypoints (visibilitymade saves having to work out visibility again)
bool CWaypoints :: save ( bool bVisiblityMade )
{
	char filename[1024];

	CBotGlobals::buildFileName(filename,CBotGlobals::getMapName(),"waypoints",BOT_WAYPOINT_EXTENSION,true);

	FILE *bfp = CBotGlobals::openFile(filename,"wb");

	if ( bfp == NULL )
	{
		return false; // give up
	}

	int iSize = numWaypoints();

	// write header
	// ----
	CWaypointHeader header;

	int flags = 0;

	if ( bVisiblityMade )
		flags |= W_FILE_FL_VISIBILITY;

	//////////////////////////////////////////////
	header.iFlags = flags;
	header.iNumWaypoints = iSize;
	header.iVersion = WAYPOINT_VERSION;
	strcpy(header.szFileType,BOT_WAYPOINT_FILE_TYPE);
	strcpy(header.szMapName,CBotGlobals::getMapName());
	//////////////////////////////////////////////

	fwrite(&header,sizeof(CWaypointHeader),1,bfp);

	for ( int i = 0; i < iSize; i ++ )
	{
		CWaypoint *pWpt = &m_theWaypoints[i];

		// save individual waypoint and paths
		pWpt->save(bfp);
	}

	fclose(bfp);

	CWaypointDistances::reset();

	CWaypointDistances::save();

	return true;
}

// load waypoints
bool CWaypoints :: load ()
{
	char filename[1024];	

	CBotGlobals::buildFileName(filename,CBotGlobals::getMapName(),"waypoints",BOT_WAYPOINT_EXTENSION,true);

	FILE *bfp = CBotGlobals::openFile(filename,"rb");

	if ( bfp == NULL )
	{
		return false; // give up
	}

	CWaypointHeader header;

	// read header
	// -----------

	fread(&header,sizeof(CWaypointHeader),1,bfp);

	if ( !FStrEq(header.szFileType,BOT_WAYPOINT_FILE_TYPE) )
	{
		CBotGlobals::botMessage(NULL,0,"Error loading waypoints: File type mismatch");
		fclose(bfp);
		return false;
	}
	if ( header.iVersion > WAYPOINT_VERSION )
	{
		CBotGlobals::botMessage(NULL,0,"Error loading waypoints: Waypoint version too new");
		fclose(bfp);
		return false;
	}
	if ( !FStrEq(header.szMapName,CBotGlobals::getMapName()) )
	{
		CBotGlobals::botMessage(NULL,0,"Error loading waypoints: Map name mismatch");
		fclose(bfp);
		return false;
	}

	int iSize = header.iNumWaypoints;

	// ok lets read the waypoints
	// initialize
	
	CWaypoints::init();

	m_iNumWaypoints = iSize;

	bool bWorkVisibility = true;

	if ( header.iFlags & W_FILE_FL_VISIBILITY )
		bWorkVisibility = ( !m_pVisibilityTable->ReadFromFile() );

	for ( int i = 0; i < iSize; i ++ )
	{
		CWaypoint *pWpt = &m_theWaypoints[i];		

		pWpt->load(bfp,header.iVersion);

		if ( pWpt->isUsed() ) // not a deleted waypoint
		{
			// add to waypoint locations for fast searching and drawing
			CWaypointLocations::AddWptLocation(pWpt,i);
		}
	}

	fclose(bfp);

	m_pVisibilityTable->setWorkVisiblity(bWorkVisibility);

	if ( bWorkVisibility ) // say a message
		Msg(" *** No waypoint visibility file ***\n *** Working out waypoint visibility information... ***\n");

	CWaypointDistances::load();

	return true;
}

void CWaypoint :: init ()
{
	//m_thePaths.clear();
	m_iFlags = 0;
	m_vOrigin = Vector(0,0,0);
	m_bUsed = false; // ( == "deleted" )
	setAim(0);
	m_thePaths.Clear();
	m_iArea = 0;
	m_fRadius = 0;
}

void CWaypoint :: save ( FILE *bfp )
{
	fwrite(&m_vOrigin,sizeof(Vector),1,bfp);
	// aim of vector (used with certain waypoint types)
	fwrite(&m_iAimYaw,sizeof(int),1,bfp);
	fwrite(&m_iFlags,sizeof(int),1,bfp);
	// not deleted
	fwrite(&m_bUsed,sizeof(bool),1,bfp);

	int iPaths = numPaths();
	fwrite(&iPaths,sizeof(int),1,bfp);

	for ( int n = 0; n < iPaths; n ++ )
	{			
		int iPath = getPath(n);
		fwrite(&iPath,sizeof(int),1,bfp);		
	}

	if ( CWaypoints::WAYPOINT_VERSION >= 2 )
	{
		fwrite(&m_iArea,sizeof(int),1,bfp);
	}

	if ( CWaypoints::WAYPOINT_VERSION >= 3 ) 
	{
		fwrite(&m_fRadius,sizeof(float),1,bfp);
	}
}

void CWaypoint :: load ( FILE *bfp, int iVersion )
{
	int iPaths;

	fread(&m_vOrigin,sizeof(Vector),1,bfp);
	// aim of vector (used with certain waypoint types)
	fread(&m_iAimYaw,sizeof(int),1,bfp);
	fread(&m_iFlags,sizeof(int),1,bfp);
	// not deleted
	fread(&m_bUsed,sizeof(bool),1,bfp);	
	fread(&iPaths,sizeof(int),1,bfp);

	for ( int n = 0; n < iPaths; n ++ )
	{		
		int iPath;
		fread(&iPath,sizeof(int),1,bfp);
		addPathTo(iPath);
	}

	if ( iVersion >= 2 )
	{
		fread(&m_iArea,sizeof(int),1,bfp);
	}

	if ( iVersion >= 3 ) 
	{
		fread(&m_fRadius,sizeof(float),1,bfp);
	}
}
// draw waypoints to this client pClient
void CWaypoints :: drawWaypoints( CClient *pClient )
{
	float fTime = engine->Time();
	CWaypoint *pWpt;
	//////////////////////////////////////////
	// TODO
	// draw time currently part of CWaypoints
	// once we can send sprites to individual players
	// make draw waypoints time part of CClient
	if ( m_fNextDrawWaypoints > fTime )
		return;

	m_fNextDrawWaypoints = engine->Time() + 1.0f;
	/////////////////////////////////////////////////
	pClient->updateCurrentWaypoint();

	CWaypointLocations::DrawWaypoints(pClient->getPlayer(),pClient->getOrigin(),CWaypointLocations::REACHABLE_RANGE,false,pClient->getDrawType());

	if ( pClient->isPathWaypointOn() )
	{
		pWpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

		// valid waypoint
		if ( pWpt )
			pWpt->drawPaths(pClient->getPlayer(),pClient->getDrawType());
	}
}

void CWaypoints :: init ()
{
	m_iNumWaypoints = 0;
	m_fNextDrawWaypoints = 0;

	for ( int i = 0; i < MAX_WAYPOINTS; i ++ )
		m_theWaypoints[i].init();

	Q_memset(m_theWaypoints,0,sizeof(CWaypoint)*MAX_WAYPOINTS);	

	CWaypointLocations::Init();
	m_pVisibilityTable->ClearVisibilityTable();
}

void CWaypoints :: setupVisibility ()
{
	m_pVisibilityTable = new CWaypointVisibilityTable();
	m_pVisibilityTable->init();
}

void CWaypoints :: freeMemory ()
{
	if ( m_pVisibilityTable )
	{
		m_pVisibilityTable->FreeVisibilityTable();

		delete m_pVisibilityTable;
	}
	m_pVisibilityTable = NULL;
}

void CWaypoints :: precacheWaypointTexture ()
{
	m_iWaypointTexture = engine->PrecacheModel( "sprites/lgtning.vmt" );
}

///////////////////////////////////////////////////////
// return nearest waypoint not visible to pinch point
CWaypoint *CWaypoints :: getPinchPointFromWaypoint ( Vector vPlayerOrigin, Vector vPinchOrigin )
{
	int iWpt = CWaypointLocations::GetCoverWaypoint(vPlayerOrigin,vPinchOrigin,NULL,&vPinchOrigin);

	return getWaypoint(iWpt);
}

void CWaypoints :: deleteWaypoint ( int iIndex )
{	
	// mark as not used
	m_theWaypoints[iIndex].setUsed(false);	
	m_theWaypoints[iIndex].clearPaths();

	// remove from waypoint locations
	Vector vOrigin = m_theWaypoints[iIndex].getOrigin();
	float fOrigin[3] = { vOrigin.x, vOrigin.y, vOrigin.z };
	CWaypointLocations::DeleteWptLocation(iIndex,fOrigin);

	// delete any paths pointing to this waypoint
	deletePathsTo(iIndex);
}

int CWaypoints :: getClosestFlagged ( int iFlags, Vector &vOrigin, int iTeam, float *fReturnDist, unsigned char *failedwpts )
{
	int i = 0;
	int size = numWaypoints();

	float fDist = 8192.0;
	float distance;
	int iwpt = -1;
	int iFrom = CWaypointLocations::NearestWaypoint(vOrigin,fDist,-1,true,false,true,NULL,false,iTeam);

	CWaypoint *pWpt;

	for ( i = 0; i < size; i ++ )
	{
		pWpt = &m_theWaypoints[i];

		if ( i == iFrom )
			continue;

		if ( failedwpts[i] == 1 )
			continue;

		if ( pWpt->isUsed() && pWpt->forTeam(iTeam) )
		{
			if ( pWpt->hasFlag(iFlags) )
			{
				if ( (iFrom == -1) )
					distance = (pWpt->getOrigin()-vOrigin).Length();
				else
					distance = CWaypointDistances::getDistance(iFrom,i);

				if ( distance < fDist)
				{
					fDist = distance;
					iwpt = i;
				}
			}
		}
	}

	if ( fReturnDist )
		*fReturnDist = fDist;

	return iwpt;
}

void CWaypoints :: deletePathsTo ( int iWpt )
{
	for ( int i = 0; i < numWaypoints(); i ++ )
		m_theWaypoints[i].removePathTo(iWpt);
}

// Fixed; 23/01
void CWaypoints :: deletePathsFrom ( int iWpt )
{
	m_theWaypoints[iWpt].clearPaths();
}

void CWaypoints :: addWaypoint ( CClient *pClient )
{
	int iFlags = 0;
	Vector vWptOrigin = pClient->getOrigin();
	QAngle playerAngles = CBotGlobals::playerAngles (pClient->getPlayer());

	//IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pClient->getPlayer());

	/*CBasePlayer *pPlayer = (CBasePlayer*)(CBaseEntity::Instance(pClient->getPlayer()));

	 // on a ladder
	if ( pPlayer->GetMoveType() == MOVETYPE_LADDER ) 
		iFlags |= CWaypoint::W_FL_LADDER;

	if ( pPlayer->GetFlags() & FL_DUCKING )
		iFlags |= CWaypoint::W_FL_CROUCH;		*/

	addWaypoint(pClient->getPlayer(),vWptOrigin,iFlags,pClient->isAutoPathOn(),(int)playerAngles.y,pClient->getWptArea()); // sort flags out
}

void CWaypoints :: addWaypoint ( edict_t *pPlayer, Vector vOrigin, int iFlags, bool bAutoPath, int iYaw, int iArea )
{
	int iIndex = freeWaypointIndex();

	if ( iIndex == -1 )	
	{
		Msg("Waypoints full!");
		return;
	}

	///////////////////////////////////////////////////
	m_theWaypoints[iIndex] = CWaypoint(vOrigin,iFlags);	
	m_theWaypoints[iIndex].setAim(iYaw);
	m_theWaypoints[iIndex].setArea(iArea);
	// increase max waypoints used
	if ( iIndex == m_iNumWaypoints )
		m_iNumWaypoints++;	
	///////////////////////////////////////////////////

	float fOrigin[3] = {vOrigin.x,vOrigin.y,vOrigin.z};

	CWaypointLocations::AddWptLocation(iIndex,fOrigin);
	m_pVisibilityTable->workVisibilityForWaypoint(iIndex,true);

	if ( bAutoPath )
		CWaypointLocations::AutoPath(pPlayer,iIndex);
}

void CWaypoints :: removeWaypoint ( int iIndex )
{
	if ( iIndex >= 0 )
		m_theWaypoints[iIndex].setUsed(false);
}

int CWaypoints :: numWaypoints ()
{
	return m_iNumWaypoints;
}

///////////

int CWaypoints :: nearestWaypointGoal ( int iFlags, Vector &origin, float fDist, int iTeam )
{
	int i = 0;
	int size = numWaypoints();

	float distance;
	int iwpt = -1;

	CWaypoint *pWpt;

	for ( i = 0; i < size; i ++ )
	{
		pWpt = &m_theWaypoints[i];

		if ( pWpt->isUsed() && pWpt->forTeam(iTeam) )
		{
			if ( (iFlags == -1) || pWpt->hasFlag(iFlags) )
			{
				if ( (distance = pWpt->distanceFrom(origin)) < fDist)
				{
					fDist = distance;
					iwpt = i;
				}
			}
		}
	}

	return iwpt;
}

CWaypoint *CWaypoints :: randomWaypointGoal ( int iFlags, int iTeam, int iArea )
{
	int i = 0;
	int size = numWaypoints();
	CWaypoint *pWpt;

	dataUnconstArray<CWaypoint*> goals;

	for ( i = 0; i < size; i ++ )
	{
		pWpt = &m_theWaypoints[i];

		if ( pWpt->isUsed() && pWpt->forTeam(iTeam) && (pWpt->getArea() == iArea) )
		{			
			if ( (iFlags == -1) || pWpt->hasFlag(iFlags) )
				goals.Add(pWpt);
		}
	}

	pWpt = NULL;

	if ( !goals.IsEmpty() )
		pWpt = goals.Random();

	goals.Clear();

	return pWpt;
}

int CWaypoints :: randomFlaggedWaypoint (int iTeam)
{
	return getWaypointIndex(randomWaypointGoal(-1,iTeam));
}

///////////

// get the next free slot to save a waypoint to
int CWaypoints :: freeWaypointIndex ()
{
	for ( int i = 0; i < MAX_WAYPOINTS; i ++ )
	{
		if ( !m_theWaypoints[i].isUsed() )
			return i;
	}

	return -1;
}

int CWaypoint :: numPaths ()
{
	return m_thePaths.Size();
}

int CWaypoint :: getPath ( int i )
{
	return m_thePaths.ReturnValueFromIndex(i);
}

bool CWaypoint :: addPathTo ( int iWaypointIndex )
{
	CWaypoint *pTo = CWaypoints::getWaypoint(iWaypointIndex);

	if ( pTo == NULL )
		return false;
	// already in list
	if ( m_thePaths.IsMember(iWaypointIndex) )
		return false;
	// dont have a path loop
	if ( this == pTo )
		return false;

	m_thePaths.Add(iWaypointIndex);

	return true;
}

void CWaypoint :: removePathTo ( int iWaypointIndex )
{
	m_thePaths.Remove(iWaypointIndex);

	return;
}

void CWaypoint :: info ( edict_t *pEdict )
{
	CWaypointTypes::printInfo(this,pEdict);
}

/////////////////////////////////////
// Waypoint Types
/////////////////////////////////////

CWaypointType *CWaypointTypes :: getType( const char *szType )
{
	for ( unsigned int i = 0; i < m_Types.size(); i ++ )
	{
		if ( FStrEq(m_Types[i]->getName(),szType) )
			return m_Types[i];
	}

	return NULL;
}

void CWaypointTypes :: showTypesOnConsole ( edict_t *pPrintTo )
{
	CBotGlobals::botMessage(pPrintTo,0,"Available waypoint types");

	for ( unsigned int i = 0; i < m_Types.size(); i ++ )
	{
		const char *name = m_Types[i]->getName();
		const char *description = m_Types[i]->getDescription();

		CBotGlobals::botMessage(pPrintTo,0,"\"%s\" (%s)",name,description);
	}
}

void CWaypointTypes:: addType ( CWaypointType *type )
{
	m_Types.push_back(type);
}

CWaypointType *CWaypointTypes :: getTypeByIndex ( unsigned int iIndex )
{
	if ( iIndex < m_Types.size() )
	{
		return m_Types[iIndex];
	}
	else
		return NULL;
}

unsigned int CWaypointTypes :: getNumTypes ()
{
	return m_Types.size();
}

void CWaypointTypes :: setup ()
{	
	addType(new CWaypointType(W_FL_JUMP,"jump","bot will jump here",WptColor(255,255,255)));
	addType(new CWaypointType(W_FL_CROUCH,"crouch","bot will duck here",WptColor(200,100,0)));
	addType(new CWaypointType(W_FL_UNREACHABLE,"unreachable","bot can't go here (used for visibility only)",WptColor(200,200,200)));
	addType(new CWaypointType(W_FL_LADDER,"ladder","bot will climb a ladder",WptColor(255,255,0)));
	addType(new CWaypointType(W_FL_FLAG,"flag","bot will find a flag here",WptColor(255,255,0)));
	addType(new CWaypointType(W_FL_CAPPOINT,"capture","bot will find a capture point here",WptColor(255,255,0)));
	addType(new CWaypointType(W_FL_NOBLU,"noblueteam","blue team can't use this waypoint",WptColor(255,0,0)));
	addType(new CWaypointType(W_FL_NORED,"noredteam","red team can't use this waypoint",WptColor(0,0,128)));
	addType(new CWaypointType(W_FL_HEALTH,"health","bot can sometimes get health here",WptColor(255,255,255)));
	addType(new CWaypointType(W_FL_OPENS_LATER,"openslater","this waypoint is available when a door is open only",WptColor(100,100,200)));
	addType(new CWaypointType(W_FL_SNIPER,"sniper","a bot can snipe here",WptColor(0,255,0)));
	addType(new CWaypointType(W_FL_ROCKET_JUMP,"rocketjump","a bot can rocket jump here",WptColor(10,100,0)));
	addType(new CWaypointType(W_FL_AMMO,"ammo","bot can sometimes get ammo here",WptColor(50,100,10)));
	addType(new CWaypointType(W_FL_RESUPPLY,"resupply","bot can always get ammo and health here",WptColor(255,100,255)));
	addType(new CWaypointType(W_FL_SENTRY,"sentry","engineer bot can build here",WptColor(255,0,0)));
	addType(new CWaypointType(W_FL_DOUBLEJUMP,"doublejump","scout can double jump here",WptColor(10,10,100)));

	addType(new CWaypointType(W_FL_TELE_ENTRANCE,"teleentrance","engineer bot can build tele entrance here",WptColor(50,50,150)));
	addType(new CWaypointType(W_FL_TELE_EXIT,"teleexit","engineer bot can build tele exit here",WptColor(100,100,255)));
	addType(new CWaypointType(W_FL_DEFEND,"defend","bot will defend at this position",WptColor(160,50,50)));
	
}

void CWaypointTypes :: freeMemory ()
{
	for ( unsigned int i = 0; i < m_Types.size(); i ++ )
	{
		delete m_Types[i];
		m_Types[i] = NULL;
	}

	m_Types.clear();
}

void CWaypointTypes:: printInfo ( CWaypoint *pWpt, edict_t *pPrintTo )
{
	char szMessage[1024];
	Q_snprintf(szMessage,1024,"Waypoint ID %d (Area = %d | Radius = %0.1f)[",CWaypoints::getWaypointIndex(pWpt),pWpt->getArea(),pWpt->getRadius());	

	if ( pWpt->getFlags() )
	{
		bool bComma = false;

		for ( unsigned int i = 0; i < m_Types.size(); i ++ )
		{
			if ( m_Types[i]->isBitsInFlags(pWpt->getFlags()) )
			{
				if ( bComma )
					strcat(szMessage,",");

				strcat(szMessage,m_Types[i]->getName());
				//strcat(szMessage," (");
				//strcat(szMessage,m_Types[i]->getDescription());
				//strcat(szMessage,")");				
				bComma = true;
			}
		}
	}
	else
	{
		strcat(szMessage,"No Waypoint Types");
	}

	strcat(szMessage,"]");

	debugoverlay->AddTextOverlay(pWpt->getOrigin()+Vector(0,0,24),0,6,szMessage);

	CRCBotPlugin :: HudTextMessage (pPrintTo,"wptinfo","Waypoint Info",szMessage,Color(255,0,0,255),1,2);
}
/*
CCrouchWaypointType :: CCrouchWaypointType()
{
    CWaypointType(W_FL_CROUCH,"crouch","bot will duck here",WptColor(200,100,0));
}

void CCrouchWaypointType :: giveTypeToWaypoint ( CWaypoint *pWaypoint )
{

}

void CCrouchWaypointType :: removeTypeFromWaypoint ( CWaypoint *pWaypoint )
{

}
*/
void CWaypointTypes :: displayTypesMenu ( edict_t *pPrintTo )
{

}

void CWaypointTypes:: selectedType ( CClient *pClient )
{

}

/*void CWaypointType :: giveTypeToWaypoint ( CWaypoint *pWaypoint )
{

}

void CWaypointType :: removeTypeFromWaypoint ( CWaypoint *pWaypoint )
{

}*/

CWaypointType :: CWaypointType (int iBit, const char *szName, const char *szDescription, WptColor vColour )
{
	m_iBit = iBit;
	m_szName = CStrings::getString(szName);
	m_szDescription = CStrings::getString(szDescription);
	m_vColour = vColour;
}

bool CWaypoint :: forTeam ( int iTeam )
{
	if ( iTeam == TF2_TEAM_BLUE )
		return (m_iFlags & CWaypointTypes::W_FL_NOBLU)==0;
	else if ( iTeam == TF2_TEAM_RED )
		return (m_iFlags & CWaypointTypes::W_FL_NORED)==0;

	return true;	
}