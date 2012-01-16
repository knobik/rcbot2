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
 *///=================================================================================//

/// NOT WORKING

#include <stdio.h>
#include <time.h>

//#include "cbase.h"
//#include "baseentity.h"
#include "filesystem.h"
#include "interface.h"
#include "engine/iserverplugin.h"
#include "iplayerinfo.h"
#ifdef __linux__
#include "shake.h"    //bir3yk
#endif
#include "IEngineTrace.h" // for traceline functions
#include "tier2/tier2.h"
#include "IEffects.h"
#include "vplane.h"
#include "eiface.h"
#include "igameevents.h"
#include "icvar.h"
//#include "iconvar.h"
#include "convar.h"
#include "Color.h"
#include "ndebugoverlay.h"
#include "server_class.h"
#include "time.h"
#include "bot.h"
#include "bot_commands.h"
#include "bot_client.h"
#include "bot_globals.h"
#include "bot_accessclient.h"
#include "bot_waypoint_visibility.h" // for initializing table
#include "bot_event.h"
#include "bot_profile.h"
#include "bot_weapons.h"
#include "bot_mods.h"
#include "bot_profiling.h"
#include "vstdlib/random.h" // for random  seed 
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#include "bot_wpt_dist.h"

#include "bot_configfile.h"
#include "bot_strings.h"

vector <char *> CBotConfigFile::m_Commands;
unsigned int CBotConfigFile::m_iCmd = 0; // current command (time delayed)
float CBotConfigFile::m_fNextCommandTime = 0.0f;

void CBotConfigFile :: load ()
{
	char filename[512];
	char line[256];
	//int len;
	m_Commands.clear();

	CBotGlobals::buildFileName(filename,"config",BOT_CONFIG_FOLDER,BOT_CONFIG_EXTENSION);

	FILE *fp = CBotGlobals::openFile(filename,"r");

	if ( !fp )
	{
		CBotGlobals::botMessage(NULL,0,"config file not found");
		return;
	}

	while ( fgets(line,255,fp) != NULL )
	{
		if ( line[0] == '#' )
			continue;

		//len = strlen(line);

		//if ( line[len-1] == '\n' || ( line[len-1] == '\r' ))
		//	line[--len] = 0;

		m_Commands.push_back(CStrings::getString(line));
	}

	fclose(fp);

}

void CBotConfigFile :: doNextCommand ()
{
	if ( (m_fNextCommandTime < engine->Time()) && (m_iCmd < m_Commands.size()) )
	{
		engine->ServerCommand(m_Commands[m_iCmd]);
		//ConCommandBase::
		/*
		if ( m_Commands[m_iCmd][0] == '@' )
		{
			CBotGlobals::m_pCommands
		}
		else if (m_Commands[m_iCmd][0] == '$' )
		{

		}
*/
		CBotGlobals::botMessage(NULL,0,"Bot Command '%s' executed",m_Commands[m_iCmd]);
		m_iCmd ++;
		m_fNextCommandTime = engine->Time() + 0.1f;
	}
}

void CRCBotTF2UtilFile :: loadConfig()
{
	 unsigned short int iFile = 0;
	 char szFullFilename[512];
	 char szFilename[64];
	 char line[256];
	 FILE *fp;

	 for ( iFile = 0; iFile < UTIL_TYPE_MAX; iFile ++ )
	 {
		 if ( iFile == 0 )
			sprintf(szFilename,"attack_util.csv");
		 else
			sprintf(szFilename,"normal_util.csv");

		CBotGlobals::buildFileName(szFullFilename,szFilename);
		fp = CBotGlobals::openFile(szFullFilename,"r");

		if ( !fp )
		{
			while ( fgets(line,255,fp) != NULL )
			{
				if ( line[0] == '#' )
					continue;
				if ( line[0] == '\n' )
					continue;
				if ( line[0] == '\r' )
					continue;
				if ( line[0] == '/' )
					continue;

			}
		}
	 }

}
