#ifndef __BOT_FORTRESS_H__
#define __BOT_FORTRESS_H__

typedef enum
{
	TF_CLASS_SCOUT = 1,
	TF_CLASS_SNIPER = 2,
	TF_CLASS_SOLDIER = 3,
	TF_CLASS_DEMOMAN = 4,
	TF_CLASS_MEDIC = 5,
	TF_CLASS_HWGUY = 6,
	TF_CLASS_PYRO = 7,
	TF_CLASS_SPY = 8,
	TF_CLASS_ENGINEER = 9,
	TF_CLASS_CIVILIAN = 0,
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

typedef enum
{
	TF_MAP_DM = 0,
	TF_MAP_CTF,
	TF_MAP_CP,
	TF_MAP_CART
}eTFMapType;

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

	CBotFortress() { CBot(); m_fCallMedic = 0; m_fTauntTime = 0; m_fTaunting = 0; m_fLastKnownFlagTime = 0.0f; m_bHasFlag = false; }

	virtual unsigned int maxEntityIndex ( ) { return gpGlobals->maxEntities; }

	virtual void init (bool bVarInit=false);
	// setup buttons and data structures
	virtual void setup ();

	virtual void getTasks ( unsigned int iIgnore = 0 ) { CBot :: getTasks(iIgnore); }

	virtual void died ( edict_t *pKiller );

	virtual void killed ( edict_t *pVictim );

	virtual void modThink ();

	void setLookAtTask ( eLookTask lookTask );

	virtual bool isEnemy ( edict_t *pEdict );

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

protected:
	virtual void selectTeam ();

	virtual void selectClass ();

	virtual void callMedic ();

	float m_fCallMedic;
	float m_fTauntTime;
	float m_fTaunting;

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

	void modThink ();

	bool isEnemy ( edict_t *pEdict );

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

};

class CBotFF : public CBotFortress
{
public:

	CBotFF() { CBotFortress(); }

	void modThink ();

	bool isEnemy ( edict_t *pEdict );

	bool isTF () { return true; }

};

#endif