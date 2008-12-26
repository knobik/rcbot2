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


class CBotFortress : public CBot
{
public:	

	CBotFortress() { CBot(); m_fCallMedic = 0; }

	virtual unsigned int maxEntityIndex ( ) { return gpGlobals->maxEntities; }

	virtual void init (bool bVarInit=false);
	// setup buttons and data structures
	virtual void setup ();

	virtual void died ( edict_t *pKiller );

	virtual void killed ( edict_t *pVictim );

	virtual void modThink ();

	void setLookAtTask ( eLookTask lookTask );

	virtual bool isEnemy ( edict_t *pEdict );

	virtual bool startGame ();

	virtual void spawnInit ();

	bool isTF () { return true; }

	//virtual TF_Class getClass () = 0;

protected:
	virtual void selectTeam ();

	virtual void selectClass ();

	virtual void callMedic ();

	float m_fCallMedic;

	
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