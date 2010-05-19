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
#include "bot.h"
#include "bot_mods.h"
#include "bot_globals.h"
#include "bot_fortress.h"
#include "bot_weapons.h"
#include "bot_script.h"

eTFMapType CTeamFortress2Mod :: m_MapType = TF_MAP_CTF;
tf_tele_t CTeamFortress2Mod :: m_Teleporters[MAX_PLAYERS];
int CTeamFortress2Mod :: m_iArea = 0;
float CTeamFortress2Mod::m_fSetupTime = 0.0f;
float CTeamFortress2Mod::m_fRoundTime = 0.0f;
edict_t *CTeamFortress2Mod::m_pFlagCarrierRed = NULL;
edict_t *CTeamFortress2Mod::m_pFlagCarrierBlue = NULL;
float CTeamFortress2Mod::m_fArenaPointOpenTime = 0.0f;
float CTeamFortress2Mod::m_fPointTime = 0.0f;
tf_sentry_t CTeamFortress2Mod::m_SentryGuns[MAX_PLAYERS];	// used to let bots know if sentries have been sapped or not
tf_disp_t  CTeamFortress2Mod::m_Dispensers[MAX_PLAYERS];	// used to let bots know where friendly/enemy dispensers are

edict_t *CTeamFortress2Mod :: getTeleporterExit ( edict_t *pTele )
{
	int i;
	edict_t *pExit;

	for ( i = 0; i < MAX_PLAYERS; i ++ )
	{
		if ( m_Teleporters[i].entrance.get() == pTele )
		{
			if ( (pExit = m_Teleporters[i].exit.get()) != NULL )
			{
				return pExit;
			}

			return false;
		}
	}

	return false;
}

bool CTeamFortress2Mod :: isHealthKit ( edict_t *pEntity )
{
	return strncmp(pEntity->GetClassName(),"item_healthkit",14)==0;
}

void CTeamFortress2Mod:: flagPickedUp (int iTeam, edict_t *pPlayer)
{
	if ( iTeam == TF2_TEAM_BLUE )
		m_pFlagCarrierBlue = pPlayer;
	else if ( iTeam == TF2_TEAM_RED )
		m_pFlagCarrierRed = pPlayer;

	CBotTF2FunctionEnemyAtIntel *function = new CBotTF2FunctionEnemyAtIntel(iTeam,CBotGlobals::entityOrigin(pPlayer),EVENT_FLAG_PICKUP);

	CBots::botFunction(function);

	delete function;
}

bool CTeamFortress2Mod :: isArenaPointOpen ()
{
	return m_fArenaPointOpenTime < engine->Time();
}

void CTeamFortress2Mod :: resetSetupTime ()
{
	m_fRoundTime = engine->Time() + m_fSetupTime;
	m_fArenaPointOpenTime = engine->Time() + m_fPointTime;
}

bool CTeamFortress2Mod::hasRoundStarted ()
{
	return (engine->Time() > m_fRoundTime);
}

void CTeamFortress2Mod :: setPointOpenTime ( int time )
{
	m_fArenaPointOpenTime = 0.0f;
	m_fPointTime = (float)time;
}

void CTeamFortress2Mod :: setSetupTime ( int time )
{
  m_fRoundTime = 0.0f;
  m_fSetupTime = (float)time;
}

bool CTeamFortress2Mod :: isAmmo ( edict_t *pEntity )
{
	return strncmp(pEntity->GetClassName(),"item_ammopack",13)==0;
}

bool CTeamFortress2Mod :: isPayloadBomb ( edict_t *pEntity, int iTeam )
{
	return ((strncmp(pEntity->GetClassName(),"mapobj_cart_dispenser",21)==0) && (CClassInterface::getTeam(pEntity)==iTeam));
}

void CTeamFortress2Mod:: clientCommand ( edict_t *pEntity, int argc, const char *pcmd, const char *arg1, const char *arg2 )
{
	if ( argc > 2 )
	{
		if ( strcmp(pcmd,"voicemenu") == 0 )
		{
			if ( (strcmp(arg1,"0") == 0) && (strcmp(arg2,"0")==0) )
			{
				// somebody shouted "Medic!"
				CBroadcastMedicCall mediccall = CBroadcastMedicCall(pEntity); 

				CBots::botFunction(&mediccall);
			}
		}
	}
}

void CTeamFortress2Mod :: getResetPoints (int iTeam, int *iCurrentDefendArea, int *iCurrentAttackArea)
{
	char *mapname = CBotGlobals::getMapName();

	if ( (strcmp(mapname,"cp_granary") == 0) || (strcmp(mapname,"cp_well") == 0) || (strcmp(mapname,"cp_badlands") == 0) )
	{
		*iCurrentDefendArea = 2;
		*iCurrentAttackArea = 2;
	}
	else if ( strcmp(mapname,"cp_dustbowl") == 0 )
	{
		*iCurrentDefendArea = 0;
		*iCurrentAttackArea = 0;
	}
}

// NOT USED NOW!!! See CPoints class
void CTeamFortress2Mod :: getNextPoints (int iPoint,int iTeam,int iMyTeam,int *iCurrentDefendArea,int *iCurrentAttackArea)
{
	char *mapname = CBotGlobals::getMapName();

	int iAttackPoint;

	if ( (strcmp(mapname,"cp_granary") == 0) || (strcmp(mapname,"cp_well") == 0) || (strcmp(mapname,"cp_badlands") == 0) )
	{
		if ( iTeam == TF2_TEAM_BLUE )
		{
			iAttackPoint = iPoint + 1;

			if ( iMyTeam == TF2_TEAM_BLUE )
			{
				*iCurrentDefendArea = iPoint;
				*iCurrentAttackArea = iAttackPoint;
			}
			else
			{
				*iCurrentDefendArea = iAttackPoint;
				*iCurrentAttackArea = iPoint;
			}
		}
		else
		{
			iAttackPoint = iPoint - 1;

			if ( iMyTeam == TF2_TEAM_BLUE )
			{
				*iCurrentDefendArea = iAttackPoint;
				*iCurrentAttackArea = iPoint;
			}
			else
			{
				*iCurrentDefendArea = iPoint;
				*iCurrentAttackArea = iAttackPoint;
			}
		}
	}
	else if ( strcmp(mapname,"cp_dustbowl") == 0 )
	{
		if ( iMyTeam == TF2_TEAM_BLUE )
		{
			*iCurrentAttackArea = iPoint + 1;
			*iCurrentDefendArea = iPoint + 1;
		}
		else
		{
			*iCurrentAttackArea = iPoint;
			*iCurrentDefendArea = iPoint + 1;
		}
	}
}
// to fix
void CTeamFortress2Mod :: teleporterBuilt ( edict_t *pOwner, eEngiBuild type, edict_t *pBuilding )
{

	int team;

	if ( (type != ENGI_TELE ) ) //(type != ENGI_ENTRANCE) && (type != ENGI_EXIT) )
		return;

	int iIndex = ENTINDEX(pOwner)-1;

	if ( (iIndex < 0) || (iIndex > gpGlobals->maxClients) )
		return;

	team = getTeam(pOwner);

	if ( CTeamFortress2Mod::isTeleporterEntrance(pBuilding,team) )
		m_Teleporters[iIndex].entrance = MyEHandle(pBuilding);
	else if ( CTeamFortress2Mod::isTeleporterExit(pBuilding,team) )
		m_Teleporters[iIndex].exit = MyEHandle(pBuilding);

	m_Teleporters[iIndex].sapper = MyEHandle();


}

void CTeamFortress2Mod::sapperPlaced(edict_t *pOwner,eEngiBuild type,edict_t *pSapper)
{
	int index = ENTINDEX(pOwner)-1;

	if ( (index>=0) && (index<MAX_PLAYERS) )
	{
		if ( type == ENGI_TELE )
			m_Teleporters[index].sapper = MyEHandle(pSapper);
		else if ( type == ENGI_DISP )
			m_Dispensers[index].sapper = MyEHandle(pSapper);
		else if ( type == ENGI_SENTRY )
			m_SentryGuns[index].sapper = MyEHandle(pSapper);
	}
}

void CTeamFortress2Mod::sapperDestroyed(edict_t *pOwner,eEngiBuild type, edict_t *pSapper)
{
	int index; 

	for ( index = 0; index < MAX_PLAYERS; index ++ )
	{
		if ( type == ENGI_TELE )
		{
			if ( m_Teleporters[index].sapper.get_old() == pSapper )
				m_Teleporters[index].sapper = MyEHandle();
			
		}
		else if ( type == ENGI_DISP )
		{
			if ( m_Dispensers[index].sapper.get_old() == pSapper )
				m_Dispensers[index].sapper = MyEHandle();
		}
		else if ( type == ENGI_SENTRY )
		{
			if ( m_SentryGuns[index].sapper.get_old() == pSapper )
				m_SentryGuns[index].sapper = MyEHandle();
		}
	}
}

void CTeamFortress2Mod::sentryBuilt(edict_t *pOwner, eEngiBuild type, edict_t *pBuilding )
{
	int index = ENTINDEX(pOwner)-1;

	if ( (index>=0) && (index<MAX_PLAYERS) )
	{
		if ( type == ENGI_SENTRY )
		{
			m_SentryGuns[index].sentry = MyEHandle(pBuilding);
			m_SentryGuns[index].sapper = MyEHandle();
		}
	}
}

void CTeamFortress2Mod::dispenserBuilt(edict_t *pOwner, eEngiBuild type, edict_t *pBuilding )
{
	int index = ENTINDEX(pOwner)-1;

	if ( (index>=0) && (index<MAX_PLAYERS) )
	{
		if ( type == ENGI_DISP )
		{
			m_Dispensers[index].disp = MyEHandle(pBuilding);
			m_Dispensers[index].sapper = MyEHandle();
		}
	}
}
//
//
//
void CBotMods :: parseFile ()
{
	char buffer[1024];
	unsigned int len;
	char key[64];
	unsigned int i,j;
	char val[256];

	eModId modtype;
	eBotType bottype;
	char steamfolder[256];
	char gamefolder[256];

	CBotGlobals::buildFileName(buffer,BOT_MOD_FILE,BOT_CONFIG_FOLDER,BOT_CONFIG_EXTENSION);

	FILE *fp = CBotGlobals::openFile(buffer,"r");

	CBotMod *curmod = NULL;

	if ( !fp )
	{
		createFile();
		fp = CBotGlobals::openFile(buffer,"r");
	}

	if ( !fp )
	{
		// ERROR!
		return;
	}

	while ( fgets(buffer,1023,fp) != NULL )
	{
		if ( buffer[0] == '#' )
			continue;

		len = strlen(buffer);

		if ( buffer[len-1] == '\n' )
			buffer[--len] = 0;

		i = 0;
		j = 0;

		while ( (i < len) && (buffer[i] != '=') )
		{
			if ( buffer[i] != ' ' )
				key[j++] = buffer[i];
			i++;
		}

		i++;

		key[j] = 0;

		j = 0;

		while ( i < len )
		{
			if ( j || (buffer[i] != ' ') )
				val[j++] = buffer[i];
			i++;
		}

		val[j] = 0;

		if ( !strcmp(key,"mod") )
		{
			if ( curmod )
			{
				curmod->setup(gamefolder,steamfolder,modtype,bottype);
				m_Mods.push_back(curmod);
			}

			curmod = NULL;

			
			bottype = BOTTYPE_GENERIC;

			modtype = MOD_CUSTOM;

			if ( !strcmp("CUSTOM",val) )
			{
				modtype = MOD_CUSTOM;
				curmod = new CBotMod();
			}
			else if ( !strcmp("CSS",val) )
			{
				modtype = MOD_CSS;
				curmod = new CCounterStrikeSourceMod();
			}
			else if ( !strcmp("HL1DM",val) )
			{
				modtype = MOD_HL1DMSRC;
				curmod = new CHLDMSourceMod();
			}
			else if ( !strcmp("HL2DM",val) )
			{
				modtype = MOD_HLDM2;
				curmod = new CHalfLifeDeathmatchMod();
			}
			else if ( !strcmp("FF",val) )
			{
				modtype = MOD_FF;
				curmod = new CFortressForeverMod();
			}
			else if ( !strcmp("TF2",val) )
			{
				modtype = MOD_TF2;
				curmod = new CTeamFortress2Mod();
			}
			else if ( !strcmp("SVENCOOP2",val) )
			{
				modtype = MOD_SVENCOOP2;
				curmod = new CBotMod();
			}
			else if ( !strcmp("TIMCOOP",val) )
			{
				modtype = MOD_TIMCOOP;
				curmod = new CBotMod();
			}
			else if ( !strcmp("NS2",val) )
			{
				modtype = MOD_NS2;
				curmod = new CBotMod();
			}
			else
				curmod = new CBotMod();
		}
		else if ( curmod && !strcmp(key,"bot") )
		{
			if ( !strcmp("GENERIC",val) )
				bottype = BOTTYPE_GENERIC;
			else if ( !strcmp("CSS",val) )
				bottype = BOTTYPE_CSS;
			else if ( !strcmp("HL1DM",val) )
				bottype = BOTTYPE_HL1DM;
			else if ( !strcmp("HL2DM",val) )
				bottype = BOTTYPE_HL2DM;
			else if ( !strcmp("FF",val) )
				bottype = BOTTYPE_FF;
			else if ( !strcmp("TF2",val) )
				bottype = BOTTYPE_TF2;
			else if ( !strcmp("COOP",val) )
				bottype = BOTTYPE_COOP;
			else if ( !strcmp("ZOMBIE",val) )
				bottype = BOTTYPE_ZOMBIE;
		}
		else if ( curmod && !strcmp(key,"steamdir") )
		{
			strncpy(steamfolder,val,255);
		}
		else if ( curmod && !strcmp(key,"gamedir") )
		{
			strncpy(gamefolder,val,255);
		}
	}

	if ( curmod )
	{

		curmod->setup(gamefolder,steamfolder,modtype,bottype);
		m_Mods.push_back(curmod);
	}

	fclose(fp);
}

void CBotMods :: createFile ()
{
	char filename[1024];

	CBotGlobals::buildFileName(filename,BOT_MOD_FILE,BOT_CONFIG_FOLDER,BOT_CONFIG_EXTENSION);

	FILE *fp = CBotGlobals::openFile(filename,"w");

	CBotGlobals::botMessage(NULL,0,"Making a %s.%s file for you... Edit it in '%s'",BOT_MOD_FILE,BOT_CONFIG_EXTENSION,filename);

	if ( fp )
	{
		fprintf(fp,"# EXAMPLE MOD FILE");
		fprintf(fp,"# valid mod types\n");
		fprintf(fp,"# ---------------\n");
		fprintf(fp,"# CSS\n");
		fprintf(fp,"# TF2\n");
		fprintf(fp,"# HL2DM\n");
		fprintf(fp,"# HL1DM\n");
		fprintf(fp,"# FF\n");
		fprintf(fp,"# SVENCOOP2\n");
		fprintf(fp,"# TIMCOOP\n");
		fprintf(fp,"# NS2\n");
		fprintf(fp,"#\n");
		fprintf(fp,"# valid bot types\n");
		fprintf(fp,"# ---------------\n");
		fprintf(fp,"# CSS\n");
		fprintf(fp,"# TF2\n");
		fprintf(fp,"# HL2DM\n");
		fprintf(fp,"# HL1DM\n");
		fprintf(fp,"# FF\n");
		fprintf(fp,"# COOP\n");
		fprintf(fp,"# ZOMBIE\n");
		fprintf(fp,"#\n");
		fprintf(fp,"#\n");
		fprintf(fp,"#mod = CSS\n");
		fprintf(fp,"#steamdir = counter-strike source\n");
		fprintf(fp,"#gamedir = cstrike\n");
		fprintf(fp,"#bot = CSS\n");
		fprintf(fp,"#\n");
		fprintf(fp,"#mod = TF2\n");
		fprintf(fp,"#steamdir = teamfortress 2\n");
		fprintf(fp,"#gamedir = tf\n");
		fprintf(fp,"#bot = TF2\n");
		fprintf(fp,"#\n");
		fprintf(fp,"#mod = FF\n");
		fprintf(fp,"#steamdir = sourcemods\n");
		fprintf(fp,"#gamedir = ff\n");
		fprintf(fp,"#bot = FF\n");
		fprintf(fp,"#\n");
		fprintf(fp,"#mod = HL2DM\n");
		fprintf(fp,"#steamdir = half-life 2 deathmatch\n");
		fprintf(fp,"#gamedir = hl2mp\n");
		fprintf(fp,"#bot = HL2DM\n");
		fprintf(fp,"#\n");
		fprintf(fp,"#mod = HL1DM\n");
		fprintf(fp,"#steamdir = half-life 1 deathmatch\n");
		fprintf(fp,"#gamedir = hl1dm\n");
		fprintf(fp,"#bot = HL1DM\n");
		fprintf(fp,"#\n");
		fprintf(fp,"mod = CUSTOM\n");
		fprintf(fp,"steamdir = day of defeat source\n");
		fprintf(fp,"gamedir = dod\n");
		fprintf(fp,"bot = ZOMBIE\n");
		fprintf(fp,"#\n");

		fclose(fp);
	}
	else
		CBotGlobals::botMessage(NULL,0,"Error! Couldn't create config file %s",filename);
}

void CBotMods :: readMods()
{
	
	m_Mods.push_back(new CCounterStrikeSourceMod());
	m_Mods.push_back(new CHalfLifeDeathmatchMod());

	m_Mods.push_back(new CCounterStrikeSourceModDedicated());
	m_Mods.push_back(new CHalfLifeDeathmatchModDedicated());

	m_Mods.push_back(new CFortressForeverMod());
	m_Mods.push_back(new CFortressForeverModDedicated());

	m_Mods.push_back(new CTeamFortress2Mod());
	m_Mods.push_back(new CTeamFortress2ModDedicated());

	m_Mods.push_back(new CHLDMSourceMod());

	// Look for extra MODs

	parseFile();
}

//////////////////////////////////////////////////////////////////////////////

void CBotMod :: setup ( char *szModFolder, char *szSteamFolder, eModId iModId, eBotType iBotType )
{
	m_szModFolder = CStrings::getString(szModFolder);
	m_szSteamFolder = CStrings::getString(szSteamFolder);
	m_iModId = iModId;
	m_iBotType = iBotType;
}

/*CBot *CBotMod :: makeNewBots ()
{
	return NULL;
}*/

bool CBotMod :: isSteamFolder ( char *szSteamFolder )
{
	return FStrEq(m_szSteamFolder,szSteamFolder);
}

bool CBotMod :: isModFolder ( char *szModFolder )
{
	return FStrEq(m_szModFolder,szModFolder);
}

char *CBotMod :: getSteamFolder ()
{
	return m_szSteamFolder;
}

char *CBotMod :: getModFolder ()
{
	return m_szModFolder;
}

eModId CBotMod :: getModId ()
{
	return m_iModId;
}

//
// MOD LIST

vector<CBotMod*> CBotMods::m_Mods;

void CBotMods :: freeMemory ()
{
	for ( unsigned int i = 0; i < m_Mods.size(); i ++ )
	{
		delete m_Mods[i];
		m_Mods[i] = NULL;
	}

	m_Mods.clear();
}

CBotMod *CBotMods :: getMod ( char *szModFolder, char *szSteamFolder )
{
	for ( unsigned int i = 0; i < m_Mods.size(); i ++ )
	{
		if ( m_Mods[i]->isModFolder(szModFolder) && m_Mods[i]->isSteamFolder(szSteamFolder) )
		{
			return m_Mods[i];
		}
	}

	CBotGlobals::botMessage(NULL,1,"HL2 MODIFICATION \"%s/%s\" NOT FOUND, EXITING... see bot_mods.ini in bot config folder",szSteamFolder,szModFolder);

	return NULL;
}

void CBotMod :: initMod ()
{

}

void CBotMod :: mapInit ()
{

}

void CBotMod :: entitySpawn ( edict_t *pEntity )
{

}

/////////////////////////////////////////////////////////////


// Naris @ AlliedModders .net

bool CTeamFortress2Mod :: TF2_IsPlayerZoomed(edict_t *pPlayer)
{
    int pcond = CClassInterface :: getTF2Conditions(pPlayer);
    return ((pcond & TF2_PLAYER_ZOOMED) == TF2_PLAYER_ZOOMED);
}

bool CTeamFortress2Mod :: TF2_IsPlayerSlowed(edict_t *pPlayer)
{
    int pcond = CClassInterface :: getTF2Conditions(pPlayer);
    return ((pcond & TF2_PLAYER_SLOWED) == TF2_PLAYER_SLOWED);
}

bool CTeamFortress2Mod :: TF2_IsPlayerDisguised(edict_t *pPlayer)
{
    int pcond = CClassInterface :: getTF2Conditions(pPlayer);
    return ((pcond & TF2_PLAYER_DISGUISED) == TF2_PLAYER_DISGUISED);
}

bool CTeamFortress2Mod :: TF2_IsPlayerTaunting ( edict_t *pPlayer )
{
    int pcond = CClassInterface :: getTF2Conditions(pPlayer);
    return ((pcond & TF2_PLAYER_TAUNTING) == TF2_PLAYER_TAUNTING);
}

bool CTeamFortress2Mod :: TF2_IsPlayerCloaked(edict_t *pPlayer)
{
    int pcond = CClassInterface :: getTF2Conditions(pPlayer);
    return ((pcond & TF2_PLAYER_CLOAKED) == TF2_PLAYER_CLOAKED);
}

bool CTeamFortress2Mod :: TF2_IsPlayerInvuln(edict_t *pPlayer)
{
    int pcond = CClassInterface :: getTF2Conditions(pPlayer);
    return ((pcond & TF2_PLAYER_INVULN) == TF2_PLAYER_INVULN);
}

bool CTeamFortress2Mod :: TF2_IsPlayerOnFire(edict_t *pPlayer)
{
    int pcond = CClassInterface :: getTF2Conditions(pPlayer);
    return ((pcond & TF2_PLAYER_ONFIRE) == TF2_PLAYER_ONFIRE);
}

float CTeamFortress2Mod :: TF2_GetClassSpeed(int iClass) 
{ 
switch (iClass) 
{ 
case TF_CLASS_SCOUT: return 133.0f; 
case TF_CLASS_SOLDIER: return 80.0f; 
case TF_CLASS_DEMOMAN: return 93.00; 
case TF_CLASS_MEDIC: return 109.0f; 
case TF_CLASS_PYRO: return 100.0f; 
case TF_CLASS_SPY: return 100.0f; 
case TF_CLASS_ENGINEER: return 100.0f; 
case TF_CLASS_SNIPER: return 100.0f; 
case TF_CLASS_HWGUY: return 77.0f; 
} 
return 0.0; 
} 
 
float CTeamFortress2Mod :: TF2_GetPlayerSpeed(edict_t *pPlayer, TF_Class iClass ) 
{ 
if (TF2_IsPlayerSlowed(pPlayer)) 
return 27.0; 
else 
return TF2_GetClassSpeed(iClass) * 1.58; 
} 




int CTeamFortress2Mod :: getTeam ( edict_t *pEntity )
{
	return CClassInterface::getTeam (pEntity);
	//return *((int*)(pEntity->GetIServerEntity()->GetBaseEntity())+110);
}

int CTeamFortress2Mod :: getSentryLevel ( edict_t *pSentry )
{
	string_t model = pSentry->GetIServerEntity()->GetModelName();
	const char *szmodel = model.ToCStr();

	return (szmodel[24] - '1')+1;
	//if ( pSentry && pSentry->
}

int CTeamFortress2Mod :: getDispenserLevel ( edict_t *pDispenser )
{
	string_t model = pDispenser->GetIServerEntity()->GetModelName();
	const char *szmodel = model.ToCStr();

	if ( strcmp(szmodel,"models/buildables/dispenser_light.mdl") == 0 )
		return 1;

	return (szmodel[31] - '1')+1;
	//if ( pSentry && pSentry->
}

int CTeamFortress2Mod :: getEnemyTeam ( int iTeam )
{
	if ( iTeam == TF2_TEAM_BLUE )
		return TF2_TEAM_RED;
	return TF2_TEAM_BLUE;
}

/*
------------------int : m_nDisguiseTeam
------------------int : m_nDisguiseClass
------------------int : m_iDisguiseTargetIndex
------------------int : m_iDisguiseHealth

*/

bool CTeamFortress2Mod :: isDispenser ( edict_t *pEntity, int iTeam )
{
	return (!iTeam || (iTeam == getTeam(pEntity))) && (strcmp(pEntity->GetClassName(),"obj_dispenser")==0);
}

bool CTeamFortress2Mod :: isFlag ( edict_t *pEntity, int iTeam )
{
	return (!iTeam || (getEnemyTeam(iTeam) == getTeam(pEntity))) && (strcmp(pEntity->GetClassName(),"item_teamflag")==0);
}

bool CTeamFortress2Mod :: isSentry ( edict_t *pEntity, int iTeam )
{
	return (!iTeam || (iTeam == getTeam(pEntity))) && (strcmp(pEntity->GetClassName(),"obj_sentrygun")==0);
}

bool CTeamFortress2Mod :: isTeleporter ( edict_t *pEntity, int iTeam )
{
	return (!iTeam || (iTeam == getTeam(pEntity))) && (strcmp(pEntity->GetClassName(),"obj_teleporter")==0);
}



bool CTeamFortress2Mod :: isTeleporterEntrance ( edict_t *pEntity, int iTeam )
{
	return isTeleporter(pEntity,iTeam) && CClassInterface::isTeleporterMode(pEntity,TELE_ENTRANCE);
	/*if ( isTeleporter(pEntity,iTeam))
	{
		int i;

		for ( i = 0; i < MAX_PLAYERS; i ++ )
		{
			if ( m_Teleporters[i].entrance.get() == pEntity )
			{
				return true;
			}
		}

		return false;
	}*/

	//return false;
	//return (!iTeam || (iTeam == getTeam(pEntity))) && (strcmp(pEntity->GetClassName(),"obj_teleporter_entrance")==0);
}

bool CTeamFortress2Mod :: isTeleporterExit ( edict_t *pEntity, int iTeam )
{
	return isTeleporter(pEntity,iTeam) && CClassInterface::isTeleporterMode(pEntity,TELE_EXIT);
	/*
	if ( isTeleporter(pEntity,iTeam))
	{
		int i;

		for ( i = 0; i < MAX_PLAYERS; i ++ )
		{
			if ( m_Teleporters[i].exit.get() == pEntity )
			{
				return true;
			}
		}

		return false;
	}*/

	//return false;
	//return (!iTeam || (iTeam == getTeam(pEntity))) && (strcmp(pEntity->GetClassName(),"obj_teleporter_exit")==0);
}

bool CTeamFortress2Mod :: isPipeBomb ( edict_t *pEntity, int iTeam)
{
	return (!iTeam || (iTeam == getTeam(pEntity))) && (strcmp(pEntity->GetClassName(),"tf_projectile_pipe_remote")==0);
}

bool CTeamFortress2Mod :: isRocket ( edict_t *pEntity, int iTeam )
{
	return (!iTeam || (iTeam == getTeam(pEntity))) && (strcmp(pEntity->GetClassName(),"tf_projectile_rocket")==0);
}


void CTeamFortress2Mod :: initMod ()
{
	unsigned int i;
	// Setup Weapons

	for ( i = 0; i < TF2_WEAPON_MAX; i ++ )
		CWeapons::addWeapon(new CWeapon(TF2Weaps[i].iSlot,TF2Weaps[i].szWeaponName,TF2Weaps[i].iId,TF2Weaps[i].m_iFlags,TF2Weaps[i].m_iAmmoIndex,TF2Weaps[i].minPrimDist,TF2Weaps[i].maxPrimDist,TF2Weaps[i].m_iPreference));

}

void CTeamFortress2Mod :: mapInit ()
{
	unsigned int i = 0;
	string_t mapname = gpGlobals->mapname;

	const char *szmapname = mapname.ToCStr();

	if ( strncmp(szmapname,"ctf_",4) == 0 )
		m_MapType = TF_MAP_CTF;
	else if ( strncmp(szmapname,"cp_",3) == 0 )
		m_MapType = TF_MAP_CP;
	else if ( strncmp(szmapname,"tc_",3) == 0 )
		m_MapType = TF_MAP_TC;
	else if ( strncmp(szmapname,"pl_",3) == 0 )
		m_MapType = TF_MAP_CART;
	else if ( strncmp(szmapname,"plr_",4) == 0 )
		m_MapType = TF_MAP_CARTRACE;
	else if ( strncmp(szmapname,"arena_",6) == 0 )
		m_MapType = TF_MAP_ARENA;
	else if ( strncmp(szmapname,"koth_",5) == 0 )
		m_MapType = TF_MAP_KOTH;
	else
		m_MapType = TF_MAP_DM;

	m_iArea = 0;

	m_fSetupTime = 0.0f;

	m_fRoundTime = 0.0f;

	m_pFlagCarrierRed = NULL;
	m_pFlagCarrierBlue = NULL;

	for ( i = 0; i < MAX_PLAYERS; i ++ )
	{
		m_Teleporters[i].entrance = MyEHandle(NULL);
		m_Teleporters[i].exit = MyEHandle(NULL);
		m_Teleporters[i].sapper = MyEHandle(NULL);
		m_SentryGuns[i].sapper = MyEHandle(NULL);
		m_SentryGuns[i].sentry = MyEHandle(NULL);
		m_Dispensers[i].sapper = MyEHandle(NULL);
		m_Dispensers[i].disp = MyEHandle(NULL);
	}


	CPoints::loadMapScript();
}

