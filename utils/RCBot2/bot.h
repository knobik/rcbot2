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
//=============================================================================//
//
// HPB_bot2.h - bot header file (Copyright 2004, Jeffrey "botman" Broome)
//
//=============================================================================//

#ifndef __RCBOT2_H__
#define __RCBOT2_H__

//#include "cbase.h"
//#include "baseentity.h"
#include "filesystem.h"
#include "interface.h"
#include "engine/iserverplugin.h"
#include "iplayerinfo.h"
#include "IEngineTrace.h" // for traceline functions
#include "IEffects.h"
#include "vplane.h"
#include "eiface.h"
#include "igameevents.h"
#include "Color.h"
#include "usercmd.h"

#include "bot_const.h"

#define MAX_AMMO_TYPES 32

// Interfaces from the engine
//using namespace VEngineServerV21;
//using namespace ServerGameClientsV3;
extern IVEngineServer *engine;  // helper functions (messaging clients, loading content, making entities, running commands, etc)
extern IFileSystem *filesystem;  // file I/O 
//extern IGameEventManager *gameeventmanager;  // game events interface
extern IGameEventManager2 *gameeventmanager;
extern IPlayerInfoManager *playerinfomanager;  // game dll interface to interact with players
extern IServerPluginHelpers *helpers;  // special 3rd party plugin helpers from the engine
extern IServerGameClients* gameclients;
extern IEngineTrace *enginetrace;
extern IEffects *g_pEffects;
extern IBotManager *g_pBotManager;
extern CGlobalVars *gpGlobals;

#define GET_HEALTH 0
#define GET_TEAM   1
#define GET_AMMO   2

#define T_OFFSETMAX  3

class CClassInterface
{
public:
    static int getFlags ( edict_t *edict );
	static int getTeam ( edict_t *edict );
	static int getHealth ( edict_t *edict );
	static int getEffects ( edict_t *edict );
	static int *getAmmoList ( edict_t *edict );
	static unsigned int findOffset(const char *szType,const char *szClass);
	static int getTF2NumHealers ( edict_t *edict );
	static int getTF2Conditions ( edict_t *edict );
	static Vector *getVelocity ( edict_t *edict );
	static int getTF2Class ( edict_t *edict );
	static float getTF2SpyCloakMeter ( edict_t *edict );
	static bool getTF2SpyDisguised( edict_t *edict, int *_class, int *_team, int *_index, int *_health );
	static bool getMedigunHealing ( edict_t *edict );
	static int getMedigunTarget ( edict_t *edict );
};

class MyEHandle 
{
public:
	MyEHandle ()
	{
		m_pEnt = NULL;
		m_iSerialNumber = 0;
	}

    MyEHandle ( edict_t *pent )
	{
		m_pEnt = pent;

		if ( pent )
			m_iSerialNumber = pent->m_NetworkSerialNumber;
	}

	edict_t *get ()
	{
		if ( m_pEnt )
		{
			if ( !m_pEnt->IsFree() && (m_iSerialNumber == m_pEnt->m_NetworkSerialNumber) )
				return m_pEnt;
		}

		return NULL;
	}
private:
	int m_iSerialNumber;
	edict_t *m_pEnt;
};

class CRCBotEventListener : public IGameEventListener2
{
	void FireGameEvent( IGameEvent *event );
};
//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
class CRCBotPlugin : public IServerPluginCallbacks, public IGameEventListener
{
public:
	CRCBotPlugin();
	~CRCBotPlugin();

	// IServerPluginCallbacks methods
	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory );
	virtual void			Unload( void );
	virtual void			Pause( void );
	virtual void			UnPause( void );
	virtual const char     *GetPluginDescription( void );      
	virtual void			LevelInit( char const *pMapName );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void			GameFrame( bool simulating );
	virtual void			LevelShutdown( void );
	virtual void			ClientActive( edict_t *pEntity );
	virtual void			ClientDisconnect( edict_t *pEntity );
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername );
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict );
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args );
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID );
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue );

	// IGameEventListener Interface
	virtual void FireGameEvent( KeyValues * event );
	//virtual void FireGameEvent( IGameEvent * event );

	virtual int GetCommandIndex() { return m_iClientCommandIndex; }

	static void HudTextMessage ( edict_t *pEntity, char *szMsgName, char *szTitle, char *szMessage, Color colour = Color( 255, 0, 0, 255 ), int level = 5, int time = 6 );

	static void ShowLicense ( void );

	CRCBotEventListener *getEventListener ( void );
private:
	int m_iClientCommandIndex;
	CRCBotEventListener *eventListener2;
};



extern CRCBotPlugin g_RCBOTServerPlugin;

//extern IVDebugOverlay *debugoverlay;

class CBotVisibles;
class CFindEnemyFunc;
class IBotNavigator;
class CBotMemoryNode;
class CBotMemoryPop;
class CGA;
class CPerceptron;
class CBotSchedules;
class CBotGAValues;
class CBotStuckValues;
class CBotButtons;
class IBotNavigator;
class CFindEnemyFunc;
class CBotWeapons;
class CBotProfile;
class CWaypoint;
class CBotWeapon;
class CWeapon;

class CBot 
{
public:

    static const float m_fAttackLowestHoldTime;
	static const float m_fAttackHighestHoldTime;
    static const float m_fAttackLowestLetGoTime;
	static const float m_fAttackHighestLetGoTime;

	CBot();

	void debugMsg ( int iLev, const char *szMsg );

	virtual unsigned int maxEntityIndex ( ) { return MAX_PLAYERS; }

	inline float CBot :: distanceFrom(edict_t *pEntity)
	{
		return (pEntity->GetIServerEntity()->GetCollideable()->GetCollisionOrigin()-m_pController->GetLocalOrigin()).Length();
	//return distanceFrom(CBotGlobals::entityOrigin(pEntity));
	}
	// return distance from this origin
	inline float CBot :: distanceFrom ( Vector vOrigin )
	{
		return (vOrigin-m_pController->GetLocalOrigin()).Length();
	}

	inline Vector getOrigin ();
	/*
	 * init()
	 *
	 * initialize all bot variables 
	 * (this is called when bot is made for the first time)
	 */
	virtual void init (bool bVarInit=false);
	// setup buttons and data structures
	virtual void setup ();

    /*
	 * runPlayerMove()
	 *
	 * make bot move in the game
	 * botman : see CBasePlayer::RunNullCommand() for example of PlayerRunCommand()...
	 */
	void runPlayerMove();

	/*
	 * called when a bot dies
	 */
	virtual void died ( edict_t *pKiller );
	virtual void killed ( edict_t *pVictim );

	virtual int getTeam ();

	bool isUnderWater ( );

	CBotWeapon *getBestWeapon ( edict_t *pEnemy );

	void setLookAtTask ( eLookTask lookTask, int iPriority = 1 );

	virtual void modThink () { return; }

	virtual bool isEnemy ( edict_t *pEdict, bool bCheckWeapons = true ) { return false; }

	inline bool hasSomeConditions ( int iConditions )
	{
		return (m_iConditions & iConditions) > 0;
	}

	virtual bool handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy );

	float DotProductFromOrigin ( Vector &pOrigin );

	bool FVisible ( edict_t *pEdict );

	bool isVisible ( edict_t *pEdict );

	inline void setEnemy ( edict_t *pEnemy )
	{
		m_pEnemy = pEnemy;
	}

	inline bool hasAllConditions ( int iConditions )
	{
		return (m_iConditions & iConditions) == iConditions;
	}

	inline void updateCondition ( int iCondition )
	{
		m_iConditions |= iCondition;
	}

	inline void removeCondition ( int iCondition )
	{
		m_iConditions &= ~iCondition;
	}

	bool FInViewCone ( edict_t *pEntity );	

	/*
	 * make bot start the gmae, e.g join a team first
	 */
	virtual bool startGame ();

	virtual bool checkStuck ();

	virtual void currentlyDead ();

	/*
	 * initialize this bot as a new bot with the edict of pEdict
	 */
	bool createBotFromEdict(edict_t *pEdict, CBotProfile *pProfile);

	/*
	 * returns true if bot is used in game
	 */
	inline bool inUse ()
	{
		return (m_bUsed && (m_pEdict!=NULL));
	}

	edict_t *getEdict ();

	inline void setEdict ( edict_t *pEdict);

	bool FVisible ( Vector &vOrigin );

	Vector getEyePosition ();

	void think ();

	virtual void freeMapMemory ();

	virtual void freeAllMemory ();

	///////////////////////////////
	bool moveToIsValid ()
	{
		return m_bMoveToIsValid;
	}

	bool lookAtIsValid ()
	{
		return m_bLookAtIsValid;
	}

	inline Vector getMoveTo ()
	{
		return m_vMoveTo;
	}

	inline bool moveFailed ()
	{
		bool ret = m_bFailNextMove;

		m_bFailNextMove = false;

		return ret;
	}

	void selectWeaponSlot ( int iSlot );

	edict_t *getAvoidEntity () { return m_pAvoidEntity; }

	void setAvoidEntity ( edict_t *pEntity ) { m_pAvoidEntity = pEntity; }

	void updateConditions ();

	virtual bool canAvoid ( edict_t *pEntity );

	inline bool hasEnemy () { return m_pEnemy && hasSomeConditions(CONDITION_SEE_CUR_ENEMY); }
	edict_t *getEnemy () { return m_pEnemy; }

	void setLookAt ( Vector vNew );

	inline void setMoveTo ( Vector vNew, int iPriority = 1 )
	{
		if ( iPriority > m_iMovePriority )
		{
			m_vMoveTo = vNew;
			m_bMoveToIsValid = true;
			m_iMovePriority = iPriority;
		}
	}

	void findEnemy ( edict_t *pOldEnemy = NULL );
	void enemyFound ( edict_t *pEnemy );

	virtual void checkDependantEntities ();

	inline IBotNavigator *getNavigator () { return m_pNavigator; }

	void stopMoving (int iPriority = 1);

	inline void stopLooking ( int iPriority = 1 ) 
	{ 
		if ( iPriority > m_iLookPriority )
		{
			m_bLookAtIsValid = false; 
			m_iLookPriority = iPriority;
		}
	}
	//////////////////////
	virtual bool isCSS () { return false; }
	virtual bool isHLDM () { return false; }
	virtual bool isTF () { return false; }

	virtual void spawnInit ();

	QAngle eyeAngles ();

	virtual bool isAlive ();

	bool onLadder ();

	inline bool currentEnemy ( edict_t *pEntity ) { return m_pEnemy == pEntity; }

	virtual Vector getAimVector ( edict_t *pEntity );

	inline Vector *getGoalOrigin ()
	{
		return &m_vGoal;
	}

	inline bool hasGoal ()
	{
		return m_bHasGoal;
	}

	void primaryAttack (bool bHold = false);
	void secondaryAttack(bool bHold=false);
	void jump ();
	void duck ( bool hold = false );

	virtual void setVisible ( edict_t *pEntity, bool bVisible );

	virtual void hurt ( edict_t *pAttacker, int iHealthNow );
	virtual void shot ( edict_t *pEnemy );
	virtual void shotmiss ();
	//inline void setAvoidEntity (edict_t *pEntity) { m_pAvoidEntity = pEntity; };
	
	int getPlayerID (); // return player ID on server
	int getHealth ();

	float getHealthPercent ();

	inline CBotSchedules *getSchedule () { return m_pSchedules; }

	void reachedCoverSpot ();

	virtual bool wantToFollowEnemy ();

	inline void selectWeapon ( int iWeaponId ) { m_iSelectWeapon = iWeaponId; }

	void selectWeaponName ( const char *szWeaponName );

	CBotWeapon *getCurrentWeapon ();

	bool isUsingProfile ( CBotProfile *pProfile );

	inline CBotProfile *getProfile () { return m_pProfile; }

	virtual bool canGotoWaypoint ( Vector vPrevWaypoint, CWaypoint *pWaypoint );

	void tapButton ( int iButton );

	inline int getAmmo ( int iIndex ) { if ( !m_iAmmo ) return 0; else return m_iAmmo[iIndex]; }

	void lookAtEdict ( edict_t *pEdict );

	bool select_CWeapon ( CWeapon *pWeapon );
	bool selectBotWeapon ( CBotWeapon *pBotWeapon );

	void updatePosition ();

	edict_t *m_pLookEdict;

	CBotWeapons *getWeapons () { return m_pWeapons; }

	virtual float getEnemyFactor ( edict_t *pEnemy );

	virtual void checkCanPickup ( edict_t *pPickup );

	virtual void touchedWpt ( CWaypoint *pWaypoint );

	inline void setAiming ( Vector aiming ) { m_vWaypointAim = aiming; }

	inline Vector getAiming () { return m_vWaypointAim; }

	inline void setLookVector ( Vector vLook ) { m_vLookVector = vLook; }

	inline Vector getLookVector () { return m_vLookVector; }

	//inline void dontAvoid () { m_fAvoidTime = engine->Time() + 1.0f; }

	float m_fWaypointStuckTime;

	inline float getSpeed () { return m_vVelocity.Length2D(); }

protected:

	static void checkEntity ( edict_t **pEdict );
	/////////////////////////
	void doMove ();

	void doLook ();

	virtual void getLookAtVector ();

	void doButtons ();
	/////////////////////////

	void changeAngles ( float fSpeed, float *fIdeal, float *fCurrent, float *fUpdate );

	// look for new tasks
	virtual void getTasks (unsigned int iIgnore=0);

	// really only need 249 bits (32 bytes) + WEAPON_SUBTYPE_BITS (whatever that is)
	static const int CMD_BUFFER_SIZE = 64; 
	///////////////////////////////////
	// bots edict
	edict_t *m_pEdict;
	edict_t *m_pAvoidEntity;
	// is bot used in the game?
	bool m_bUsed;
	// time the bot was made in the server
	float m_fTimeCreated;
	// next think time
	float m_fNextThink;

	
	bool m_bInitAlive;
	bool m_bThinkStuck;

	int m_iMovePriority;
	int m_iLookPriority;

	int *m_iAmmo;
	bool m_bLookedForEnemyLast;

	//CBotStuckValues *m_pGAvStuck;
	//CGA *m_pGAStuck;
	//CPerceptron *m_pThinkStuck;
	Vector m_vStuckPos;
	//int m_iTimesStuck;
	float m_fAvoidTime;
	///////////////////////////////////
	// current impulse command
	int m_iImpulse;
	// buttons held
	int m_iButtons;
	// bots forward move speed
	float m_fForwardSpeed;
	// bots side move speed
	float m_fSideSpeed;
	// bots upward move speed (e.g in water)
	float m_fUpSpeed;

	float m_fLookAtTimeStart;
	float m_fLookAtTimeEnd;
	// Look task can't be changed if this is greater than Time()
	float m_fLookSetTime; 
	float m_fLookAroundTime;

	int m_iFlags;
	// Origin a second ago to check if stuck
	Vector m_vLastOrigin;
	// Generated velocity found from last origin (not always correct)
	Vector m_vVelocity;
	// next update time (1 second update)
	float m_fUpdateOriginTime;
	float m_fStuckTime;
	float m_fCheckStuckTime;
	float m_fNextUpdateStuckConstants;

	float m_fStrafeTime;

	float m_fUpdateDamageTime;
	// Damage bot accumulated over the last second or so
	int m_iAccumulatedDamage;
	int m_iPrevHealth;
	///////////////////////////////////
	int m_iConditions;
	// bot tasks etc -- complex actuators
	CBotSchedules *m_pSchedules;
	// buttons held -- simple actuators
	CBotButtons *m_pButtons;
	// Navigation used for this bot -- environment sensor 1
	IBotNavigator *m_pNavigator;
	// bot visible list -- environment sensor 2
	CBotVisibles *m_pVisibles;
	// visible functions -- sensory functions
	CFindEnemyFunc *m_pFindEnemyFunc;
	// weapons storage -- sensor
	CBotWeapons *m_pWeapons;
	////////////////////////////////////
	IPlayerInfo *m_pPlayerInfo; //-- sensors
	IBotController *m_pController; //-- actuators
	CBotCmd cmd; // actuator command
	////////////////////////////////////
	edict_t *m_pEnemy; // current enemy
	edict_t *m_pOldEnemy;
	Vector m_vLastSeeEnemy;
	edict_t *m_pLastEnemy; // enemy we were fighting before we lost it
	//edict_t *m_pAvoidEntity; // avoid this guy
	Vector m_vHurtOrigin;
	Vector m_vLookVector;
	Vector m_vLookAroundOffset;
	edict_t *m_pPickup;
	Vector m_vWaypointAim;

	Vector m_vMoveTo;
	Vector m_vLookAt;
	Vector m_vGoal; // goal vector
	bool m_bHasGoal;
	QAngle m_vViewAngles;
	float m_fNextUpdateAimVector;
	Vector m_vAimVector;

	eLookTask m_iLookTask;

	bool m_bMoveToIsValid;
	bool m_bLookAtIsValid;

	float m_fIdealMoveSpeed;

	float m_fLastWaypointVisible;

	bool m_bFailNextMove;

	int m_iSelectWeapon;

	int m_iDesiredTeam;
	int m_iDesiredClass;

	// bots profile data
	CBotProfile *m_pProfile;

	bool wantsToShoot ();
	/////////////////////////////////

	char m_szBotName[64];
};

class IBotFunction
{
public:
	virtual void execute ( CBot *pBot ) = 0;
};

class CBots
{
public:
	static void botThink ();

	static CBot *getBotPointer ( edict_t *pEdict );

	static void freeMapMemory ();

	static void freeAllMemory ();

	static CBot *findBotByProfile ( CBotProfile *pProfile );

	static void init ();

	static bool createBot (const char *szClass, const char *szTeam, const char *szName);

	static int numBots ();

	static int slotOfEdict ( edict_t *pEdict );

	static void roundStart ();

	static void kickRandomBot ();

	static void kickRandomBotOnTeam ( int team );

	static void mapInit ();

	static bool needToAddBot ();

	static bool needToKickBot ();

	static void setMaxBots ( int iMax ) { m_iMaxBots = iMax; }

	static int getMaxBots () { return m_iMaxBots; }

	static void setMinBots ( int iMin ) { m_iMinBots = iMin; }

	static int getMinBots () { return m_iMinBots; }

	static void botFunction ( IBotFunction *function );

private:
	static CBot **m_Bots;

	//config
	static int m_iMaxBots;
	static int m_iMinBots;

	// add or kick bot time
	static float m_flAddKickBotTime;
};

void DrawLine ( const Vector &origin, const Vector &target, int r, int g, int b, bool noDepthTest, float duration );

void WriteUsercmd( bf_write *buf, CUserCmd *cmd );
// useful helper funcs...
//int RANDOM_INT(int min, int max);
bool FStrEq(const char *sz1, const char *sz2);
//edict_t* INDEXENT( int iEdictNum );
// get entity index
//int ENTINDEX( edict_t *pEdict );
float VectorDistance(Vector &vec);
int Ceiling ( float fVal );
bool FNullEnt(const edict_t* pent);

// ???
//#define SAFE_STRCPY (to,from,siz)\
//	strncpy(to,from,siz-1);\
//	to[siz-1] = 0;

#endif // __RCBOT2_H__
