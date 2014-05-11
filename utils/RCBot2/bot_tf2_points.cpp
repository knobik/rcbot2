// TO DO : All defend points valid
// TO DO : Fix probability of attack/defending particular points
#include "vstdlib/random.h" // for random  seed 

#include "bot.h"
#include "bot_getprop.h"
#include "bot_fortress.h"
#include "bot_tf2_points.h"
#include "bot_waypoint.h"
#include "bot_globals.h"
#include "bot_waypoint_locations.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// INPUT = Waypoint Area
bool CTFObjectiveResource :: isWaypointAreaValid ( int wptarea ) 
{
//	CWaypoint *pWaypoint;

	if ( wptarea == 0 )
		return true;

	// Translate Waypoint Area to Index
	if ( (wptarea < 0) || (wptarea > MAX_CONTROL_POINTS) )
		return false;

	int cpindex = m_WaypointAreaToIndexTranslation[wptarea];

	if ( cpindex == -1 )
		return false;

	return m_ValidAreas[cpindex];
	/*
	for ( int i = 0; i < MAX_CONTROL_POINTS; i ++ )
	{
		pWaypoint = CWaypoints::getWaypoint(m_iControlPointWpt[i]);

		if ( pWaypoint && (pWaypoint->getArea() == wptarea) )
		{
			return m_ValidAreas[i];
		}
	}
	
	// can't find return default
	return m_ValidAreas[wptarea-1];*/
}

// TO DO  - Base on waypoint danger
// base on base point -- if already have attack point and base point -- less focus on base point
int CTFObjectiveResource::getRandomValidPointForTeam ( int team, ePointAttackDefend_s type)
{
	TF2PointProb_t *arr = NULL;
	vector<int> points;

	float fTotal = 0.0f;

	if (( team < 2 ) || ( team > 3 ))
		return 0;

	if ( m_iNumControlPoints == NULL )
		return 0;

	arr = m_ValidPoints[team-2][type];

	for ( int i = 0; i < *m_iNumControlPoints; i ++ )
	{
		if ( arr[i].bValid == true )
		{
			points.push_back(i);
			fTotal += arr[i].fProb;
		}
	}

	float fRand = randomFloat(0.0f,fTotal);

	fTotal = 0.0f;

	for ( unsigned int i = 0; i < points.size(); i ++ )
	{
		int index = points[i];

		fTotal += arr[index].fProb;

		if ( fTotal > fRand )
		{
			return m_IndexToWaypointAreaTranslation[index];
		}
	}

	// no points
	return 0;
}

void CTeamRoundTimer :: reset ()
{
	if ( m_Resource.get() == NULL )
	{
		m_Resource = CClassInterface::FindEntityByNetClass(gpGlobals->maxClients,"CTeamRoundTimer");

		if ( m_Resource.get() != NULL )
		{
			CClassInterface::setupCTeamRoundTimer(this);
		}
	}
}

bool CTeamControlPointRound :: isPointInRound ( int iIndex )
{
	edict_t *pPoint;
	extern ConVar rcbot_const_point_offset;

	for ( int i = 0; i < m_ControlPoints.Size(); i ++ )
	{
		CBaseHandle *hndl;

		hndl = (CBaseHandle *)&(m_ControlPoints[i]); 

		if ( hndl )
		{ 
			pPoint = INDEXENT(hndl->GetEntryIndex());

			CBaseEntity *pent = pPoint->GetUnknown()->GetBaseEntity();

			CTeamControlPoint *point = (CTeamControlPoint*)((unsigned long)pent + rcbot_const_point_offset.GetInt() );

			if ( point )
			{
				if ( point->m_iIndex == iIndex )
					return true;
			}
		}
	}

	return false;
}

CTeamControlPointRound *CTeamControlPointMaster:: getCurrentRound ( )
{
	if ( m_iCurrentRoundIndex == -1 )
		return NULL;

	CBaseEntity *pent = m_ControlPointRounds[m_iCurrentRoundIndex];

	extern ConVar rcbot_const_round_offset;

	return (CTeamControlPointRound*)((unsigned long)pent+(unsigned long)rcbot_const_round_offset.GetInt());
}

//////////////////


void CTFObjectiveResource::setup ()
{
	memset(m_pControlPoints,0,sizeof(edict_t*)*8);
	memset(m_iControlPointWpt,0xFF,sizeof(int)*8);
	// Find control point entities

	edict_t *pent;

	Vector vOrigin;

	int i = gpGlobals->maxClients;

	memset(m_IndexToWaypointAreaTranslation,0,sizeof(int)*MAX_CONTROL_POINTS);
	memset(m_WaypointAreaToIndexTranslation,0xFF,sizeof(int)*(MAX_CONTROL_POINTS+1));

	// find visible flags -- with a model
	while ( ++i < gpGlobals->maxEntities )
	{
		pent = INDEXENT(i);

		if ( !pent || pent->IsFree() )
			continue;
			
		if ( strcmp(pent->GetClassName(),"team_control_point") == 0 )
		{
			vOrigin = CBotGlobals::entityOrigin(pent);

			for ( int j = 0; j < *m_iNumControlPoints; j ++ )
			{
				if ( m_pControlPoints[j] != NULL )
					continue;

				if ( vOrigin == m_vCPPositions[j] )
				{
					m_pControlPoints[j] = pent;
				}
			}
		}
	}

	CWaypoint *pWaypoint;
	int iWpt;

	for ( int j = 0; j < *m_iNumControlPoints; j ++ )
	{
		vOrigin = m_vCPPositions[j];

		if ( m_iControlPointWpt[j] == -1 )
		{
			iWpt = CWaypointLocations::NearestWaypoint(vOrigin,1024.0f,-1,false,false,false,NULL,false,0,false,false,Vector(0,0,0),CWaypointTypes::W_FL_CAPPOINT);
			pWaypoint = CWaypoints::getWaypoint(iWpt);
			m_iControlPointWpt[j] = iWpt;

			// For compatibility -- old waypoints are already set with an area, so take the area from the waypoint here
			// in the future waypoints will automatically be set to the waypoint area anyway
			if ( pWaypoint )
			{
				int iArea = pWaypoint->getArea();
				m_IndexToWaypointAreaTranslation[j] = iArea;

				if ( ( iArea >= 1 ) && ( iArea < MAX_CONTROL_POINTS ) )
					m_WaypointAreaToIndexTranslation[iArea] = j;
			}
			else
			{
				m_IndexToWaypointAreaTranslation[j] = j;
				m_WaypointAreaToIndexTranslation[j] = j;
			}
		}
	}
	

}

int CTFObjectiveResource :: getControlPointArea ( edict_t *pPoint )
{
	for ( int j = 0; j < *m_iNumControlPoints; j ++ )
	{
		if ( m_pControlPoints[j] == pPoint )
			return (j+1); // return waypoint area (+1)
	}

	return 0;
}
void CTFObjectiveResource::	debugprint ( void )
{
	edict_t *pEdict = CClients::getListenServerClient();

	CBotGlobals::botMessage(pEdict,0,"m_iNumControlPoints = %d",*m_iNumControlPoints);
	CBotGlobals::botMessage(pEdict,0,"m_bBlocked[8]\t[%s,%s,%s,%s,%s,%s,%s,%s]",m_bBlocked[0]?"Y":"N",m_bBlocked[1]?"Y":"N",m_bBlocked[2]?"Y":"N",m_bBlocked[3]?"Y":"N",m_bBlocked[4]?"Y":"N",m_bBlocked[5]?"Y":"N",m_bBlocked[6]?"Y":"N",m_bBlocked[7]?"Y":"N");
	CBotGlobals::botMessage(pEdict,0,"m_bCPLocked[byte]\t[%d]",*m_bCPLocked);
	CBotGlobals::botMessage(pEdict,0,"m_bCPLocked[8]\t[%s,%s,%s,%s,%s,%s,%s,%s]",m_bCPLocked[0]?"Y":"N",m_bCPLocked[1]?"Y":"N",m_bCPLocked[2]?"Y":"N",m_bCPLocked[3]?"Y":"N",m_bCPLocked[4]?"Y":"N",m_bCPLocked[5]?"Y":"N",m_bCPLocked[6]?"Y":"N",m_bCPLocked[7]?"Y":"N");
	CBotGlobals::botMessage(pEdict,0,"m_bCPIsVisible[8]\t[%s,%s,%s,%s,%s,%s,%s,%s]",m_bCPIsVisible[0]?"Y":"N",m_bCPIsVisible[1]?"Y":"N",m_bCPIsVisible[2]?"Y":"N",m_bCPIsVisible[3]?"Y":"N",m_bCPIsVisible[4]?"Y":"N",m_bCPIsVisible[5]?"Y":"N",m_bCPIsVisible[6]?"Y":"N",m_bCPIsVisible[7]?"Y":"N");
	CBotGlobals::botMessage(pEdict,0,"m_iOwner[8]\t[%s,%s,%s,%s,%s,%s,%s,%s]",(m_iOwner[0]==2)?"red":((m_iOwner[0]==3)?"blue":"unassigned"),(m_iOwner[1]==2)?"red":((m_iOwner[1]==3)?"blue":"unassigned"),(m_iOwner[2]==2)?"red":((m_iOwner[2]==3)?"blue":"unassigned"),(m_iOwner[3]==2)?"red":((m_iOwner[3]==3)?"blue":"unassigned"),(m_iOwner[4]==2)?"red":((m_iOwner[4]==3)?"blue":"unassigned"),(m_iOwner[5]==2)?"red":((m_iOwner[5]==3)?"blue":"unassigned"),(m_iOwner[6]==2)?"red":((m_iOwner[6]==3)?"blue":"unassigned"),(m_iOwner[7]==2)?"red":((m_iOwner[7]==3)?"blue":"unassigned"));
}

int CTFObjectiveResource::NearestArea ( Vector vOrigin )
{
	int iNearest = -1;
	float fNearest = 1024.0f;
	float fDist;

	for ( int i = 0; i < *m_iNumControlPoints; i ++ )
	{
		if ( (fDist = (m_vCPPositions[i]-vOrigin).Length()) < fNearest )
		{
			fNearest = fDist;
			iNearest = i;
		}
	}

	// Add one for waypoint area
	return iNearest+1;
}

void CTFObjectiveResource :: updateDefendPoints ( int team )
{
	/*int other = (team==2)?3:2;

	return GetCurrentAttackPoint(other);
	*/
	
	int other;
	int prev;
	TF2PointProb_t *arr;

	CTeamControlPointRound *pRound = CTeamFortress2Mod::getCurrentRound();

	if ( m_ObjectiveResource.get() == NULL ) // not set up yet
		return;
	if ( team == 0 ) // invalid team
		return;

	arr = m_ValidPoints[team-2][TF2_POINT_DEFEND];

	// reset array
	memset(arr,0,sizeof(bool)*MAX_CONTROL_POINTS);

	for ( int i = 0; i < *m_iNumControlPoints; i ++ )
	{
		arr[i].fProb = 1.0f;
		// not visible
		if ( m_bCPIsVisible[i] == 0 )
			continue;
		// not unlocked
		if ( m_flUnlockTimes[i] > gpGlobals->curtime )
			continue;
		// not in round
		if ( m_pControlPoints[i] && pRound && !pRound->isPointInRound(i) )
			continue;

		// We own this point
		if ( GetOwningTeam(i) == team )
		{
			// The other team can capture
			other = (team==2)?3:2;

			if ( TeamCanCapPoint(i,other) )
			{
				// if the other team has capture the previous points
				if ( (prev = GetPreviousPointForPoint(i,other,0)) != -1 )
				{
					if ( prev == i )
					{
						arr[i].bValid = true;
						continue;
					}
					else
					{
						// This point needs previous points to be captured first
						int j;

						for ( j = 0; j < 3; j ++ )
						{
							prev = GetPreviousPointForPoint(i,other,j);

							if ( prev == -1 )
								continue;
							else if ( GetOwningTeam(prev) != other )
								break;
						}

						arr[i].bValid = true;

						if ( j != 3 )
							arr[i].fProb = 0.1f;
						/*
						// other team has captured previous points
						if ( j == 3 )
						{
							arr[i].bValid = true;
							continue;
						}
						else
						{
							continue;*/
					}						
				}
				else
				{
					int basepoint = GetBaseControlPointForTeam(team);

					if ( i == basepoint )
					{
						arr[i].fProb = 0.2f;
					}

					arr[i].bValid = true;
					continue;
				}
			}
			else
			{
				//arr[i].bValid = true;
				//arr[i].fProb = 0.1f; // 10% of the time defend here
			}
		}
	}
}
void CTFObjectiveResource :: updateCaptureTime(int index)
{
	m_fLastCaptureTime[index] = engine->Time();
}

float CTFObjectiveResource :: getLastCaptureTime(int index)
{
	return m_fLastCaptureTime[index];
}

void CTFObjectiveResource :: updateAttackPoints ( int team )
{	
	int prev;
	CTeamControlPointRound *pRound = CTeamFortress2Mod::getCurrentRound();
	TF2PointProb_t *arr;

	if ( m_ObjectiveResource.get() == NULL ) // not set up yet
		return;

	arr = m_ValidPoints[team-2][TF2_POINT_ATTACK];

	// reset array
	memset(arr,0,sizeof(bool)*MAX_CONTROL_POINTS);

	if ( (team == TF2_TEAM_RED) && (CTeamFortress2Mod::isAttackDefendMap()) )
	{
		// no attacking for red on this map
		return;
	}

	for ( int i = 0; i < *m_iNumControlPoints; i ++ )
	{
		arr[i].fProb = 1.0f;
		// not visible
		if ( m_bCPIsVisible[i] == 0 )
			continue;
		// not unlocked
		if ( m_flUnlockTimes[i] > engine->Time() )
			continue;
		// not in round
		if ( m_pControlPoints[i] && pRound && !pRound->isPointInRound(i) )
			continue;

		// We don't own this point
		if ( GetOwningTeam(i) != team )
		{
			// we can capture
			if ( TeamCanCapPoint(i,team) )
			{
				// if we have captured the previous points we can capture
				if ( (prev = GetPreviousPointForPoint(i,team,0)) != -1 )
				{
					if ( prev == i )
					{
						int other = (team==2)?3:2;

						// find the base point
						int basepoint = GetBaseControlPointForTeam(other);

						if ( i == basepoint )
						{
							arr[i].fProb = 0.25f;
						}

						arr[i].bValid = true;
					}
					else
					{
						int j;

						for ( j = 0; j < 3; j ++ )
						{
							prev = GetPreviousPointForPoint(i,team,j);

							if ( prev == -1 )
								continue;
							else if ( GetOwningTeam(GetPreviousPointForPoint(i,team,j)) != team )
								break;
						}

						if ( j == 3 )
							arr[i].bValid = true;
						else
							continue;
					}
				}
				else
				{
					// if its not an attack defend map check previous points are owned
					int other = (team==2)?3:2;

					// find the base point
					int basepoint = GetBaseControlPointForTeam(other);

					if ( i == basepoint )
					{
						arr[i].bValid = true;
						arr[i].fProb = 0.25f;
					}
					else if ( basepoint == 0 )
					{
						bool allowned = true;

						// make sure bot owns all points above this point
						for ( int x = i+1; x < *m_iNumControlPoints; x ++ )
						{
							if ( GetOwningTeam(x) != team )
							{
								allowned = false;
								break;
							}
						}				

						if ( allowned )
							arr[i].bValid  = true;
						
						continue;
					}
					else if ( basepoint == ((*m_iNumControlPoints)-1) )
					{
						bool allowned = true;
						// make sure team owns all points below this point
						for ( int x = 0; x < i; x ++ )
						{
							if ( GetOwningTeam(x) != team )
							{
								allowned = false;
								break;
							}
						}				

						if ( allowned )
							arr[i].bValid  = true;

						continue;
					}

					arr[i].bValid  = true;
				}
			}
		}
	}
}