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
#ifndef __DOD_RCBOT_H__
#define __DOD_RCBOT_H__

#define TEAM_ALLIES 2
#define TEAM_AXIS 3

typedef enum
{
 DOD_CLASS_RIFLEMAN = 0,
 DOD_CLASS_ASSAULT,
 DOD_CLASS_SUPPORT,
 DOD_CLASS_SNIPER,
 DOD_CLASS_MACHINEGUNNER,
 DOD_CLASS_ROCKET
}DOD_Class;

#define DOD_CLASSNAME_CONTROLPOINT "dod_control_point"

// bot for DOD
class CDODBot : public CBot
{
public:

	bool isDOD () { return true; }

	void modThink ();

	void init ();
	void setup ();

	bool startGame ();

	void died ( edict_t *pKiller );
	void killed ( edict_t *pVictim );

	void spawnInit ();

	bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	float getArmorPercent () { return (0.01f * m_pPlayerInfo->GetArmorValue()); }

	void getTasks (unsigned int iIgnore);

	void fixWeapons ();

	bool executeAction ( eBotAction iAction, CBotWeapon *pWeaponChoice );

	void selectedClass ( int iClass );

	void setVisible ( edict_t *pEntity, bool bVisible );

	bool select_CWeapon ( CWeapon *pWeapon );

	bool selectBotWeapon ( CBotWeapon *pBotWeapon );

	bool canGotoWaypoint (Vector vPrevWaypoint, CWaypoint *pWaypoint);

	void defending ();

	bool handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy );

	void handleWeapons ();

	void reachedCoverSpot ();

	void touchedWpt ( CWaypoint *pWaypoint );

	bool checkStuck ();

private:

	int m_iSelectedClass;

	eBotAction m_CurrentUtil;

	edict_t *m_pCurrentWeapon;

	CBaseHandle *m_Weapons;

	float m_fFixWeaponTime;
	float m_flSprintTime;

	float m_flStamina;
	bool m_bProne;

	int m_iClip1;
	int m_iClip2;

	int m_iTeam;

	edict_t *m_pNearestFlag;
	edict_t *m_pGoalFlag;

	DOD_Class m_iClass;
	float m_fShootTime;
	float m_fZoomOrDeployTime;

	float m_fProneTime;

	// blah blah
};

#endif