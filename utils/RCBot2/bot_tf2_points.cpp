#include <stdio.h>
#include <math.h>

#include "filesystem.h"
#include "interface.h"
#include "engine/iserverplugin.h"
#include "iplayerinfo.h"
#include "IEngineTrace.h"
#include "tier2/tier2.h"
#ifdef __linux__
#include "shake.h"    //bir3yk
#endif
#include "IEffects.h"
#include "vplane.h"
#include "eiface.h"
#include "igameevents.h"
#include "icvar.h"
//#include "iconvar.h"
#include "convar.h"
#include "Color.h"
#include "ndebugoverlay.h"
#include "server_class.h"
#include "time.h"
#include "bot.h"
#include "bot_getprop.h"
#include "bot_globals.h"

#include "vstdlib/random.h" // for random  seed 

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#include "bot.h"
#include "bot_tf2_points.h"

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