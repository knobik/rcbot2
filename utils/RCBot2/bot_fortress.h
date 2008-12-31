#ifndef __BOT_FORTRESS_H__
#define __BOT_FORTRESS_H__

#define TF2_TEAM_BLUE 3
#define TF2_TEAM_RED 2

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

typedef enum
{
	ENGI_DISP = 0,
	ENGI_ENTRANCE,
	ENGI_EXIT,
	ENGI_SENTRY,
	
}eEngiBuild;

typedef enum
{
	ENGI_BUILD,
	ENGI_DESTROY
}eEngiCmd;

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

class CBotFortress : public CBot
{
public:	

	CBotFortress();

	inline edict_t *getHealingEntity () { return m_pHeal; }

	virtual unsigned int maxEntityIndex ( ) { return gpGlobals->maxEntities; }

	virtual void init (bool bVarInit=false);

	virtual void getTasks ( unsigned int iIgnore = 0 ) { CBot :: getTasks(iIgnore); }

	virtual void died ( edict_t *pKiller );

	virtual void killed ( edict_t *pVictim );

	virtual void modThink ();

	virtual void checkBuildingsValid () {};
	virtual void checkHealingValid ();

	virtual edict_t *findEngineerBuiltObject ( eEngiBuild iBuilding ) { return false; }

	virtual void engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd ) {};

	virtual void spyDisguise ( int iTeam, int iClass ) {};

	virtual bool hasEngineerBuilt ( eEngiBuild iBuilding ) {return false;}

	virtual void engiBuildSuccess ( eEngiBuild iObject ) {};

	virtual bool isCloaked () { return false; }
	virtual bool isDisguised () { return false; }

	void setVisible ( edict_t *pEntity, bool bVisible );

	virtual void setClass ( TF_Class _class );

	inline edict_t *seeFlag ( bool reset = false ) { if ( reset ) { m_pFlag = NULL; } return m_pFlag; }

	void setLookAtTask ( eLookTask lookTask );

	virtual bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	virtual bool startGame ();

	virtual void spawnInit ();

	bool isTF () { return true; }

	virtual TF_Class getClass () { return TF_CLASS_CIVILIAN; }

	virtual void updateClass () { };

	virtual void currentlyDead ();

	inline void pickedUpFlag () { m_bHasFlag = true; }

	inline bool hasFlag () { return m_bHasFlag; }

	inline void droppedFlag () { m_bHasFlag = false; }

	bool isAlive ();

	void flagDropped ( Vector vOrigin );

	inline void flagReset () { m_fLastKnownFlagTime = 0.0f; }

	bool canGotoWaypoint ( Vector vPrevWaypoint, CWaypoint *pWaypoint );

	virtual void setup ();

protected:
	virtual void selectTeam ();

	virtual void selectClass ();

	virtual void callMedic ();

	float m_fCallMedic;
	float m_fTauntTime;
	float m_fTaunting;

	edict_t *m_pHeal;
	edict_t *m_pSentryGun;
	edict_t *m_pDispenser;
	edict_t *m_pTeleEntrance;
	edict_t *m_pTeleExit;

	edict_t *m_pFlag;

	// valid flag point area
	Vector m_vLastKnownFlagPoint;

	// 1 minute wait
	float m_fLastKnownFlagTime;

	TF_Class m_iClass;

	float m_fUpdateClass;

	bool m_bHasFlag;

	
	
};

class CBotTF2 : public CBotFortress
{
public:

	CBotTF2() { CBotFortress(); }

	void engiBuildSuccess ( eEngiBuild iObject );

	void spawnInit ();

	void modThink ();

	bool isCloaked ();

	void setClass ( TF_Class _class );

	bool isDisguised ();

	void checkBuildingsValid ();

	edict_t *findEngineerBuiltObject ( eEngiBuild iBuilding );

	bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	bool isTF () { return true; }

	void taunt ();

	void callMedic ();

	void engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd );

	void spyDisguise ( int iTeam, int iClass );

	bool hasEngineerBuilt ( eEngiBuild iBuilding );

	void getTasks ( unsigned int iIgnore = 0 );

	void died ( edict_t *pKiller );

	void killed ( edict_t *pVictim );

	void capturedFlag ();

	void capturedPoint ();

	TF_Class getClass ();

	void updateClass ();

	void setup ();
	//bool canGotoWaypoint ( CWaypoint *pWaypoint );

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