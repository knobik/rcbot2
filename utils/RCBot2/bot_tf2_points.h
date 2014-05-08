#ifndef __RCBOT_TF2_POINTS_H__
#define __RCBOT_TF2_POINTS_H__

#include "utlmap.h"

class CTeamControlPointRound
{
public:

	bool isPointInRound ( int iIndex );

	CUtlVector< CBaseHandle > m_ControlPoints;

	bool m_bDisabled;

	string_t	m_iszCPNames;
	int			m_nPriority;
	int			m_iInvalidCapWinner;
	string_t	m_iszPrintName;
};

class CTeamControlPointMaster
{
public:

	CUtlMap<int, CBaseEntity *> m_ControlPoints;

	bool m_bFoundPoints;		// true when the control points have been found and the array is initialized

	CTeamControlPointRound *getCurrentRound ( );

	CUtlVector<CBaseEntity *> m_ControlPointRounds;
	int m_iCurrentRoundIndex;

	bool m_bDisabled;

	string_t m_iszTeamBaseIcons[MAX_TEAMS];
	int m_iTeamBaseIcons[MAX_TEAMS];
	string_t m_iszCapLayoutInHUD;

	float m_flCustomPositionX;
	float m_flCustomPositionY;

	int m_iInvalidCapWinner;
	bool m_bSwitchTeamsOnWin;
	bool m_bScorePerCapture;
	bool m_bPlayAllRounds;

	bool m_bFirstRoundAfterRestart;

};


class CEventAction;
class CSoundPatch;

class CBaseEntityOutput
{
public:
	union
	{
		bool bVal;
		string_t iszVal;
		int iVal;
		float flVal;
		float vecVal[3];
		color32 rgbaVal;
	};
	CHandle<CBaseEntity> eVal; // this can't be in the union because it has a constructor.

	fieldtype_t fieldType;
	// end variant_t
	CEventAction *m_ActionList_Ptr;
};

class CTeamControlPoint
{
public:
	int			m_iTeam;			
	int			m_iDefaultOwner;			// Team that initially owns the cap point
	int			m_iIndex;					// The index of this point in the controlpointArray
	int			m_iWarnOnCap;				// Warn the team that owns the control point when the opposing team starts to capture it.
	string_t	m_iszPrintName;
	string_t	m_iszWarnSound;				// Sound played if the team needs to be warned about this point being captured
	bool		m_bRandomOwnerOnRestart;	// Do we want to randomize the owner after a restart?
	bool		m_bLocked;
	float		m_flUnlockTime;				// Time to unlock

	// We store a copy of this data for each team, +1 for the un-owned state.
	struct perteamdata_t
	{
		perteamdata_t()
		{
			iszCapSound = NULL_STRING;
			iszModel = NULL_STRING;
			iModelBodygroup = -1;
			iIcon = 0;
			iszIcon = NULL_STRING;
			iOverlay = 0;
			iszOverlay = NULL_STRING;
			iPlayersRequired = 0;
			iTimedPoints = 0;
			for ( int i = 0; i < MAX_PREVIOUS_POINTS; i++ )
			{
				iszPreviousPoint[i] = NULL_STRING;
			}
			iTeamPoseParam = 0;
		}

		string_t	iszCapSound;
		string_t	iszModel;
		int			iModelBodygroup;
		int			iTeamPoseParam;
		int			iIcon;
		string_t	iszIcon;
		int			iOverlay;
		string_t	iszOverlay;
		int			iPlayersRequired;
		int			iTimedPoints;
		string_t	iszPreviousPoint[MAX_PREVIOUS_POINTS];
	};
	CUtlVector<perteamdata_t>	m_TeamData;

	CBaseEntityOutput	m_OnCapReset;

	CBaseEntityOutput	m_OnCapTeam1;
	CBaseEntityOutput	m_OnCapTeam2;

	CBaseEntityOutput	m_OnOwnerChangedToTeam1;
	CBaseEntityOutput	m_OnOwnerChangedToTeam2;

	CBaseEntityOutput	m_OnRoundStartOwnedByTeam1;
	CBaseEntityOutput	m_OnRoundStartOwnedByTeam2;

	CBaseEntityOutput	m_OnUnlocked;

	int			m_bPointVisible;		//should this capture point be visible on the hud?
	int			m_iPointIndex;			//the mapper set index value of this control point

	int			m_iCPGroup;			//the group that this control point belongs to
	bool		m_bActive;			//

	string_t	m_iszName;				//Name used in cap messages

	bool		m_bStartDisabled;

	float		m_flLastContestedAt;

	CSoundPatch *m_pCaptureInProgressSound;
	string_t	m_iszCaptureStartSound;
	string_t	m_iszCaptureEndSound;
	string_t	m_iszCaptureInProgress;
	string_t	m_iszCaptureInterrupted;
};

#endif