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
#include "bot_strings.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_genclass.h"
#include "bot_visibles.h"
#include "bot_navigator.h"
#include "bot_kv.h"

vector <CBotProfile*> CBotProfiles :: m_Profiles;
CBotProfile *CBotProfiles :: m_pDefaultProfile = NULL;

CBotProfile :: CBotProfile (const char *szName, const char *szModel,int iTeam,const char *szWeapon, int iVisionTicks, int iPathTicks, int iClass)
{
	m_szName = CStrings::getString(szName);
	m_szModel = CStrings::getString(szModel);
	m_szWeapon = CStrings::getString(szWeapon);
	m_iPathTicks = iPathTicks;
	m_iVisionTicks = iVisionTicks;
	m_iTeam = iTeam;
	m_iClass = iClass;
}

void CBotProfiles :: deleteProfiles ()
{
	for ( unsigned int i = 0; i < m_Profiles.size(); i ++ )
	{
		delete m_Profiles[i];
		m_Profiles[i] = NULL;
	}

	m_Profiles.clear();

	delete m_pDefaultProfile;
	m_pDefaultProfile = NULL;
}

// find profiles and setup list
void CBotProfiles :: setupProfiles ()
{
	unsigned int iId;
	bool bDone;
	char szId[4];
	char filename[512];

	// Setup Default profile
	m_pDefaultProfile = new CBotProfile("RCBot","default",-1,"default",CBotVisibles::DEFAULT_MAX_TICKS,IBotNavigator::MAX_PATH_TICKS);	

	// read profiles
	iId = 1;
	bDone = false;

	while ( (iId < 999) && (!bDone) )
	{
		sprintf(szId,"%d",iId);
		CBotGlobals::buildFileName(filename,szId,BOT_PROFILE_FOLDER,BOT_CONFIG_EXTENSION);

		FILE *fp = CBotGlobals::openFile(filename,"r");

		if ( fp )
		{
			CRCBotKeyValueList *pKVL = new CRCBotKeyValueList();

			CBotGlobals::botMessage(NULL,0,"Reading bot profile \"%s\"",filename);

			int iTeam;
			char *szModel;
			char *szName;
			char *szWeapon;
			int iVisionTicks;
			int iPathTicks;
			int iClass;

			float skill;
			float aim_skill;
			float aim_time;
			float aim_speed;

			pKVL->parseFile(fp);		

			if ( !pKVL->getInt("team",&iTeam) )
				iTeam = m_pDefaultProfile->getTeam();
			if ( !pKVL->getString("model",&szModel) )
				szModel = m_pDefaultProfile->getModel();
			if ( !pKVL->getString("name",&szName) )
				szName = m_pDefaultProfile->getName();
			if ( !pKVL->getString("weapon",&szWeapon) )
				szWeapon = m_pDefaultProfile->getWeapon();
			if ( !pKVL->getInt("visionticks",&iVisionTicks) )
				iVisionTicks = m_pDefaultProfile->getVisionTicks();
			if ( !pKVL->getInt("pathticks",&iPathTicks) )
				iPathTicks = m_pDefaultProfile->getPathTicks();
			if ( !pKVL->getInt("class",&iClass) )
				iClass = 0;
			if ( !pKVL->getFloat("skill",&skill) )
				skill = 0;
			if ( !pKVL->getFloat("aim_speed",&aim_speed) )
				aim_speed = 0;
			if ( !pKVL->getFloat("aim_time",&aim_time) )
				aim_time = 0;
			if ( !pKVL->getFloat("aim_skill",&aim_skill) )
				aim_skill = 0;

			m_Profiles.push_back(new CBotProfile(szName,szModel,iTeam,szWeapon,iVisionTicks,iPathTicks,iClass));

			delete pKVL;

			fclose(fp);
		}
		else
		{
			bDone = true;
			CBotGlobals::botMessage(NULL,0,"Bot profile \"%s\" not found",filename);
		}

		iId ++;
	}

}

CBotProfile *CBotProfiles :: getDefaultProfile ()
{
	if ( m_pDefaultProfile == NULL )
		CBotGlobals::botMessage(NULL,1,"Error, default profile is NULL (Caused by memory problem, bad initialisation or overwrite) Exiting..");

	return m_pDefaultProfile;
}

// return a profile unused by a bot
CBotProfile *CBotProfiles :: getRandomFreeProfile ()
{
	unsigned int i;
	dataUnconstArray<int> iList;
	CBotProfile *found = NULL;

	for ( i = 0; i < m_Profiles.size(); i ++ )
	{
		if ( !CBots::findBotByProfile(m_Profiles[i]) )
			iList.Add(i);
	}

	if ( iList.IsEmpty() )
		return NULL;
	
	found = m_Profiles[iList.Random()];
	iList.Clear();

	return found;
}

	

