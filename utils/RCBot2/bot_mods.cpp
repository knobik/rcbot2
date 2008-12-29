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

eTFMapType CTeamFortress2Mod :: m_MapType = TF_MAP_CTF;
/*
CBot *CCounterStrikeSourceMod :: makeNewBots ()
{
	return new CCSSBot[MAX_PLAYERS];
}

CBot *CHalfLifeDeathmatchMod :: makeNewBots ()
{
	return new CHLDMBot[MAX_PLAYERS];
}

CBot *CFortressForeverMod :: makeNewBots ()
{
	return new CBotFF[MAX_PLAYERS];
}

CBot *CTeamFortress2Mod :: makeNewBots()
{
	return new CBotTF2[MAX_PLAYERS];
}
*/

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

			curmod = new CBotMod();
			bottype = BOTTYPE_GENERIC;

			modtype = MOD_CUSTOM;

			if ( !strcmp("CUSTOM",val) )
				modtype = MOD_CUSTOM;
			else if ( !strcmp("CSS",val) )
				modtype = MOD_CSS;
			else if ( !strcmp("HL1DM",val) )
				modtype = MOD_HL1DMSRC;
			else if ( !strcmp("HL2DM",val) )
				modtype = MOD_HLDM2;
			else if ( !strcmp("FF",val) )
				modtype = MOD_FF;
			else if ( !strcmp("TF2",val) )
				modtype = MOD_TF2;
			else if ( !strcmp("SVENCOOP2",val) )
				modtype = MOD_SVENCOOP2;
			else if ( !strcmp("TIMCOOP",val) )
				modtype = MOD_TIMCOOP;
			else if ( !strcmp("NS2",val) )
				modtype = MOD_NS2;
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
		fprintf(fp,"mod = CSS\n");
		fprintf(fp,"steamdir = counter-strike source\n");
		fprintf(fp,"gamedir = cstrike\n");
		fprintf(fp,"bot = CSS\n");
		fprintf(fp,"#\n");
		fprintf(fp,"mod = TF2\n");
		fprintf(fp,"steamdir = teamfortress 2\n");
		fprintf(fp,"gamedir = tf\n");
		fprintf(fp,"bot = TF2\n");
		fprintf(fp,"#\n");
		fprintf(fp,"mod = FF\n");
		fprintf(fp,"steamdir = sourcemods\n");
		fprintf(fp,"gamedir = ff\n");
		fprintf(fp,"bot = FF\n");
		fprintf(fp,"#\n");
		fprintf(fp,"mod = HL2DM\n");
		fprintf(fp,"steamdir = half-life 2 deathmatch\n");
		fprintf(fp,"gamedir = hl2mp\n");
		fprintf(fp,"bot = HL2DM\n");
		fprintf(fp,"#\n");
		fprintf(fp,"mod = HL1DM\n");
		fprintf(fp,"steamdir = half-life 1 deathmatch\n");
		fprintf(fp,"gamedir = hl1dm\n");
		fprintf(fp,"bot = HL1DM\n");
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
	parseFile();
	/*
	m_Mods.push_back(new CCounterStrikeSourceMod());
	m_Mods.push_back(new CHalfLifeDeathmatchMod());

	m_Mods.push_back(new CCounterStrikeSourceModDedicated());
	m_Mods.push_back(new CHalfLifeDeathmatchModDedicated());

	m_Mods.push_back(new CFortressForeverMod());
	m_Mods.push_back(new CFortressForeverModDedicated());

	m_Mods.push_back(new CTeamFortress2Mod());
	m_Mods.push_back(new CTeamFortress2ModDedicated());

	m_Mods.push_back(new CHLDMSourceMod());*/
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

int CTeamFortress2Mod :: getTeam ( edict_t *pEntity )
{
	return *((int*)(pEntity->GetIServerEntity()->GetBaseEntity())+110);
}

int CTeamFortress2Mod :: getSentryLevel ( edict_t *pSentry )
{
	string_t model = pSentry->GetIServerEntity()->GetModelName();
	const char *szmodel = model.ToCStr();

	return (szmodel[24] - '1')+1;
	//if ( pSentry && pSentry->
}

int CTeamFortress2Mod :: getEnemyTeam ( int iTeam )
{
	if ( iTeam == TF2_TEAM_BLUE )
		return TF2_TEAM_RED;
	return TF2_TEAM_BLUE;
}

bool CTeamFortress2Mod :: isDispenser ( edict_t *pEntity, int iTeam )
{
	return (strcmp(pEntity->GetClassName(),"obj_dispenser")==0) && (!iTeam || (iTeam == getTeam(pEntity)));
}

bool CTeamFortress2Mod :: isSentry ( edict_t *pEntity, int iTeam )
{
	return (strcmp(pEntity->GetClassName(),"obj_sentrygun")==0) && (!iTeam || (iTeam == getTeam(pEntity)));
}

bool CTeamFortress2Mod :: isTeleporterEntrance ( edict_t *pEntity, int iTeam )
{
	return (strcmp(pEntity->GetClassName(),"obj_teleporter_entrance")==0) && (!iTeam || (iTeam == getTeam(pEntity)));
}

bool CTeamFortress2Mod :: isTeleporterExit ( edict_t *pEntity, int iTeam )
{
	return (strcmp(pEntity->GetClassName(),"obj_teleporter_exit")==0) && (!iTeam || (iTeam == getTeam(pEntity)));
}

void CTeamFortress2Mod :: mapInit ()
{
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
	else
		m_MapType = TF_MAP_DM;
}