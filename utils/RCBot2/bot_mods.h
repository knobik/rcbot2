/*
 *    part of https://rcbot2.svn.sourceforge.net/svnroot/rcbot2
 *
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
#ifndef __BOT_MODS_H__
#define __BOT_MODS_H__

#include "bot_const.h"
#include "bot_strings.h"
#include "bot_fortress.h"
#include "bot_dod_bot.h"

#define MAX_CAP_POINTS 32

#define DOD_MAPTYPE_UNKNOWN 0 
#define DOD_MAPTYPE_FLAG 1
#define DOD_MAPTYPE_BOMB 2

class CBotNeuralNet;

#include <vector>
using namespace std;


/*
		CSS
		TF2
		HL2DM
		HL1DM
		FF
		COOP
		ZOMBIE
*/
typedef enum
{
	BOTTYPE_GENERIC = 0,
	BOTTYPE_CSS,
	BOTTYPE_TF2,
	BOTTYPE_HL2DM,
	BOTTYPE_HL1DM,
	BOTTYPE_FF,
	BOTTYPE_COOP,
	BOTTYPE_ZOMBIE,
	BOTTYPE_DOD,
	BOTTYPE_MAX
}eBotType;

class CBotMod
{
public:
	CBotMod() 
	{
		m_szModFolder = NULL;
		m_szSteamFolder = NULL;
		m_iModId = MOD_UNSUPPORTED;
		m_iBotType = BOTTYPE_GENERIC;
	}

	void setup ( char *szModFolder, char *szSteamFolder, eModId iModId, eBotType iBotType );

	bool isSteamFolder ( char *szSteamFolder );

	bool isModFolder ( char *szModFolder );

	char *getSteamFolder ();

	char *getModFolder ();

	virtual const char *getPlayerClass ()
	{
		return "CBasePlayer";
	}

	eModId getModId ();

	eBotType getBotType () { return m_iBotType; }

	virtual void addWaypointFlags (edict_t *pEdict, int *iFlags, int *iArea, float *fMaxDistance ){ return; }

////////////////////////////////
	virtual void initMod ();

	virtual void mapInit ();

	virtual void entitySpawn ( edict_t *pEntity );

	virtual void clientCommand ( edict_t *pEntity, int argc,const char *pcmd, const char *arg1, const char *arg2 ) {};

	virtual void modFrame () { };

	virtual void freeMemory() {};
private:
	char *m_szModFolder;
	char *m_szSteamFolder;
	eModId m_iModId;
	eBotType m_iBotType;
};

///////////////////
/*
class CDODFlag
{
public:
	CDODFlag()
	{
		m_pEdict = NULL;
		m_iId = -1;
	}

	void setup (edict_t *pEdict, int id)
	{
		m_pEdict = pEdict;
		m_iId = id;
	}

	inline bool isFlag ( edict_t *pEdict ) { return m_pEdict == pEdict; }

	void update ();

private:
	edict_t *m_pEdict;
	int m_iId;
};
*/
#define MAX_DOD_FLAGS 8

class CDODFlags
{
public:
	CDODFlags()
	{
		init();
	}

	void init ()
	{
		m_iNumControlPoints = 0;
		m_vCPPositions = NULL;

		m_iAlliesReqCappers = NULL;
		m_iAxisReqCappers = NULL;
		m_iNumAllies = NULL;
		m_iNumAxis = NULL;
		m_iOwner = NULL;
		m_bBombPlanted_Unreliable = NULL;
		m_iBombsRequired = NULL;
		m_iBombsRemaining = NULL;
		m_bBombBeingDefused = NULL;
		m_iNumAxisBombsOnMap = 0;
		m_iNumAlliesBombsOnMap = 0;
		memset(m_bBombPlanted,0,sizeof(bool)*MAX_DOD_FLAGS);
		memset(m_pFlags,0,sizeof(edict_t*)*MAX_DOD_FLAGS);
		memset(m_pBombs,0,sizeof(edict_t*)*MAX_DOD_FLAGS);
	}

	int getNumFlags () { return m_iNumControlPoints; }
	int getNumFlagsOwned (int iTeam)
	{
		int count = 0;

		for ( short i = 0; i < m_iNumControlPoints; i ++ )
		{
			if ( m_iOwner[i] == iTeam )
				count++;
		}

		return count;
	}

	void setup (edict_t *pResourceEntity, int iMapType);

	bool getRandomEnemyControlledFlag ( Vector *position, int iTeam, int *id = NULL );
	bool getRandomTeamControlledFlag ( Vector *position, int iTeam, int *id = NULL );

	bool getRandomBombToDefuse ( Vector *position, int iTeam, edict_t **pBombTarget, int *id = NULL );
	bool getRandomBombToPlant ( Vector *position, int iTeam, edict_t **pBombTarget, int *id = NULL );
	bool getRandomBombToDefend ( Vector *position, int iTeam, edict_t **pBombTarget, int *id = NULL );

	inline int getNumBombsToDefend ( int iTeam )
	{
		int count = 0;

		for ( short int i = 0; i < m_iNumControlPoints; i ++ )
		{
			if ( canDefendBomb(iTeam,i) )
				count++;
		}

		return count;
	}

	inline int getNumBombsToDefuse ( int iTeam )
	{
		int count = 0;

		for ( short int i = 0; i < m_iNumControlPoints; i ++ )
		{
			if ( canDefuseBomb(iTeam,i) )
				count++;
		}

		return count;
	}

	inline int getNumPlantableBombs (int iTeam)
	{
		int count = 0;

		for ( short int i = 0; i < m_iNumControlPoints; i ++ )
		{
			if ( canPlantBomb(iTeam,i) )
				count += getNumBombsRequired(i);
		}

		return count;
	}

	inline float isBombExplodeImminent ( int id )
	{
		return (engine->Time() - m_fBombPlantedTime[id]) > DOD_BOMB_EXPLODE_IMMINENT_TIME;
	}

	inline void setBombPlanted ( int id, bool val )
	{
		m_bBombPlanted[id] = val;

		if ( val )
			m_fBombPlantedTime[id] = engine->Time();
		else
			m_fBombPlantedTime[id] = 0;
	}

	inline int getNumBombsToPlant ( int iTeam)
	{
		int count = 0;

		for ( short int i = 0; i < m_iNumControlPoints; i ++ )
		{
			if ( canPlantBomb(iTeam,i) )
				count += getNumBombsRemaining(i);
		}

		return count;
	}

	inline bool ownsFlag ( edict_t *pFlag, int iTeam ) { return ownsFlag(getFlagID(pFlag),iTeam); }
	inline bool ownsFlag ( int iFlag, int iTeam )
	{
		if ( iFlag == -1 )
			return false;

		return m_iOwner[iFlag] == iTeam;
	}

	inline int numFlagsOwned (int iTeam)
	{
		int count = 0;

		for ( short int i = 0; i < m_iNumControlPoints; i ++ )
		{
			if ( m_iOwner[i] == iTeam )
				count++;
		}

		return count;
	}

	inline int numCappersRequired ( edict_t *pFlag, int iTeam ) { return numCappersRequired(getFlagID(pFlag),iTeam); }
	inline int numCappersRequired ( int iFlag, int iTeam )
	{
		if ( iFlag == -1 )
			return 0;

		return (iTeam == TEAM_ALLIES) ? (m_iAlliesReqCappers[iFlag]) : (m_iAxisReqCappers[iFlag]);
	}

	inline bool isBombPlanted ( int iId )
	{
		if ( iId == -1 )
			return false;

		return m_bBombPlanted[iId];
	}

	inline bool isBombPlanted ( edict_t *pBomb )
	{
		return isBombPlanted(getBombID(pBomb));
	}

	inline bool canDefendBomb ( int iTeam, int iId )
	{
		return ((m_pBombs[iId]!=NULL)&&(m_iOwner[iId]!=iTeam) && isBombPlanted(iId));
	}

	inline bool canDefuseBomb ( int iTeam, int iId )
	{
		return ((m_pBombs[iId]!=NULL)&&(m_iOwner[iId]==iTeam) && isBombPlanted(iId));
	}

	inline bool canPlantBomb ( int iTeam, int iId )
	{
		return ((m_pBombs[iId]!=NULL)&&(m_iOwner[iId]!=iTeam) && !isBombPlanted(iId));
	}

	bool isTeamMateDefusing ( edict_t *pIgnore, int iTeam, int id );
	bool isTeamMatePlanting ( edict_t *pIgnore, int iTeam, int id );

	inline int getNumBombsRequired ( int iId )
	{
		if ( iId == -1 )
			return false;

		return m_iBombsRequired[iId];
	}

	inline int getNumBombsRequired ( edict_t *pBomb )
	{
		return getNumBombsRequired(getBombID(pBomb));
	}

	inline int getNumBombsRemaining ( int iId )
	{
		if ( iId == -1 )
			return false;

		return m_iBombsRemaining[iId];
	}

	inline int getNumBombsRemaining ( edict_t *pBomb )
	{
		return getNumBombsRemaining(getBombID(pBomb));
	}

	inline bool isBombBeingDefused ( int iId )
	{
		if ( iId == -1 )
			return false;

		return m_bBombBeingDefused[iId];
	}

	inline bool isBombBeingDefused ( edict_t *pBomb )
	{
		return isBombBeingDefused(getBombID(pBomb));
	}

	inline int numEnemiesAtCap ( edict_t *pFlag, int iTeam ) { return numEnemiesAtCap(getFlagID(pFlag),iTeam); }

	inline int numFriendliesAtCap ( edict_t *pFlag, int iTeam ) { return numFriendliesAtCap(getFlagID(pFlag),iTeam); }

	inline int numFriendliesAtCap ( int iFlag, int iTeam )
	{
		if ( iFlag == -1 )
			return 0;

		return (iTeam == TEAM_ALLIES) ? (m_iNumAllies[iFlag]) : (m_iNumAxis[iFlag]);
	}

	inline int numEnemiesAtCap ( int iFlag, int iTeam )
	{
		if ( iFlag == -1 )
			return 0;

		return (iTeam == TEAM_ALLIES) ? (m_iNumAxis[iFlag]) : (m_iNumAllies[iFlag]);
	}

	inline int getFlagID ( edict_t *pent )
	{
		for ( short int i = 0; i < m_iNumControlPoints; i ++ )
		{
			if ( m_pFlags[i] == pent )
				return i;
		}

		return -1;
	}

	inline int getBombID ( edict_t *pent )
	{
		for ( short int i = 0; i < m_iNumControlPoints; i ++ )
		{
			if ( m_pBombs[i] == pent )
				return i;
		}

		return -1;
	}

	inline bool isFlag ( edict_t *pent )
	{
		return getFlagID(pent) != -1;
	}

	inline bool isBomb ( edict_t *pent )
	{
		return getBombID(pent) != -1;
	}

	inline int getNumBombsOnMap ( int iTeam )
	{
		if ( iTeam == TEAM_ALLIES )
			return m_iNumAlliesBombsOnMap;
		return m_iNumAxisBombsOnMap;
	}

private:
	edict_t *m_pFlags[MAX_DOD_FLAGS];
	edict_t *m_pBombs[MAX_DOD_FLAGS];

	int m_iNumControlPoints;
	Vector *m_vCPPositions;

	int *m_iAlliesReqCappers;
	int *m_iAxisReqCappers;
	int *m_iNumAllies;
	int *m_iNumAxis;
	int *m_iOwner;

	// reply on this one
	bool *m_bBombPlanted_Unreliable;
    bool m_bBombPlanted[MAX_DOD_FLAGS];
	float m_fBombPlantedTime[MAX_DOD_FLAGS];
	int *m_iBombsRequired;
	int *m_iBombsRemaining;
	bool *m_bBombBeingDefused;
	int m_iNumAlliesBombsOnMap;
	int m_iNumAxisBombsOnMap;
};

class CDODMod : public CBotMod
{
public:
	CDODMod()
	{
		setup("dod","day of defeat source",MOD_DOD,BOTTYPE_DOD);
	}

	static void roundStart ();
	
	static int numClassOnTeam( int iTeam, int iClass );

	static int getScore(edict_t *pPlayer);

	static int getHighestScore ();

	void clientCommand ( edict_t *pEntity, int argc, const char *pcmd, const char *arg1, const char *arg2 );

	static float getMapStartTime ();

	inline static bool isBombMap () { return m_iMapType == DOD_MAPTYPE_BOMB; }
	inline static bool isFlagMap () { return m_iMapType == DOD_MAPTYPE_FLAG; }

	inline static bool isCommunalBombPoint () { return m_bCommunalBombPoint; }
	inline static int getBombPointArea (int iTeam) { if ( iTeam == TEAM_ALLIES ) return m_iBombAreaAllies; return m_iBombAreaAxis; } 

	void addWaypointFlags (edict_t *pEdict, int *iFlags, int *iArea, float *fMaxDistance );

	static CDODFlags m_Flags;

	static bool shouldAttack ( int iTeam ); // uses the neural net to return probability of attack

protected:

	void initMod ();

	void mapInit ();

	void modFrame ();

	void freeMemory ();

	static edict_t *m_pResourceEntity;
	static edict_t *m_pPlayerResourceEntity;
	static edict_t *m_pGameRules;
	static float m_fMapStartTime;
	static int m_iMapType;
	static bool m_bCommunalBombPoint; // only one bomb suuply point for both teams
	static int m_iBombAreaAllies;
	static int m_iBombAreaAxis;

	static CPerceptron *gNetAttackOrDefend;

	//virtual void entitySpawn ( edict_t *pEntity );

	//virtual void clientCommand ( edict_t *pEntity, int argc,const char *pcmd, const char *arg1, const char *arg2 ) {};

};

class CDODModDedicated : public CDODMod
{
public:
	CDODModDedicated()
	{
		setup("dod","source dedicated server",MOD_DOD,BOTTYPE_DOD);
	}
protected:

};

class CCounterStrikeSourceMod : public CBotMod
{
public:
	CCounterStrikeSourceMod()
	{
		setup("cstrike","counter-strike source",MOD_CSS,BOTTYPE_CSS);
	}

	//void initMod ();

	//void mapInit ();

	//void entitySpawn ( edict_t *pEntity );
protected:
	// storing mod specific info
	vector<edict_t*> m_pHostages;
	vector<edict_t*> m_pBombPoints;
	vector<edict_t*> m_pRescuePoints;
};


class CCounterStrikeSourceModDedicated : public CCounterStrikeSourceMod
{
public:
	CCounterStrikeSourceModDedicated()
	{
		setup("cstrike","source dedicated server",MOD_CSS,BOTTYPE_CSS);
	}

	//void initMod ();

	//void mapInit ();

	//void entitySpawn ( edict_t *pEntity );
};

class CTimCoopMod : public CBotMod
{
public:
	CTimCoopMod()
	{
		setup("SourceMods","timcoop",MOD_TIMCOOP,BOTTYPE_COOP);
	}

	//void initMod ();

	//void mapInit ();

	//void entitySpawn ( edict_t *pEntity );
};

class CSvenCoop2Mod : public CBotMod
{
public:
	CSvenCoop2Mod()
	{
		setup("SourceMods","svencoop2",MOD_SVENCOOP2,BOTTYPE_COOP);
	}

	//void initMod ();

	//void mapInit ();

	//void entitySpawn ( edict_t *pEntity );
};

class CFortressForeverMod : public CBotMod
{
public:
	CFortressForeverMod()
	{
		setup("FortressForever","SourceMods",MOD_FF,BOTTYPE_FF);
	}
private:

};

class CFortressForeverModDedicated : public CBotMod
{
public:
	CFortressForeverModDedicated()
	{
		setup("FortressForever","source dedicated server",MOD_FF,BOTTYPE_FF);
	}
private:

};

class CHLDMSourceMod : public CBotMod
{
public:
	CHLDMSourceMod()
	{
		setup("hl1mp","half-life deathmatch source",MOD_HL1DMSRC,BOTTYPE_HL1DM);
	}
};

class CSynergyMod : public CBotMod
{
public:
	CSynergyMod()
	{
		setup("synergy","synergy",MOD_SYNERGY,BOTTYPE_COOP);
	}

	//void initMod ();

	//void mapInit ();

	//void entitySpawn ( edict_t *pEntity );
};

typedef enum
{
	TF_MAP_DM = 0,
	TF_MAP_CTF,
	TF_MAP_CP,
	TF_MAP_TC,
	TF_MAP_CART,
	TF_MAP_CARTRACE,
	TF_MAP_ARENA,
	TF_MAP_KOTH,
	TF_MAP_SD, // special delivery : added 15 jul 12
	TF_MAP_TR,
	TF_MAP_MVM
}eTFMapType;

// These must be MyEHandles because they may be destroyed at any time
typedef struct
{
	MyEHandle entrance;
	MyEHandle exit;
	MyEHandle sapper;
//	short builder;
}tf_tele_t;

typedef struct
{
	MyEHandle sentry;
	MyEHandle sapper;
//	short builder;
}tf_sentry_t;

typedef struct
{
	MyEHandle disp;
	MyEHandle sapper;
//	short builder;
}tf_disp_t;

class CTeamFortress2Mod : public CBotMod
{
public:
	CTeamFortress2Mod()
	{
		setup("tf","team fortress 2",MOD_TF2,BOTTYPE_TF2);

		m_pResourceEntity = NULL;
	}

	void mapInit ();

	void clientCommand ( edict_t *pEntity, int argc, const char *pcmd, const char *arg1, const char *arg2 );

	virtual const char *getPlayerClass ()
	{
		return "CTFPlayer";
	}

	void initMod ();

	static int getTeam ( edict_t *pEntity );

	static int getSentryLevel ( edict_t *pSentry );
	static int getDispenserLevel ( edict_t *pDispenser );

	static bool isDispenser ( edict_t *pEntity, int iTeam );

	static bool isPayloadBomb ( edict_t *pEntity, int iTeam );

	static bool isHealthKit ( edict_t *pEntity );

	static bool isAmmo ( edict_t *pEntity );

	static int getArea (); // get current area of map

	static void setArea ( int area ) { m_iArea = area; }

	static bool isSentry ( edict_t *pEntity, int iTeam );

	static bool isTeleporter ( edict_t *pEntity, int iTeam );

	static bool isTeleporterEntrance ( edict_t *pEntity, int iTeam );

	static bool isTeleporterExit ( edict_t *pEntity, int iTeam );

	static inline bool isMapType ( eTFMapType iMapType ) { return iMapType == m_MapType; }

	static bool isFlag ( edict_t *pEntity, int iTeam );

	static bool isPipeBomb ( edict_t *pEntity, int iTeam);

	static bool isRocket ( edict_t *pEntity, int iTeam );

	static int getEnemyTeam ( int iTeam );

// Naris @ AlliedModders .net

	static bool TF2_IsPlayerZoomed(edict_t *pPlayer);

	static bool TF2_IsPlayerSlowed(edict_t *pPlayer);

	static bool TF2_IsPlayerDisguised(edict_t *pPlayer);

	static bool TF2_IsPlayerCloaked(edict_t *pPlayer);

	static bool TF2_IsPlayerInvuln(edict_t *pPlayer);

	static bool TF2_IsPlayerOnFire(edict_t *pPlayer);

	static bool TF2_IsPlayerTaunting(edict_t *pPlayer);

	static float TF2_GetPlayerSpeed(edict_t *pPlayer, TF_Class iClass );

	static void teleporterBuilt ( edict_t *pOwner, eEngiBuild type, edict_t *pBuilding );

	static edict_t *getTeleporterExit ( edict_t *pTele );

	static void setPointOpenTime ( int time );

	static void setSetupTime ( int time );

	static void resetSetupTime ();

	static bool isArenaPointOpen ();

	static bool hasRoundStarted ();

	static int getHighestScore ();

	static edict_t *CTeamFortress2Mod :: nearestDispenser ( Vector vOrigin, int team );

	static void flagPickedUp (int iTeam, edict_t *pPlayer);
	static void flagReturned (int iTeam);

	static void setAttackDefendMap ( bool bSet ) { m_bAttackDefendMap = bSet; }
	static bool isAttackDefendMap () { return m_bAttackDefendMap; }

	void addWaypointFlags (edict_t *pEdict, int *iFlags, int *iArea, float *fMaxDistance );

	static void flagDropped (int iTeam)
	{
		if ( iTeam == TF2_TEAM_BLUE )
			m_pFlagCarrierBlue = NULL;
		else if ( iTeam == TF2_TEAM_RED )
			m_pFlagCarrierRed = NULL;

		m_iFlagCarrierTeam = iTeam;
	}

	static void roundStarted ()
	{
		m_bHasRoundStarted = true;
	}

	static void roundReset ()
	{
		m_bHasRoundStarted = false;
		m_iFlagCarrierTeam = 0;
	}

	static bool isFlagCarrier (edict_t *pPlayer)
	{
		return (m_pFlagCarrierBlue==pPlayer)||(m_pFlagCarrierRed==pPlayer);
	}

	static bool isFlagCarried (int iTeam)
	{
		if ( iTeam == TF2_TEAM_BLUE )
			return (m_pFlagCarrierBlue != NULL);
		else if ( iTeam == TF2_TEAM_RED )
			return (m_pFlagCarrierRed != NULL);

		return false;
	}

	static void sapperPlaced(edict_t *pOwner,eEngiBuild type,edict_t *pSapper);
	static void sapperDestroyed(edict_t *pOwner,eEngiBuild type,edict_t *pSapper);
	static void sentryBuilt(edict_t *pOwner, eEngiBuild type, edict_t *pBuilding);
	static void dispenserBuilt(edict_t *pOwner, eEngiBuild type, edict_t *pBuilding);

	static bool isMySentrySapped ( edict_t *pOwner ) 
	{
		int id = ENTINDEX(pOwner)-1;

		if ( id>=0 )
		{
			return (m_SentryGuns[id].sentry.get()!=NULL)&&(m_SentryGuns[id].sapper.get()!=NULL);
		}

		return false;
	}

	static edict_t *getSentryGun ( int id )
	{
		return m_SentryGuns[id].sentry.get();
	}

	static bool isMyTeleporterSapped ( edict_t *pOwner )
	{
		int id = ENTINDEX(pOwner)-1;

		if ( id>=0 )
		{
			return ((m_Teleporters[id].exit.get()!=NULL)||(m_Teleporters[id].entrance.get()!=NULL))&&(m_Teleporters[id].sapper.get()!=NULL);
		}

		return false;
	}

	static bool isMyDispenserSapped ( edict_t *pOwner )
	{
		int id = ENTINDEX(pOwner)-1;

		if ( id>=0 )
		{
			return (m_Dispensers[id].disp.get()!=NULL)&&(m_Dispensers[id].sapper.get()!=NULL);
		}

		return false;
	}

	static bool isSentrySapped ( edict_t *pSentry )
	{
		unsigned int i;

		for ( i = 0; i < MAX_PLAYERS; i ++ )
		{
			if ( m_SentryGuns[i].sentry.get() == pSentry )
				return m_SentryGuns[i].sapper.get()!=NULL;
		}

		return false;
	}

	static bool isTeleporterSapped ( edict_t *pTele )
	{
		unsigned int i;

		for ( i = 0; i < MAX_PLAYERS; i ++ )
		{
			if ( (m_Teleporters[i].entrance.get() == pTele) || (m_Teleporters[i].exit.get() == pTele) )
				return m_Teleporters[i].sapper.get()!=NULL;
		}

		return false;
	}

	static bool isDispenserSapped ( edict_t *pDisp )
	{
		unsigned int i;

		for ( i = 0; i < MAX_PLAYERS; i ++ )
		{
			if ( m_Dispensers[i].disp.get() == pDisp )
				return m_Dispensers[i].sapper.get()!=NULL;
		}

		return false;
	}

	static edict_t *findResourceEntity ();

	static void addCapper ( int cp, int capper )
	{
		if ( capper && (cp < MAX_CAP_POINTS) )
			m_Cappers[cp] |= (1<<(capper-1));
	}

	static void removeCappers ( int cp )
	{
		m_Cappers[cp] = 0;
	}

	static bool isCapping ( edict_t *pPlayer )
	{
		int index = ENTINDEX(pPlayer)-1;

		if ( index >= 0 )
		{
			int i = 0;

			for ( i = 0; i < MAX_CAP_POINTS; i ++ )
			{
				if ( (m_Cappers[i] & (1<<index)) )
					return true;
			}
		}

		return false;
	}

	static void resetCappers ()
	{
		int i = 0;

		for ( i = 0; i < MAX_CAP_POINTS; i ++ )
		{
			m_Cappers[i] = 0;
		}
	}

	static int numPlayersOnTeam ( int iTeam, bool bAliveOnly = false );
	static int numClassOnTeam ( int iTeam, int iClass );

	static int getFlagCarrierTeam () { return m_iFlagCarrierTeam; }
	static bool canTeamPickupFlag_SD(int iTeam,bool bGetUnknown);

	static edict_t *getBuildingOwner (eEngiBuild object, short index);

	static bool isBoss ( edict_t *pEntity );

	static void initBoss ( bool bSummoned ) { m_bBossSummoned = bSummoned; m_pBoss = NULL; }

	static bool isBossSummoned () { return m_bBossSummoned; }

	static bool isSentryGun ( edict_t *pEdict );

	static edict_t *getMediGun ( edict_t *pPlayer );

	static void findMediGun ( edict_t *pPlayer );

private:

	static float TF2_GetClassSpeed(int iClass);

	static eTFMapType m_MapType;	

	static tf_tele_t m_Teleporters[MAX_PLAYERS];	// used to let bots know who made a teleport ans where it goes
	static tf_sentry_t m_SentryGuns[MAX_PLAYERS];	// used to let bots know if sentries have been sapped or not
	static tf_disp_t  m_Dispensers[MAX_PLAYERS];	// used to let bots know where friendly/enemy dispensers are

	static int m_iArea;

	static float m_fSetupTime;

	static float m_fRoundTime;

	static MyEHandle m_pFlagCarrierRed;
	static MyEHandle m_pFlagCarrierBlue;

	static float m_fPointTime;
	static float m_fArenaPointOpenTime;

	static MyEHandle m_pResourceEntity;
	static bool m_bAttackDefendMap;

	static int m_Cappers[MAX_CAP_POINTS];

	static bool m_bHasRoundStarted;

	static int m_iFlagCarrierTeam;
	static MyEHandle m_pBoss;
	static bool m_bBossSummoned;

	static MyEHandle pMediGuns[MAX_PLAYERS];

};

class CTeamFortress2ModDedicated : public CTeamFortress2Mod
{
public:
	CTeamFortress2ModDedicated()
	{
#ifdef __linux__
        setup("tf","orangebox",MOD_TF2,BOTTYPE_TF2);    //bir3yk
#else
        setup("tf","source dedicated server",MOD_TF2,BOTTYPE_TF2);
#endif
	}

private:

};

class CHalfLifeDeathmatchMod : public CBotMod
{
public:
	CHalfLifeDeathmatchMod()
	{
		setup("hl2mp","half-life 2 deathmatch",MOD_HLDM2,BOTTYPE_HL2DM);
	}



	void initMod ();

	//void mapInit ();

	//void entitySpawn ( edict_t *pEntity );
private:

};

class CHalfLifeDeathmatchModDedicated : public CHalfLifeDeathmatchMod
{
public:
	CHalfLifeDeathmatchModDedicated()
	{
		setup("hl2mp","source dedicated server",MOD_HLDM2,BOTTYPE_HL2DM);
	}

	//void initMod ();

	//void mapInit ();

	//void entitySpawn ( edict_t *pEntity );
protected:

};

class CBotMods
{
public:

	static void parseFile ();

	static void createFile ();

	static void readMods();

	static void freeMemory ();

	static CBotMod *getMod ( char *szModFolder, char *szSteamFolder );

private:
	static vector<CBotMod*> m_Mods;
};

#endif