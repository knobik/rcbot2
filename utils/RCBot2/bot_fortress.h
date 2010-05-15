#ifndef __BOT_FORTRESS_H__
#define __BOT_FORTRESS_H__

#include "bot_utility.h"

class CBotWeapon;
class CWaypoint;
class CBotUtility;

#define TF2_TEAM_BLUE 3
#define TF2_TEAM_RED 2

#define TF2_SENTRY_LEVEL1_HEALTH 150
#define TF2_SENTRY_LEVEL2_HEALTH 180
#define TF2_SENTRY_LEVEL3_HEALTH 216

#define TF2_DISPENSER_LEVEL1_HEALTH 150
#define TF2_DISPENSER_LEVEL2_HEALTH 180
#define TF2_DISPENSER_LEVEL3_HEALTH 216

// Naris @ Alliedmodders.net

#define TF2_PLAYER_SLOWED       (1 << 0)    // 1
#define TF2_PLAYER_ZOOMED       (1 << 1)    // 2
#define TF2_PLAYER_DISGUISING   (1 << 2)    // 4
#define TF2_PLAYER_DISGUISED	(1 << 3)    // 8
#define TF2_PLAYER_CLOAKED      (1 << 4)    // 16
#define TF2_PLAYER_INVULN       (1 << 5)    // 32
#define TF2_PLAYER_HEALING	    (1 << 6)    // 64
#define TF2_PLAYER_TAUNTING	    (1 << 7)    // 128
#define TF2_PLAYER_ONFIRE	    (1 << 14)   // 16384

typedef enum
{
	TF_CLASS_CIVILIAN = 0,
	TF_CLASS_SCOUT,
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HWGUY,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,
	TF_CLASS_UNDEFINED
}TF_Class;

enum
{
	TF_TEAM_SPEC = 0,
	TF_TEAM_BLUE = 1,
	TF_TEAM_RED = 2,
	TF_TEAM_GREEN = 3,
	TF_TEAM_YELLOW = 4
};
/*
typedef enum
{
	ENGI_DISP = 0,
	ENGI_ENTRANCE,
	ENGI_EXIT,
	ENGI_SENTRY,
	ENGI_SAPPER
}eEngiBuild;*/
typedef enum
{
	ENGI_DISP = 0,
	ENGI_TELE,
	ENGI_SENTRY,
	ENGI_SAPPER,
	ENGI_EXIT,
	ENGI_ENTRANCE,
}eEngiBuild;

typedef enum
{
	ENGI_BUILD,
	ENGI_DESTROY
}eEngiCmd;


class CBotTF2FunctionEnemyAtIntel : public IBotFunction
{
public:
	CBotTF2FunctionEnemyAtIntel( int iTeam, Vector vPos, int type ){m_iTeam = iTeam;m_vPos = vPos;m_iType = type;}

	void execute (CBot *pBot);
private:
	int m_iTeam;
	Vector m_vPos;
	int m_iType;
};

class CBroadcastFlagDropped : public IBotFunction
{
public:
	CBroadcastFlagDropped (int iTeam, Vector origin) { m_iTeam = iTeam; m_vOrigin = origin; }
	void execute ( CBot *pBot );

private:
	Vector m_vOrigin;
	int m_iTeam;
};

class CBroadcastFlagCaptured : public IBotFunction
{
public:
	CBroadcastFlagCaptured(int iTeam) { m_iTeam = iTeam; }

	void execute ( CBot *pBot );
private:
	int m_iTeam;
};

class CBroadcastRoundStart : public IBotFunction
{
public:
	CBroadcastRoundStart ( bool bFullReset ) { m_bFullReset = bFullReset; }
	void execute ( CBot *pBot );
private:
	bool m_bFullReset;
};

class CBroadcastCapturedPoint : public IBotFunction
{
public:
	CBroadcastCapturedPoint ( int iPoint, int iTeam, const char *szName );

	void execute ( CBot *pBot );
private:
	int m_iPoint;
	int m_iTeam;
	const char *m_szName;
};

#define EVENT_FLAG_PICKUP 0
#define EVENT_CAPPOINT    1

class CBotFortress : public CBot
{
public:	

	CBotFortress();

	virtual int engiBuildObject ( int *iState, eEngiBuild iObject, float *fTime, int *iTries );

	virtual float getEnemyFactor ( edict_t *pEnemy ) { return CBot::getEnemyFactor(pEnemy); }

	virtual void checkDependantEntities();

	virtual Vector getAimVector ( edict_t *pEntity ) { return CBot::getAimVector(pEntity); }

	virtual void touchedWpt ( CWaypoint *pWaypoint ) { CBot::touchedWpt(pWaypoint); }

	inline edict_t *getHealingEntity () { return m_pHeal; }

	inline void clearHealingEntity () { m_pHeal = NULL; }

	virtual unsigned int maxEntityIndex ( ) { return gpGlobals->maxEntities; }

	virtual void init (bool bVarInit=false);

	virtual void foundSpy (edict_t *pEdict);

	virtual void getTasks ( unsigned int iIgnore = 0 ) { CBot :: getTasks(iIgnore); }

	virtual void died ( edict_t *pKiller );

	virtual void killed ( edict_t *pVictim );

	virtual void modThink ();

	bool wantToHeal ( edict_t *pPlayer );

	virtual bool wantToFollowEnemy ();

	virtual void checkBuildingsValid () {};

	virtual void checkHealingValid ();

	virtual edict_t *findEngineerBuiltObject ( eEngiBuild iBuilding, int index ) { return false; }

	virtual void engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd ) {};

	virtual void spyDisguise ( int iTeam, int iClass ) {};

	virtual bool lookAfterBuildings (float *fTime) { return false; }

	inline void nextLookAfterSentryTime ( float fTime ) { m_fLookAfterSentryTime = fTime; }

	inline edict_t *getSentry () { return m_pSentryGun; }

	virtual bool hasEngineerBuilt ( eEngiBuild iBuilding ) {return false;}

	virtual void engiBuildSuccess ( eEngiBuild iObject, int index ) {};

	virtual bool healPlayer ( edict_t *pPlayer, edict_t *pPrevPlayer ) { return false; }
	virtual bool upgradeBuilding ( edict_t *pBuilding ) {return false;}

	virtual bool isCloaked () { return false; }
	virtual bool isDisguised () { return false; }

	virtual bool handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy ) { return CBot::handleAttack(pWeapon,pEnemy); }

	virtual void setVisible ( edict_t *pEntity, bool bVisible );

	virtual void setClass ( TF_Class _class );

	inline edict_t *seeFlag ( bool reset = false ) { if ( reset ) { m_pFlag = NULL; } return m_pFlag; }

	virtual bool canAvoid ( edict_t *pEntity );

	virtual bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	virtual bool startGame ();

	virtual void spawnInit ();

	bool isTF () { return true; }

	virtual void hurt ( edict_t *pAttacker, int iHealthNow );

	virtual TF_Class getClass () { return TF_CLASS_CIVILIAN; }

	virtual void updateClass () { };

	virtual void currentlyDead ();

	void pickedUpFlag ();

	inline bool hasFlag () { return m_bHasFlag; }

	inline void droppedFlag () { m_bHasFlag = false; }

	bool isAlive ();

	bool isTeleporterUseful ( edict_t *pTele );

	void waitForFlag ( Vector *vOrigin, float *fWait );

	void flagDropped ( Vector vOrigin );
	void teamFlagDropped ( Vector vOrigin );

	inline void flagReset () { m_fLastKnownFlagTime = 0.0f; }
	inline void teamFlagReset () { m_fLastKnownTeamFlagTime = 0.0f; }

	bool canGotoWaypoint ( Vector vPrevWaypoint, CWaypoint *pWaypoint );

	virtual void setup ();

	virtual bool needHealth();

	virtual bool needAmmo ();

	void waitBackstab ();
 
protected:
	virtual void selectTeam ();

	virtual void selectClass ();

	virtual void callMedic ();

	static bool isClassOnTeam ( int iClass, int iTeam );

	static int getSpyDisguiseClass ( int iTeam );

	bool thinkSpyIsEnemy ( edict_t *pEdict );

	float m_fCallMedic;
	float m_fTauntTime;
	float m_fTaunting;
	float m_fDefendTime;

	edict_t *m_pHeal;
	edict_t *m_pSentryGun;
	edict_t *m_pDispenser;
	edict_t *m_pTeleEntrance;
	edict_t *m_pTeleExit;
	edict_t *m_pAmmo;
	edict_t *m_pHealthkit;

	edict_t *m_pNearestDisp;
	edict_t *m_pNearestEnemySentry;
	edict_t *m_pNearestAllySentry;

	edict_t *m_pFlag;
	edict_t *m_pPrevSpy;

	float m_fFrenzyTime;
	float m_fSpyCloakTime;
	float m_fSeeSpyTime;
	float m_fSpyDisguiseTime;
	float m_fLastSaySpy;
	float m_fPickupTime;
	float m_fLookAfterSentryTime;

	// valid flag point area
	Vector m_vLastKnownFlagPoint;
	Vector m_vLastKnownTeamFlagPoint;

	Vector m_vTeleportEntrance;
	bool m_bEntranceVectorValid;
	Vector m_vSentryGun;
	bool m_bSentryGunVectorValid;
	Vector m_vDispenser;
	bool m_bDispenserVectorValid;
	Vector m_vTeleportExit;
	bool m_bTeleportExitVectorValid;

	// 1 minute wait
	float m_fLastKnownFlagTime;
	float m_fLastKnownTeamFlagTime;

	float m_fBackstabTime;

	TF_Class m_iClass;

	float m_fUpdateClass;
	float m_fUseTeleporterTime;

	bool m_bHasFlag;	
	float m_fSnipeAttackTime;
};
//
//
//
//
class CBotTF2 : public CBotFortress
{
public:

	// 
	CBotTF2() 
	{ 
		CBotFortress(); 
		m_fDoubleJumpTime = 0;
		m_fSpySapTime = 0;
		m_iCurrentDefendArea = 0;
		m_iCurrentAttackArea = 0;
	    m_bBlockPushing = false;
	    m_fBlockPushTime = 0;
		m_pDefendPayloadBomb = NULL;
		m_pPushPayloadBomb = NULL;
		m_pRedPayloadBomb = NULL;
		m_pBluePayloadBomb = NULL;
		m_bFixWeapons = false;
		m_bDeployedStickies = false;
	}

	void enemyAtIntel ( Vector vPos, int type = EVENT_FLAG_PICKUP );
	//
	void fixWeapons ();

	void checkDependantEntities ();

	void getDefendArea ( vector<int> *m_iAreas );

	void getAttackArea ( vector <int> *m_iAreas );

	eBotFuncState rocketJump(int *iState,float *fTime);

	float getEnemyFactor ( edict_t *pEnemy );

	void foundSpy (edict_t *pEdict);

	void touchedWpt ( CWaypoint *pWaypoint );

	bool handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy );

	void engiBuildSuccess ( eEngiBuild iObject, int index );

	bool lookAfterBuildings (float *fTime);

	void spawnInit ();

	void setVisible ( edict_t *pEntity, bool bVisible );

	Vector getAimVector ( edict_t *pEntity );

	void modThink ();

	bool isCloaked ();

	bool executeAction ( eBotAction id, CWaypoint *pWaypointResupply, CWaypoint *pWaypointHealth, CWaypoint *pWaypointAmmo );

	void setClass ( TF_Class _class );

	bool isDisguised ();

	void checkBuildingsValid ();

	edict_t *findEngineerBuiltObject ( eEngiBuild iBuilding, int index );

	bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	bool isTF () { return true; }

	void taunt ();

	void callMedic ();

	void roundReset (bool bFullReset);

	void pointCaptured ( int iPoint, int iTeam, const char *szPointName );

	void engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd );

	void spyDisguise ( int iTeam, int iClass );

	bool hasEngineerBuilt ( eEngiBuild iBuilding );

	void getTasks ( unsigned int iIgnore = 0 );

	void died ( edict_t *pKiller );

	void killed ( edict_t *pVictim );

	void capturedFlag ();

	void pointCaptured ();

	virtual bool needAmmo();

	TF_Class getClass ();

	void updateClass ();

	bool healPlayer ( edict_t *pPlayer, edict_t *pPrevPlayer );
	
	bool upgradeBuilding ( edict_t *pBuilding );

	void setup ();

	void buildingSapped ( eEngiBuild building, edict_t *pSapper );

	void sapperDestroyed ( edict_t *pSapper );
	//bool canGotoWaypoint ( CWaypoint *pWaypoint );

	bool deployStickies ( Vector vLocation, Vector vSpread, int *iState, int *iStickyNum, bool *bFail );

	void detonateStickies ();

	bool canDeployStickies ();

private:
	// time for next jump
	float m_fDoubleJumpTime;
	// time bot has taken to sap something
	float m_fSpySapTime;
	// 
	int m_iCurrentDefendArea;
	int m_iCurrentAttackArea;
	//
	bool m_bBlockPushing;
	float m_fBlockPushTime;
	//
	edict_t *m_pDefendPayloadBomb;
	edict_t *m_pPushPayloadBomb;
	edict_t *m_pRedPayloadBomb;
	edict_t *m_pBluePayloadBomb;
	//
	bool m_bFixWeapons;
	// if demoman has already deployed stickies this is true
	// once the demoman explodes them then this becomes false
	// and it can deploy stickies again
	bool m_bDeployedStickies;
};

class CBotFF : public CBotFortress
{
public:

	CBotFF() { CBotFortress(); }

	void modThink ();

	bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	bool isTF () { return true; }

};

#endif