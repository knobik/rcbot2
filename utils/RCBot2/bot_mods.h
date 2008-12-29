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
#ifndef __BOT_MODS_H__
#define __BOT_MODS_H__

#include "bot_const.h"
#include "bot_strings.h"

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
	BOTTYPE_ZOMBIE
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

	eModId getModId ();

	eBotType getBotType () { return m_iBotType; }

////////////////////////////////
	virtual void initMod ();

	virtual void mapInit ();

	virtual void entitySpawn ( edict_t *pEntity );

private:
	char *m_szModFolder;
	char *m_szSteamFolder;
	eModId m_iModId;
	eBotType m_iBotType;
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

typedef enum
{
	TF_MAP_DM = 0,
	TF_MAP_CTF,
	TF_MAP_CP,
	TF_MAP_TC,
	TF_MAP_CART
}eTFMapType;

class CTeamFortress2Mod : public CBotMod
{
public:
	CTeamFortress2Mod()
	{
		setup("tf","team fortress 2",MOD_TF2,BOTTYPE_TF2);
	}

	void mapInit ();

	static int getTeam ( edict_t *pEntity );

	static int getSentryLevel ( edict_t *pSentry );

	static bool isDispenser ( edict_t *pEntity, int iTeam );

	static bool isSentry ( edict_t *pEntity, int iTeam );

	static bool isTeleporterEntrance ( edict_t *pEntity, int iTeam );

	static bool isTeleporterExit ( edict_t *pEntity, int iTeam );

	static inline bool isMapType ( eTFMapType iMapType ) { return iMapType == m_MapType; }

	static bool isFlag ( edict_t *pEntity, int iTeam );

	static int getEnemyTeam ( int iTeam );

private:
	static eTFMapType m_MapType;
};

class CTeamFortress2ModDedicated : public CTeamFortress2Mod
{
public:
	CTeamFortress2ModDedicated()
	{
		setup("tf","source dedicated server",MOD_TF2,BOTTYPE_TF2);
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

	//void initMod ();

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