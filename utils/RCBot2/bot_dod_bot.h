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

#define SMOKE_RADIUS 128

typedef enum
{
   DOD_VC_GOGOGO = 0,
	DOD_VC_YES = 1,
	DOD_VC_DROPWEAP = 2,
    DOD_VC_HOLD = 3,
	DOD_VC_NO = 4,
	DOD_VC_DISPLACE = 5,
    DOD_VC_GO_LEFT = 6,
    DOD_VC_NEED_BACKUP = 7,
	DOD_VC_MGAHEAD = 8,
    DOD_VC_GO_RIGHT = 9,
    DOD_VC_FIRE_IN_THE_HOLE =10,
    DOD_VC_ENEMY_BEHIND = 11,
    DOD_VC_STICK_TOGETHER = 12,
	DOD_VC_USE_GRENADE = 13,
	DOD_VC_ENEMY_DOWN = 14,
    DOD_VC_COVERING_FIRE = 15,
    DOD_VC_SNIPER = 16,
    DOD_VC_NEED_MG = 17,
    DOD_VC_SMOKE = 18,
    DOD_VC_NICE_SHOT = 19,
    DOD_VC_NEED_AMMO = 20,
    DOD_VC_GRENADE2 = 21,
    DOD_VC_THANKS = 22,
    DOD_VC_USE_BAZOOKA = 23,
	DOD_VC_CEASEFIRE = 24,
	DOD_VC_AREA_CLEAR = 25,
	DOD_VC_BAZOOKA = 26,
	DOD_VC_INVALID = 27
}eDODVoiceCMD;

typedef struct
{
	eDODVoiceCMD id;
	char *pcmd;
}eDODVoiceCommand_t;

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

typedef struct
{
	float fLastTime;
	float fProb;
	bool bLastResult;
	bool bInSmoke;
}smoke_t;

// bot for DOD
class CDODBot : public CBot
{
public:

	bool isDOD () { return true; }

	void modThink ();

	void init ();
	void setup ();

	Vector getAimVector ( edict_t *pEntity );

	bool startGame ();

	void died ( edict_t *pKiller );
	void killed ( edict_t *pVictim );

	void spawnInit ();

	bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	float getArmorPercent () { return (0.01f * m_pPlayerInfo->GetArmorValue()); }

	void getTasks (unsigned int iIgnore);

	void fixWeapons ();

	bool executeAction ( CBotUtility *util );

	void selectedClass ( int iClass );

	void setVisible ( edict_t *pEntity, bool bVisible );

	bool select_CWeapon ( CWeapon *pWeapon );

	bool selectBotWeapon ( CBotWeapon *pBotWeapon );

	bool canGotoWaypoint (Vector vPrevWaypoint, CWaypoint *pWaypoint);

	void defending ();

	bool handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy );

	void hearVoiceCommand ( edict_t *pPlayer, byte cmd );

	void handleWeapons ();

	void reachedCoverSpot ();

	void touchedWpt ( CWaypoint *pWaypoint );

	bool checkStuck ();

	void voiceCommand ( eDODVoiceCMD cmd );

	virtual unsigned int maxEntityIndex ( ) { return gpGlobals->maxEntities; }

	void seeFriendlyDie ( edict_t *pDied, edict_t *pKiller, CWeapon *pKillerWeapon );

	bool isVisibleThroughSmoke ( edict_t *pSmoke, edict_t *pCheck );

	void grenadeThrown () { voiceCommand(DOD_VC_FIRE_IN_THE_HOLE); }

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

	int m_iTeam; // either 2 / 3 TEAM_ALLIES/TEAM_AXIS
	int m_iEnemyTeam; // is the opposite of m_iTeam to check for enemy things

	edict_t *m_pNearestFlag;
	edict_t *m_pGoalFlag;

	DOD_Class m_iClass;
	float m_fShootTime;
	float m_fZoomOrDeployTime;

	float m_fProneTime;

	// EHandles cos they will be destroyed soon
	MyEHandle m_pEnemyRocket;
	float m_fShoutRocket;
	MyEHandle m_pEnemyGrenade;
	float m_fShoutGrenade;
	MyEHandle m_pOwnGrenade;
	MyEHandle m_pNearestSmokeToEnemy;

	float m_fChangeClassTime;
	bool m_bCheckClass;

	smoke_t m_CheckSmoke[MAX_PLAYERS];

	float m_fDeployMachineGunTime;
	// blah blah
};

#endif