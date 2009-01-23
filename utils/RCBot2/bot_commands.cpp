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
#include "bot_client.h"
#include "bot_strings.h"
#include "bot_commands.h"
#include "bot_globals.h"
#include "bot_accessclient.h"

#include "bot_waypoint.h" // for waypoint commands
#include "bot_waypoint_locations.h" // for waypoint commands

CBotCommandContainer *CBotGlobals :: m_pCommands = new CRCBotCommand();

///////////////////////////////////////////////////
// Setup commands
CRCBotCommand :: CRCBotCommand ()
{
	setName("rcbot");
	setAccessLevel(0);
	add(new CWaypointCommand());
	add(new CAddBotCommand());
	add(new CPathWaypointCommand());
	add(new CDebugCommand());
	add(new CPrintCommands());
	add(new CConfigCommand());
	add(new CKickBotCommand());
	add(new CUsersCommand());
	add(new CUtilCommand());
}

CWaypointCommand :: CWaypointCommand()
{
	setName("waypoint");
	//setAccessLevel(CMD_ACCESS_WAYPOINT);
	setAccessLevel(0);
	add(new CWaypointOnCommand());
	add(new CWaypointOffCommand());
	add(new CWaypointAddCommand());
	add(new CWaypointDeleteCommand());
	add(new CWaypointInfoCommand());
	add(new CWaypointSaveCommand());
	add(new CWaypointLoadCommand());
	add(new CWaypointClearCommand());
	add(new CWaypointGiveTypeCommand());
	add(new CWaypointDrawTypeCommand());
	add(new CWaypointAngleCommand());
	add(new CWaypointSetAngleCommand());
	add(new CWaypointSetAreaCommand());
	add(new CWaypointSetRadiusCommand());
}


CWaypointSetAreaCommand :: CWaypointSetAreaCommand ()
{
	setName("setarea");
	setHelp("Go to a waypoint, use setarea <areaid>");
}

eBotCommandResult CWaypointSetAreaCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();

	if ( pcmd && *pcmd && ( CWaypoints::validWaypointIndex(pClient->currentWaypoint()) ) )
	{
		CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

		pWpt->setArea(atoi(pcmd));
	}
	else
		return COMMAND_ERROR;

	return COMMAND_ACCESSED;
}
///////////////
CWaypointSetRadiusCommand :: CWaypointSetRadiusCommand ()
{
	setName("setradius");
	setHelp("Go to a waypoint, use setradius <radius>");
}

eBotCommandResult CWaypointSetRadiusCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();

	if ( pcmd && *pcmd && ( CWaypoints::validWaypointIndex(pClient->currentWaypoint()) ) )
	{
		CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

		pWpt->setRadius(atof(pcmd));
	}
	else
		return COMMAND_ERROR;

	return COMMAND_ACCESSED;
}
/////////////////

CWaypointSetAngleCommand :: CWaypointSetAngleCommand()
{
	setName("updateyaw");
}

eBotCommandResult CWaypointSetAngleCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();

	if ( CWaypoints::validWaypointIndex(pClient->currentWaypoint()) )
	{
		CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

		pWpt->setAim(CBotGlobals::playerAngles(pClient->getPlayer()).y);
	}

	return COMMAND_ACCESSED;
}


CUsersCommand :: CUsersCommand ()
{
	setName("users");
	setAccessLevel(0);

	add(new CShowUsersCommand());
}

CDebugCommand :: CDebugCommand()
{
	setName("debug");
	setAccessLevel(CMD_ACCESS_DEBUG);

	add(new CDebugGameEventCommand());
	add(new CDebugBotCommand());
	add(new CDebugNavCommand());
	add(new CDebugVisCommand());
	add(new CDebugTaskCommand());
	add(new CDebugButtonsCommand());
	add(new CDebugSpeedCommand());
	add(new CDebugUsercmdCommand());

}
/////////////////////
CWaypointOnCommand::CWaypointOnCommand()
{
	setName("on");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

CWaypointDrawTypeCommand::CWaypointDrawTypeCommand()
{
	setName("drawtype");
	setHelp("0: for effects engine (maximum limit of beams)\n1: for debug overlay (no limit of beams) [LISTEN SERVER CLIENT ONLY]");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CWaypointDrawTypeCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( pClient )
	{
		if ( pcmd && *pcmd )
		{
			pClient->setDrawType(atoi(pcmd));
			return COMMAND_ACCESSED;
		}
	}

	return COMMAND_ERROR;
}

eBotCommandResult CWaypointOnCommand:: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( pClient )
		pClient->setWaypointOn(true);

	return COMMAND_ACCESSED;
}

eBotCommandResult CWaypointGiveTypeCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	edict_t *pEntity = pClient->getPlayer();

	if ( pcmd && *pcmd )
	{
		if ( pClient->currentWaypoint() == -1 )
			CBotGlobals::botMessage(pEntity,0,"No waypoint nearby to give types (move closer to the waypoint you want to give types)");
		else
		{
			CWaypointType *pType = CWaypointTypes::getType(pcmd);

			if ( pType )
			{
				CWaypoint *pWaypoint = CWaypoints::getWaypoint(pClient->currentWaypoint());

				if ( pWaypoint )
				{
					if ( pWaypoint->hasFlag(pType->getBits()) )
					{
						pWaypoint->removeFlag(pType->getBits());
						CBotGlobals::botMessage(pEntity,0,"type %s removed from waypoint %d",pcmd,CWaypoints::getWaypointIndex(pWaypoint));
					}
					else
					{
						pWaypoint->addFlag(pType->getBits());
						CBotGlobals::botMessage(pEntity,0,"type %s added to waypoint %d",pcmd,CWaypoints::getWaypointIndex(pWaypoint));
					}
					
				}
			}
			else
			{
				CBotGlobals::botMessage(pEntity,0,"type not found");
				CWaypointTypes::showTypesOnConsole(pEntity);
			}
		}
	}
	else
	{
		CWaypointTypes::showTypesOnConsole(pEntity);
	}

	return COMMAND_ACCESSED;
}

//////////////////
CWaypointOffCommand::CWaypointOffCommand()
{
	setName("off");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CWaypointClearCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	CWaypoints::init();
	CBotGlobals::botMessage(pClient->getPlayer(),0,"waypoints cleared");

	return COMMAND_ACCESSED;
}

eBotCommandResult CWaypointOffCommand:: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->setWaypointOn(false);
	CBotGlobals::botMessage(pClient->getPlayer(),0,"waypoints off");

	return COMMAND_ACCESSED;
}
////////////////////////
CWaypointAddCommand::CWaypointAddCommand()
{
	setName("add");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CWaypointAddCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	CWaypoints::addWaypoint(pClient);

	return COMMAND_ACCESSED;
}
////////////////////
CWaypointDeleteCommand ::CWaypointDeleteCommand()
{
	setName("delete");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CWaypointDeleteCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{	
	pClient->updateCurrentWaypoint();

	if ( CWaypoints::validWaypointIndex(pClient->currentWaypoint()) )
	{
		CWaypoints::deleteWaypoint(pClient->currentWaypoint());
		CBotGlobals::botMessage(pClient->getPlayer(),0,"waypoint %d deleted",pClient->currentWaypoint());
		pClient->updateCurrentWaypoint(); // waypoint deleted so get a new one
	}
	else
	{
		CBotGlobals::botMessage(pClient->getPlayer(),0,"no waypoint nearby to delete");
	}


	return COMMAND_ACCESSED;
}
/////////////////////

CAddBotCommand ::CAddBotCommand()
{
	setName("addbot");
	setAccessLevel(CMD_ACCESS_BOT);
}

eBotCommandResult CAddBotCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{	
//	bool bOkay = false;

	edict_t *pEntity = NULL;

	if ( pClient )
		pEntity = pClient->getPlayer();

	//if ( !pcmd || !*pcmd )
	//	bOkay = CBots::createBot();
	//else
	//bOkay = CBots::createBot();

	if ( CBots::createBot(pcmd,arg1,arg2) )
		CBotGlobals::botMessage(pEntity,0,"bot added");
	else
		CBotGlobals::botMessage(pEntity,0,"error: couldn't create bot! (Check maxplayers)");

	return COMMAND_ACCESSED;
}
/////////////////////

CPathWaypointCommand :: CPathWaypointCommand ()
{
	setName("pathwaypoint");
	setAccessLevel(CMD_ACCESS_WAYPOINT);

	add(new CPathWaypointOnCommand());
	add(new CPathWaypointOffCommand());
	add(new CPathWaypointAutoOnCommand());
	add(new CPathWaypointAutoOffCommand());
	add(new CPathWaypointCreate1Command());
	add(new CPathWaypointCreate2Command());
	add(new CPathWaypointRemove1Command());
	add(new CPathWaypointRemove2Command());
	add(new CPathWaypointDeleteToCommand());
	add(new CPathWaypointDeleteFromCommand());

}

CPathWaypointDeleteToCommand :: CPathWaypointDeleteToCommand()
{
	setName("deleteto");
}

eBotCommandResult CPathWaypointDeleteToCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();

	if ( CWaypoints::validWaypointIndex(pClient->currentWaypoint()) )
	{
		CWaypoints::deletePathsTo(pClient->currentWaypoint());
	}

	return COMMAND_ACCESSED;;
}


CPathWaypointDeleteFromCommand :: CPathWaypointDeleteFromCommand()
{
	setName("deletefrom");
}

eBotCommandResult CPathWaypointDeleteFromCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();

	if ( CWaypoints::validWaypointIndex(pClient->currentWaypoint()) )
	{
		CWaypoints::deletePathsFrom(pClient->currentWaypoint());
	}

	return COMMAND_ACCESSED;
}

CPathWaypointOnCommand :: CPathWaypointOnCommand()
{
	setName("on");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CPathWaypointOnCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->setPathWaypoint(true);
	return COMMAND_ACCESSED;
}

CPathWaypointOffCommand :: CPathWaypointOffCommand()
{
	setName("off");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CPathWaypointOffCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->setPathWaypoint(false);
	return COMMAND_ACCESSED;
}

CPathWaypointAutoOnCommand :: CPathWaypointAutoOnCommand()
{
	setName("enable");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CPathWaypointAutoOnCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->setAutoPath(true);
	return COMMAND_ACCESSED;
}

CPathWaypointAutoOffCommand :: CPathWaypointAutoOffCommand()
{
	setName("disable");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CPathWaypointAutoOffCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->setAutoPath(false);
	return COMMAND_ACCESSED;
}

CPathWaypointCreate1Command :: CPathWaypointCreate1Command()
{
	setName("create1");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CPathWaypointCreate1Command :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();

	pClient->setPathFrom(pClient->currentWaypoint());

	return COMMAND_ACCESSED;
}

CPathWaypointCreate2Command :: CPathWaypointCreate2Command()
{
	setName("create2");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CPathWaypointCreate2Command :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();
	pClient->setPathTo(pClient->currentWaypoint());

	CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->getPathFrom());

	// valid?
	if ( pWpt )
		pWpt->addPathTo(pClient->getPathTo());

	return COMMAND_ACCESSED;
}

CPathWaypointRemove1Command :: CPathWaypointRemove1Command()
{
	setName("remove1");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CPathWaypointRemove1Command :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();
	pClient->setPathFrom(pClient->currentWaypoint());
	return COMMAND_ACCESSED;
}

CPathWaypointRemove2Command :: CPathWaypointRemove2Command()
{
	setName("remove2");
	setAccessLevel(CMD_ACCESS_WAYPOINT);
}

eBotCommandResult CPathWaypointRemove2Command :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();
	pClient->setPathTo(pClient->currentWaypoint());

	CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->getPathFrom());

	// valid?
	if ( pWpt )
		pWpt->removePathTo(pClient->getPathTo());

	return COMMAND_ACCESSED;	
}

CWaypointAngleCommand :: CWaypointAngleCommand()
{
	setName("angle");
}

eBotCommandResult CWaypointAngleCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( pClient && pClient->getPlayer() )
	{
		pClient->updateCurrentWaypoint();

		CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

		if ( pWpt )
		{
			QAngle eye = CBotGlobals::playerAngles(pClient->getPlayer());
			CBotGlobals::botMessage(pClient->getPlayer(),0,"Waypoint Angle == %0.3f deg, (Eye == %0.3f)",CBotGlobals::yawAngleFromEdict(pClient->getPlayer(),pWpt->getOrigin()),eye.y);
		}
	}

	return COMMAND_ACCESSED;
}

CWaypointInfoCommand :: CWaypointInfoCommand()
{
	setName("info");
}

eBotCommandResult CWaypointInfoCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	pClient->updateCurrentWaypoint();

	CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

	if ( pWpt )
		pWpt->info(pClient->getPlayer());

	return COMMAND_ACCESSED;
}

eBotCommandResult CWaypointSaveCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( CWaypoints::save(false) )
		CBotGlobals::botMessage(pClient->getPlayer(),0,"waypoints saved");
	else
		CBotGlobals::botMessage(pClient->getPlayer(),0,"error: could not save waypoints");

	return COMMAND_ACCESSED;
}

eBotCommandResult CWaypointLoadCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( CWaypoints::load() )
		CBotGlobals::botMessage(pClient->getPlayer(),0,"waypoints loaded");
	else
		CBotGlobals::botMessage(pClient->getPlayer(),0,"error: could not load waypoints");

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugGameEventCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
		return COMMAND_ERROR;

	pClient->setDebug(BOT_DEBUG_GAME_EVENT,atoi(pcmd)>0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugVisCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
		return COMMAND_ERROR;

	pClient->setDebug(BOT_DEBUG_VIS,atoi(pcmd)>0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugNavCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
		return COMMAND_ERROR;

	pClient->setDebug(BOT_DEBUG_NAV,atoi(pcmd)>0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugTaskCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
		return COMMAND_ERROR;

	pClient->setDebug(BOT_DEBUG_TASK,atoi(pcmd)>0);

	return COMMAND_ACCESSED;
}


eBotCommandResult CDebugSpeedCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
		return COMMAND_ERROR;

	pClient->setDebug(BOT_DEBUG_SPEED,atoi(pcmd)>0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugUsercmdCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
		return COMMAND_ERROR;

	pClient->setDebug(BOT_DEBUG_USERCMD,atoi(pcmd)>0);

	return COMMAND_ACCESSED;
}


eBotCommandResult CDebugButtonsCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
		return COMMAND_ERROR;

	pClient->setDebug(BOT_DEBUG_BUTTONS,atoi(pcmd)>0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugBotCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
	{
		pClient->setDebugBot(NULL);
		CBotGlobals::botMessage(pClient->getPlayer(),0,"debug bot cleared");
		return COMMAND_ERROR;
	}
	
	edict_t *pEnt = CBotGlobals::findPlayerByTruncName(pcmd);

	if ( !pEnt )
	{
		CBotGlobals::botMessage(pClient->getPlayer(),0,"can't find a player with that name");
		return COMMAND_ERROR;
	}

	CBot *pBot = CBots::getBotPointer(pEnt);

	if ( !pBot )
	{
		CBotGlobals::botMessage(pClient->getPlayer(),0,"can't find a bot with that name");
		return COMMAND_ERROR;
	}

	pClient->setDebugBot(pBot);	

	return COMMAND_ACCESSED;
}

///////////////////////
// command

CUtilCommand :: CUtilCommand()
{
	setName("util");
	add(new CSearchCommand());
}

CConfigCommand :: CConfigCommand()
{
	setName("config");
	add(new CGameEventVersion());
	add(new CMaxBotsCommand());
	add(new CMinBotsCommand());
}

eBotCommandResult CGameEventVersion :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
		return COMMAND_ERROR;

	CBotGlobals::setEventVersion(atoi(pcmd));
	
	return COMMAND_ACCESSED;
}


// kickbot
eBotCommandResult CKickBotCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( !pcmd || !*pcmd )
	{
		//remove random bot
		CBots::kickRandomBot();
	}
	else
	{
		int team = atoi(pcmd);

		CBots::kickRandomBotOnTeam(team);
	}

	
	return COMMAND_ACCESSED;
}

eBotCommandResult CShowUsersCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	edict_t *pEntity = NULL;

	if ( pClient )
		pEntity = pClient->getPlayer();

	CAccessClients::showUsers(pEntity);

	return COMMAND_ACCESSED;
}

eBotCommandResult CMaxBotsCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	edict_t *pEntity = NULL;

	if ( pClient )
		pEntity = pClient->getPlayer();

	if ( pcmd && *pcmd )
	{
		int max = atoi(pcmd);

		bool err = false;
		int min_bots = CBots::getMinBots();

		if ( max <= -1 )// skip check for disabling max bots (require <=)
			max = -1;
		else if ( (min_bots >= 0) && (max <= min_bots) )
		{
			CBotGlobals::botMessage(pEntity,0,"max_bots must be greater than min_bots (min_bots is currently : %d)",min_bots);
			err = true;
		}
		if ( max > CBotGlobals::maxClients() )				
			max = CBotGlobals::maxClients();

		if ( !err )
		{
			CBots :: setMaxBots(max);

			CBotGlobals::botMessage(pEntity,0,"max_bots set to %d",max);
		}
		
	}
	else
		CBotGlobals::botMessage(pEntity,0,"max_bots is currently %d",CBots::getMaxBots());

	return COMMAND_ACCESSED;
}

eBotCommandResult CMinBotsCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	edict_t *pEntity = NULL;

	if ( pClient )
		pEntity = pClient->getPlayer();

	if ( pcmd && *pcmd )
	{
		int min = atoi(pcmd);
		int max_bots = CBots::getMaxBots();

		bool err = false;

		if ( min > CBotGlobals::maxClients() )
			min = CBotGlobals::maxClients();	
		
		if ( min <= -1 ) // skip check for disabling min bots (require <=)
			min = -1;
		else if ( (max_bots >= 0) && (min >= CBots::getMaxBots()) )
		{
			CBotGlobals::botMessage(pEntity,0,"min_bots must be less than max_bots (max_bots is currently : %d)",max_bots);
			err = true;
		}	

		if ( !err )
		{
			CBots :: setMinBots(min);

			CBotGlobals::botMessage(pEntity,0,"min_bots set to %d",min);
		}
	}
	else
		CBotGlobals::botMessage(pEntity,0,"min_bots is currently %d",CBots::getMinBots());

	return COMMAND_ACCESSED;
}

eBotCommandResult CSearchCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	int i = 0;

	edict_t *pPlayer = pClient->getPlayer();
	edict_t *pEdict;
	float fDistance;
	string_t model;

	for ( i = 0; i < gpGlobals->maxEntities; i ++ )
	{
		pEdict = INDEXENT(i);

		if ( pEdict )
		{
			if ( !pEdict->IsFree() )
			{
				if ( pEdict->m_pNetworkable && pEdict->GetIServerEntity() )
				{				
					if ( (fDistance=(CBotGlobals::entityOrigin(pEdict) - CBotGlobals::entityOrigin(pPlayer)).Length()) < 128 )
					{
						model = pEdict->GetIServerEntity()->GetModelName();

						//CBaseEntity *p = pEdict->GetNetworkable()->GetBaseEntity();

						
						
						CBotGlobals::botMessage(pPlayer,0,"D:%0.2f C:'%s', MI:%d, MN:'%s'",fDistance,pEdict->GetClassName(),pEdict->GetIServerEntity()->GetModelIndex(),model.ToCStr());
					}
				}
			}
		}
	}

	return COMMAND_ACCESSED;

}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CBotCommand :: setName ( char *szName )
{
	m_szCommand = strdup(szName);
}

void CBotCommand :: setHelp ( char *szHelp )
{
	m_szHelp = strdup(szHelp);
}

void CBotCommand :: setAccessLevel ( int iAccessLevel )
{
	m_iAccessLevel = iAccessLevel;
}

CBotCommand :: CBotCommand ( char *szCommand, int iAccessLevel )
{
	m_szCommand = CStrings::getString(szCommand);
	m_iAccessLevel = iAccessLevel;
}

void CBotCommand :: freeMemory ()
{
	// nothing to free

	if ( m_szHelp != NULL )
	{
		free(m_szHelp);
		m_szHelp = NULL;
	}

	if ( m_szCommand != NULL )
	{
		free(m_szCommand);
		m_szCommand = NULL;
	}
}

bool CBotCommand :: hasAccess ( CClient *pClient )
{
	return (m_iAccessLevel & pClient->accessLevel()) == m_iAccessLevel;
}

bool CBotCommand :: isCommand ( const char *szCommand )
{
	return FStrEq(szCommand,m_szCommand);
}

eBotCommandResult CBotCommand :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	return COMMAND_NOT_FOUND;
}

////////////////////////////
// container of commands
eBotCommandResult CBotCommandContainer :: execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	for ( unsigned int i = 0; i < m_theCommands.size(); i ++ )
	{
		CBotCommand *pCommand = m_theCommands[i];

		if ( pCommand->isCommand(pcmd) )
		{			
			if ( pClient && !pCommand->hasAccess(pClient) )
				return COMMAND_REQUIRE_ACCESS;
			if ( !pClient && !canbeUsedDedicated() )
			{
				CBotGlobals::botMessage(NULL,0,"Sorry, this command cannot be used on a dedicated server");
				return COMMAND_ERROR;
			}
			// move arguments
			eBotCommandResult iResult = pCommand->execute(pClient,arg1,arg2,arg3,arg4,arg5,NULL);

			if ( iResult == COMMAND_ERROR )
			{
				if ( pClient )
					pCommand->printHelp(pClient->getPlayer());
				else
					pCommand->printHelp(NULL);
			}
			
			return COMMAND_ACCESSED;
		}
	}

	if ( pClient )
		printHelp(pClient->getPlayer());
	else
		printHelp(NULL);

	return COMMAND_NOT_FOUND;
}

void CBotCommandContainer :: freeMemory ()
{
	for ( unsigned int i = 0; i < m_theCommands.size(); i ++ )
	{
		m_theCommands[i]->freeMemory();
		delete m_theCommands[i];
		m_theCommands[i] = NULL;
	}

	m_theCommands.clear();
}
//////////////////////////////////////////

eBotCommandResult CPrintCommands ::execute ( CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5 )
{
	if ( pClient == NULL )
	{
		CBotGlobals::botMessage(pClient->getPlayer(),0,"All bot commands:");
		CBotGlobals::m_pCommands->printCommand(pClient->getPlayer());
	}
	else
	{
		CBotGlobals::botMessage(NULL,0,"All bot commands:");
		CBotGlobals::m_pCommands->printCommand(NULL);
	}

	return COMMAND_ACCESSED;
}

///////////////////////////////////////////

void CBotCommand :: printCommand ( edict_t *pPrintTo, int indent )
{
	if ( indent )
	{
		const int maxIndent = 64;
		char szIndent[maxIndent];
		int i;

		for ( i = 0; (i < (indent*2)) && (i < maxIndent-1); i ++ )
			szIndent[i] = ' ';

		szIndent[maxIndent-1] = 0;
		szIndent[i]=0;

		if ( !pPrintTo && !canbeUsedDedicated() )
			CBotGlobals::botMessage(pPrintTo,0,"%s%s [can't use]",szIndent,m_szCommand);
		else
			CBotGlobals::botMessage(pPrintTo,0,"%s%s",szIndent,m_szCommand);
	}
	else
	{
		if ( !pPrintTo && !canbeUsedDedicated() )
			CBotGlobals::botMessage(pPrintTo,0,"%s [can't use]",m_szCommand);
		else
			CBotGlobals::botMessage(pPrintTo,0,m_szCommand);
	}
}

void CBotCommand :: printHelp ( edict_t *pPrintTo )
{
	if ( m_szHelp )
		CBotGlobals::botMessage(pPrintTo,0,m_szHelp);
	else
		CBotGlobals::botMessage(pPrintTo,0,"Sorry, no help for this command (yet)");

	return;
}

void CBotCommandContainer :: printCommand ( edict_t *pPrintTo, int indent )
{
	//char cmd1[512];
	//char cmd2[512];

	//sprintf(cmd1,"%%%ds",indent);
	//sprintf(cmd2,cmd1,m_szCommand);

	if ( indent )
	{
		const int maxIndent = 64;
		char szIndent[maxIndent];

		int i;

		for ( i = 0; (i < (indent*2)) && (i < maxIndent-1); i ++ )
			szIndent[i] = ' ';

		szIndent[maxIndent-1] = 0;
		szIndent[i]=0;

		CBotGlobals::botMessage(pPrintTo,0,"%s[%s]",szIndent,m_szCommand);
	}
	else
		CBotGlobals::botMessage(pPrintTo,0,"[%s]",m_szCommand);

	for ( unsigned int i = 0; i < m_theCommands.size(); i ++ )
	{
		m_theCommands[i]->printCommand(pPrintTo,indent+1);
	}
}

void CBotCommandContainer :: printHelp ( edict_t *pPrintTo )
{
	printCommand(pPrintTo);
	return;
}