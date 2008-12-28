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

#include "bot.h"

#include "bot_waypoint.h"
#include "bot_waypoint_visibility.h"
#include "bot_waypoint_locations.h"
#include "bot_genclass.h"
#include "bot_globals.h"

int CWaypointLocations :: g_iFailedWaypoints[CWaypoints::MAX_WAYPOINTS];
dataUnconstArray<int> CWaypointLocations :: m_iLocations[MAX_WPT_BUCKETS][MAX_WPT_BUCKETS][MAX_WPT_BUCKETS];

#define READ_LOC(loc) abs((int)((int)(loc + HALF_MAX_MAP_SIZE) / BUCKET_SPACING));

void CWaypointLocations :: getMinMaxs ( int iLoc, int jLoc, int kLoc, 
									    int *iMinLoci, int *iMinLocj, int *iMinLock,
									    int *iMaxLoci, int *iMaxLocj, int *iMaxLock )
{
	// get current area
	*iMinLoci = iLoc-1;
	*iMaxLoci = iLoc+1;

	*iMinLocj = jLoc-1;
	*iMaxLocj = jLoc+1;

	*iMinLock = kLoc-1;
	*iMaxLock = kLoc+1;

	int iMaxLoc = MAX_WPT_BUCKETS-1;

	if ( *iMinLoci < 0 )
		*iMinLoci = 0;
	if ( *iMinLocj < 0 )
		*iMinLocj = 0;
	if ( *iMinLock < 0 )
		*iMinLock = 0;

	if ( *iMaxLoci > iMaxLoc )
		*iMaxLoci = iMaxLoc;
	if ( *iMaxLocj > iMaxLoc )
		*iMaxLocj = iMaxLoc;
	if ( *iMaxLock > iMaxLoc )
		*iMaxLock = iMaxLoc;

}
///////////////
// return nearest waypoint that can be used to cover from vCoverFrom vector
void CWaypointLocations :: AutoPath ( edict_t *pPlayer, int iWpt )
{
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);
	Vector vOrigin = pWpt->getOrigin();

	int iLoc = READ_LOC(vOrigin.x);
	int jLoc = READ_LOC(vOrigin.y);
	int kLoc = READ_LOC(vOrigin.z);

	int i,j,k;

	int iMinLoci,iMaxLoci,iMinLocj,iMaxLocj,iMinLock,iMaxLock;

	getMinMaxs(iLoc,jLoc,kLoc,&iMinLoci,&iMinLocj,&iMinLock,&iMaxLoci,&iMaxLocj,&iMaxLock);

	for ( i = iMinLoci; i <= iMaxLoci; i++ )
	{
		for ( j = iMinLocj; j <= iMaxLocj; j++ )
		{			
			for ( k = iMinLock; k <= iMaxLock; k++ )
			{		
				// check each area around the current area
				// for closer waypoints
				AutoPathInBucket(pPlayer, i,j,k,iWpt);
			}
		}
	}
}

void CWaypointLocations :: GetAllVisible ( Vector vVisibleFrom, Vector vOrigin, dataUnconstArray<int> *iVisible )
{
	int iLoc = READ_LOC(vOrigin.x);
	int jLoc = READ_LOC(vOrigin.y);
	int kLoc = READ_LOC(vOrigin.z);

	int i,j,k;

	int iMinLoci,iMaxLoci,iMinLocj,iMaxLocj,iMinLock,iMaxLock;

	getMinMaxs(iLoc,jLoc,kLoc,&iMinLoci,&iMinLocj,&iMinLock,&iMaxLoci,&iMaxLocj,&iMaxLock);

	for ( i = iMinLoci; i <= iMaxLoci; i++ )
	{
		for ( j = iMinLocj; j <= iMaxLocj; j++ )
		{			
			for ( k = iMinLock; k <= iMaxLock; k++ )
			{		
				//dataStack <int> tempStack = m_iLocations[i][j][k];

				//while ( !tempStack.IsEmpty() )
				for (  int l = 0; l < m_iLocations[i][j][k].Size(); l ++ )
				{
					int iWpt = m_iLocations[i][j][k][l];
					//int iWpt = tempStack.ChooseFromStack();
					
					if ( CBotGlobals::isVisible(vVisibleFrom,CWaypoints::getWaypoint(iWpt)->getOrigin()) )
						iVisible->Add(iWpt);
				}
			}
		}
	}
}

void CWaypointLocations :: AutoPathInBucket ( edict_t *pPlayer, int i, int j, int k, int iWptFrom )
{
	//dataStack <int> tempStack = m_iLocations[i][j][k];
	int iWpt;
	CWaypoint *pOtherWpt;

	CWaypoint *pWpt = CWaypoints::getWaypoint(iWptFrom);
	Vector vWptOrigin = pWpt->getOrigin();
	Vector vOtherWptOrigin;

	trace_t tr;

	//CTraceFilterWorldOnly filter;

	//while ( !tempStack.IsEmpty() )
	for ( int l = 0; l < m_iLocations[i][j][k].Size(); l ++ )
	{
		iWpt = m_iLocations[i][j][k][l];

		//iWpt = tempStack.ChooseFromStack();

		pOtherWpt = CWaypoints::getWaypoint(iWpt);

		if ( !pOtherWpt->isUsed() )
			continue;

		if ( pOtherWpt == pWpt )
			continue;
		
		vOtherWptOrigin = pOtherWpt->getOrigin();

	//	if ( fabs(vOtherWptOrigin.z-vWptOrigin.z) > 128 )
		//	continue;

		if ( CBotGlobals::isVisible(vWptOrigin,vOtherWptOrigin) )
		{
			if ( CBotGlobals::walkableFromTo(pPlayer, vWptOrigin,vOtherWptOrigin) )
			{
				pWpt->addPathTo(iWpt);			
			}

			if ( CBotGlobals::walkableFromTo(pPlayer,vOtherWptOrigin,vWptOrigin) )
			{
				pOtherWpt->addPathTo(iWptFrom);		
			}
		}
	}
}

void CWaypointLocations :: AddWptLocation ( int iIndex, const float *fOrigin )
{
// Add a waypoint with index and at origin (for quick insertion in the list)
//
	int i = READ_LOC(fOrigin[0]);
	int j = READ_LOC(fOrigin[1]);
	int k = READ_LOC(fOrigin[2]);

	m_iLocations[i][j][k].Add(iIndex);
	//m_iLocations[i][j][k].Push(iIndex);
}

void CWaypointLocations :: DeleteWptLocation ( int iIndex, const float *fOrigin )
// Delete the waypoint index at the origin (for finding it quickly in the list)
//
{
	int i = READ_LOC(fOrigin[0]);
	int j = READ_LOC(fOrigin[1]);
	int k = READ_LOC(fOrigin[2]);

	m_iLocations[i][j][k].Remove(iIndex);
	//m_iLocations[i][j][k].Remove(iIndex);
}


///////////////
// return nearest waypoint that can be used to cover from vCoverFrom vector
int CWaypointLocations :: GetCoverWaypoint ( Vector vPlayerOrigin, Vector vCoverFrom, dataUnconstArray<int> *iIgnoreWpts, Vector *vGoalOrigin, int iTeam )
{
	int iWaypoint;

	iWaypoint = CWaypointLocations::NearestWaypoint(vCoverFrom,REACHABLE_RANGE,-1,false,true);

	if ( iWaypoint == -1 )
		return -1;

	float fNearestDist = HALF_MAX_MAP_SIZE;
	
	int iNearestIndex = -1;

	int iLoc = READ_LOC(vPlayerOrigin.x);
	int jLoc = READ_LOC(vPlayerOrigin.y);
	int kLoc = READ_LOC(vPlayerOrigin.z);

	int i,j,k;

	int iMinLoci,iMaxLoci,iMinLocj,iMaxLocj,iMinLock,iMaxLock;

	getMinMaxs(iLoc,jLoc,kLoc,&iMinLoci,&iMinLocj,&iMinLock,&iMaxLoci,&iMaxLocj,&iMaxLock);

	memset(g_iFailedWaypoints,0,sizeof(unsigned char)*CWaypoints::MAX_WAYPOINTS);
	
	if ( iIgnoreWpts )
	{   
		//dataStack<int> ignoreWptStack = *iIgnoreWpts;
		int iWpt;
		
		//while ( !ignoreWptStack.IsEmpty() )
		for ( int l = 0; l < iIgnoreWpts->Size(); l ++ )
		{
			if ( (iWpt = (*iIgnoreWpts)[l]) != -1 )//(iWpt = ignoreWptStack.ChooseFromStack()) != -1 )
				g_iFailedWaypoints[iWpt] = 1;
		}
	}

	for ( i = iMinLoci; i <= iMaxLoci; i++ )
	{
		for ( j = iMinLocj; j <= iMaxLocj; j++ )
		{
			for ( k = iMinLock; k <= iMaxLock; k++ )
			{
				// check each area around the current area
				// for closer waypoints
				FindNearestCoverWaypointInBucket(i,j,k,vPlayerOrigin,&fNearestDist,&iNearestIndex,iIgnoreWpts,iWaypoint,vGoalOrigin,iTeam);
			}
		}
	}

	return iNearestIndex;
}

void CWaypointLocations :: FindNearestCoverWaypointInBucket ( int i, int j, int k, const Vector &vOrigin, float *pfMinDist, int *piIndex, dataUnconstArray<int> *iIgnoreWpts, int iCoverFromWpt, Vector *vGoalOrigin, int iTeam )
// Search for the nearest waypoint : I.e.
// Find the waypoint that is closest to vOrigin from the distance pfMinDist
// And set the piIndex to the waypoint index if closer.
{
	//dataStack <int> tempStack = m_iLocations[i][j][k];
	
	CWaypoint *curr_wpt;
	int iSelectedIndex;
	float fDist;
	
	for ( int l = 0; l < m_iLocations[i][j][k].Size(); l ++ )
	//while ( !tempStack.IsEmpty() )
	{
		iSelectedIndex = m_iLocations[i][j][k][l];//tempStack.ChooseFromStack();

		if ( iCoverFromWpt == iSelectedIndex )
			continue;
		if ( g_iFailedWaypoints[iSelectedIndex] )
		    continue;

		curr_wpt = CWaypoints::getWaypoint(iSelectedIndex);

		if ( !curr_wpt->isUsed() )
			continue; 
		if ( curr_wpt->hasFlag(CWaypointTypes::W_FL_UNREACHABLE) )
			continue;
		if ( !curr_wpt->forTeam(iTeam) )
			continue;
		if ( CWaypoints::getVisiblity()->GetVisibilityFromTo(iCoverFromWpt,iSelectedIndex) )
			continue;


		(fDist = curr_wpt->distanceFrom(vOrigin));

		if ( vGoalOrigin != NULL )
			fDist += curr_wpt->distanceFrom(*vGoalOrigin);

		if ( fDist < *pfMinDist )
		{
			*piIndex = iSelectedIndex;
			*pfMinDist = fDist;			
		}
	}
}
///////////////////////////////////////////////
//

void CWaypointLocations :: FindNearestInBucket ( int i, int j, int k, const Vector &vOrigin, float *pfMinDist, int *piIndex, int iIgnoreWpt, bool bGetVisible, bool bGetUnReachable, bool bIsBot, dataUnconstArray<int> *iFailedWpts, bool bNearestAimingOnly, int iTeam )
// Search for the nearest waypoint : I.e.
// Find the waypoint that is closest to vOrigin from the distance pfMinDist
// And set the piIndex to the waypoint index if closer.
{
	//dataStack <int> tempStack = m_iLocations[i][j][k];

	CWaypoint *curr_wpt;
	int iSelectedIndex;
	float fDist;
//	int iWptFlags;

	trace_t tr;
	
	bool bAdd;
	
	//while ( !tempStack.IsEmpty() )
	for ( int l = 0; l < m_iLocations[i][j][k].Size(); l ++ )
	{
		//iSelectedIndex = tempStack.ChooseFromStack();
		iSelectedIndex = m_iLocations[i][j][k][l];

		if ( iSelectedIndex == iIgnoreWpt )
			continue;
		if ( g_iFailedWaypoints[iSelectedIndex] == 1 )
			continue;

		curr_wpt = CWaypoints::getWaypoint(iSelectedIndex);

		if ( !bGetUnReachable && curr_wpt->hasFlag(CWaypointTypes::W_FL_UNREACHABLE) )
			continue;

		if ( !curr_wpt->isUsed() )
			continue;

		if ( !curr_wpt->forTeam(iTeam) )
			continue;

		if ( (fDist = curr_wpt->distanceFrom(vOrigin)) < *pfMinDist )
		{
			bAdd = false;
			
			if ( bGetVisible == false )
				bAdd = true;
			else
			{
				bAdd = CBotGlobals::isVisible(vOrigin,curr_wpt->getOrigin());
			}
			
			if ( bAdd )
			{
				*piIndex = iSelectedIndex;
				*pfMinDist = fDist;
			}
		}
		
	}
}

/////////////////////////////
// get the nearest waypoint INDEX from an origin
int CWaypointLocations :: NearestWaypoint ( const Vector &vOrigin, float fNearestDist, int iIgnoreWpt, bool bGetVisible, bool bGetUnReachable, bool bIsBot, dataUnconstArray<int> *iFailedWpts, bool bNearestAimingOnly, int iTeam )
{
	int iNearestIndex = -1;

	int iLoc = READ_LOC(vOrigin.x);
	int jLoc = READ_LOC(vOrigin.y);
	int kLoc = READ_LOC(vOrigin.z);

	int i,j,k;

	int iMinLoci,iMaxLoci,iMinLocj,iMaxLocj,iMinLock,iMaxLock;

	getMinMaxs(iLoc,jLoc,kLoc,&iMinLoci,&iMinLocj,&iMinLock,&iMaxLoci,&iMaxLocj,&iMaxLock);

	if ( !bNearestAimingOnly )
	{
		memset(g_iFailedWaypoints,0,sizeof(unsigned char)*CWaypoints::MAX_WAYPOINTS);
		
		if ( iFailedWpts )
		{   
			int iWpt;
			
			for ( int l = 0; l < iFailedWpts->Size(); l ++ )
			{
				if ( (iWpt=(*iFailedWpts)[l]) != -1 ) //( (iWpt = tempStack.ChooseFromStack()) != -1 )
					g_iFailedWaypoints[iWpt] = 1;
			}
		}
	}

	for ( i = iMinLoci; i <= iMaxLoci; i++ )
	{
		for ( j = iMinLocj; j <= iMaxLocj; j++ )
		{
			for ( k = iMinLock; k <= iMaxLock; k++ )
			{
				FindNearestInBucket(i,j,k,vOrigin,&fNearestDist,&iNearestIndex,iIgnoreWpt,bGetVisible,bGetUnReachable,bIsBot,iFailedWpts,bNearestAimingOnly,iTeam);
			}
		}
	}

	return iNearestIndex;
}

//////////////////////////////////
// Draw waypoints around a player
void CWaypointLocations :: DrawWaypoints ( edict_t *pEntity, Vector &vOrigin, float fDist, bool bDrawPaths, unsigned short int iDrawType )
{
	static byte m_bPvs[MAX_MAP_CLUSTERS/8];
	int clusterIndex;

	int iLoc = READ_LOC(vOrigin.x);
	int jLoc = READ_LOC(vOrigin.y);
	int kLoc = READ_LOC(vOrigin.z);

	int i,j,k;

	int iMinLoci,iMaxLoci,iMinLocj,iMaxLocj,iMinLock,iMaxLock;

	getMinMaxs(iLoc,jLoc,kLoc,&iMinLoci,&iMinLocj,&iMinLock,&iMaxLoci,&iMaxLocj,&iMaxLock);

	Vector vWpt;

	for ( i = iMinLoci; i <= iMaxLoci; i++ )
	{
		for ( j = iMinLocj; j <= iMaxLocj; j++ )
		{
			for ( k = iMinLock; k <= iMaxLock; k++ )
			{
				for ( int l = 0; l < m_iLocations[i][j][k].Size(); l ++ )
				{
					// Draw each waypoint in distance
					int iWpt = m_iLocations[i][j][k][l];

					CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);//tempStack.ChooseFromStack());

					if ( !pWpt->isUsed() ) // deleted
						continue;

					vWpt = pWpt->getOrigin();

					if ( fabs(vWpt.z - vOrigin.z) <= 256.0 ) // also in z range
					{
						//if ( CBotGlobals::FInViewCone(pEntity) )
						{
							// from Valve developer community wiki
							// http://developer.valvesoftware.com/wiki/Transforming_the_Multiplayer_SDK_into_Coop

							clusterIndex = engine->GetClusterForOrigin( vOrigin );
							engine->GetPVSForCluster( clusterIndex, sizeof(m_bPvs), m_bPvs );							

							if ( engine->CheckOriginInPVS( vWpt, m_bPvs, sizeof( m_bPvs ) ) )
								pWpt->draw(pEntity,bDrawPaths,iDrawType);
						}
					}
				}
			}
		}
	}
}

void CWaypointLocations ::AddWptLocation (CWaypoint *pWaypoint, int iIndex)
{
	Vector vOrigin = pWaypoint->getOrigin();
	float flOrigin[3] = { vOrigin.x, vOrigin.y, vOrigin.z };

	AddWptLocation(iIndex,flOrigin);
}