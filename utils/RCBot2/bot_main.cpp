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
//
// HPB_bot2_main.cpp - bot source code file (Copyright 2004, Jeffrey "botman" Broome)
//
//=================================================================================//

#include <stdio.h>
#include <time.h>

//#include "cbase.h"
//#include "baseentity.h"
#include "filesystem.h"
#include "interface.h"
#include "engine/iserverplugin.h"
#include "iplayerinfo.h"
#include "IEngineTrace.h"
#include "tier2/tier2.h"
#ifdef __linux__
#include "shake.h"    //bir3yk
#endif
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

static ICvar *s_pCVar;

ConVar bot_spyknifefov("rcbot_spyknifefov","80",0,"the FOV from the enemy that spies must backstab from");
ConVar bot_visrevs("rcbot_visrevs","9",0,"how many revs the bot searches for visible players and enemies, lower to reduce cpu usage");
ConVar bot_pathrevs("rcbot_pathrevs","40",0,"how many revs the bot searches for a path each frame, lower to reduce cpu usage, but causes bots to stand still more");
ConVar bot_command("rcbot_cmd","",0,"issues a command to all bots");
ConVar bot_rocketpredict( "rcbot_rocketpred", "0.4", 0, "multiplier for soldier / demoman rocket/grenade prediction" );
ConVar bot_attack( "rcbot_flipout", "0", 0, "Rcbots all attack" );
ConVar bot_scoutdj( "rcbot_scoutdj", "0.28", 0, "time scout uses to double jump" );
ConVar bot_anglespeed( "rcbot_anglespeed", "8.0", 0, "speed that bots turn" );
ConVar bot_stop( "rcbot_stop", "0", 0, "Make bots stop thinking!");
ConVar bot_waypointpathdist("rcbot_wpt_pathdist","512",0,"Length for waypoints to automatically add paths at");
ConVar bot_rj("rcbot_rj","0.01",0,"time for soldier to fire rocket after jumping");
ConVar bot_defrate("rcbot_defrate","0.24",0,"rate for bots to defend");
ConVar bot_beliefmulti("rcbot_beliefmulti","20.0",0,"multiplier for increasing bot belief");
ConVar bot_belief_fade("rcbot_belief_fade","0.75",0,"the multiplayer rate bot belief decreases");
ConVar bot_change_class("rcbot_change_classes","0",0,"bots change classes at random intervals");
ConVar bot_use_vc_commands("rcbot_voice_cmds","1",0,"bots use voice commands e.g. medic/spy etc");
ConVar bot_use_disp_dist("rcbot_disp_dist","800.0",0,"distance that bots will go back to use a dispenser");
ConVar bot_max_cc_time("rcbot_max_cc_time","240",0,"maximum time for bots to consider changing class <seconds>");
ConVar bot_min_cc_time("rcbot_min_cc_time","60",0,"minimum time for bots to consider changing class <seconds>");
ConVar bot_avoid_radius("rcbot_avoid_radius","100",0,"radius in units for bots to avoid things");
ConVar bot_avoid_strength("rcbot_avoid_strength","64",0,"strength of avoidance (0 = disable)");

// Interfaces from the engine*/
IVEngineServer *engine = NULL;  // helper functions (messaging clients, loading content, making entities, running commands, etc)
IFileSystem *filesystem = NULL;  // file I/O 
IGameEventManager2 *gameeventmanager = NULL;
IGameEventManager *gameeventmanager1 = NULL;  // game events interface
IPlayerInfoManager *playerinfomanager = NULL;  // game dll interface to interact with players
IServerPluginHelpers *helpers = NULL;  // special 3rd party plugin helpers from the engine
IServerGameClients* gameclients = NULL;
IEngineTrace *enginetrace = NULL;
IEffects *g_pEffects = NULL;
IBotManager *g_pBotManager = NULL;
CGlobalVars *gpGlobals = NULL;
IVDebugOverlay *debugoverlay = NULL;
IServerGameEnts *servergameents = NULL; // for accessing the server game entities
IServerGameDLL *servergamedll = NULL;

// 
// The plugin is a static singleton that is exported as an interface
//
CRCBotPlugin g_RCBOTServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CRCBotPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_RCBOTServerPlugin );
/* shameless hack
typedef float (*HKPROCESSUSERCMDS) ( edict_t *, bf_read *, int , int ,int , bool , bool  );

HKPROCESSUSERCMDS ProcessUserCmds = NULL;

float MyProcessUsercmds( edict_t *player, bf_read *buf, int numcmds, int totalcmds, int dropped_packets, bool ignore, bool paused )
{
	return ProcessUserCmds(player,buf,numcmds,totalcmds,dropped_packets,ignore,paused);
}*/

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
CRCBotPlugin::CRCBotPlugin()
{
	m_iClientCommandIndex = 0;
}

CRCBotPlugin::~CRCBotPlugin()
{
}

//------------
//---------------------------------------------------------------------------------
// Purpose: an example of how to implement a new command
//---------------------------------------------------------------------------------
static ConVar empty_cvar(BOT_VER_CVAR, BOT_VER, FCVAR_REPLICATED, BOT_NAME_VER);

CON_COMMAND( rcbotd, "access the bot commands on a server" )
{
        eBotCommandResult iResult;

		if ( !engine->IsDedicatedServer() || !CBotGlobals::IsMapRunning() )
		{
			CBotGlobals::botMessage(NULL,0,"Error, no map running or not dedicated server");
			return;
		}

		//iResult = CBotGlobals::m_pCommands->execute(NULL,engine->Cmd_Argv(1),engine->Cmd_Argv(2),engine->Cmd_Argv(3),engine->Cmd_Argv(4),engine->Cmd_Argv(5),engine->Cmd_Argv(6));
		iResult = CBotGlobals::m_pCommands->execute(NULL,args.Arg(1),args.Arg(2),args.Arg(3),args.Arg(4),args.Arg(5),args.Arg(6));

		if ( iResult == COMMAND_ACCESSED )
		{
			// ok
		}
		else if ( iResult == COMMAND_REQUIRE_ACCESS )
		{
			CBotGlobals::botMessage(NULL,0,"You do not have access to this command");
		}
		else if ( iResult == COMMAND_NOT_FOUND )
		{
			CBotGlobals::botMessage(NULL,0,"bot command not found");	
		}
		else if ( iResult == COMMAND_ERROR )
		{
			CBotGlobals::botMessage(NULL,0,"bot command returned an error");	
		}
}

void CRCBotPlugin::OnEdictAllocated( edict_t *edict )
{
}

void CRCBotPlugin::OnEdictFreed( const edict_t *edict  )
{
}



///////////////
// hud message

void CRCBotPlugin :: HudTextMessage ( edict_t *pEntity, char *szMsgName, char *szTitle, char *szMessage, Color colour, int level, int time )
{
	KeyValues *kv = new KeyValues( "menu" );
	kv->SetString( "title", szMessage );
	
	kv->SetColor( "color", colour);
	kv->SetInt( "level", level);
	kv->SetInt( "time", time);
//DIALOG_TEXT
	helpers->CreateMessage( pEntity, DIALOG_MSG, kv, &g_RCBOTServerPlugin );

	kv->deleteThis();
}

#ifdef __linux__
#define LOAD_INTERFACE(var,type,version) if ( (var = (type*)interfaceFactory(version,NULL)) == NULL ) { Warning("[RCBOT] Cannot open interface "#version " "#type " "#var "\n"); return false; } else { Msg("[RCBOT] Found interface "#version " "#type " "#var "\n"); }
#define LOAD_GAME_SERVER_INTERFACE(var,type,version) if ( (var = (type*)gameServerFactory(version,NULL)) == NULL ) { Warning("[RCBOT] Cannot open game server interface "#version " "#type " "#var "\n"); return false; } else { Msg("[RCBOT] Found interface "#version " "#type " "#var "\n"); }
#else
#define LOAD_INTERFACE(var,type,version) if ( (var = (type*)interfaceFactory(version,NULL)) == NULL ) { Warning("[RCBOT] Cannot open interface "## #version ##" "## #type ##" "## #var ##"\n"); return false; } else { Msg("[RCBOT] Found interface "## #version ##" "## #type ##" "## #var ## "\n"); }
#define LOAD_GAME_SERVER_INTERFACE(var,type,version) if ( (var = (type*)gameServerFactory(version,NULL)) == NULL ) { Warning("[RCBOT] Cannot open game server interface "## #version ##" "## #type ##" "## #var ##"\n"); return false; } else { Msg("[RCBOT] Found interface "## #version ##" "## #type ##" "## #var ## "\n"); }
#endif
//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool CRCBotPlugin::Load( CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	extern MTRand_int32 irand;

	ConnectTier1Libraries( &interfaceFactory, 1 );
	ConnectTier2Libraries( &interfaceFactory, 1 );

	LOAD_GAME_SERVER_INTERFACE(playerinfomanager,IPlayerInfoManager,INTERFACEVERSION_PLAYERINFOMANAGER);

	gpGlobals = playerinfomanager->GetGlobalVars();	

	LOAD_INTERFACE(engine,IVEngineServer,INTERFACEVERSION_VENGINESERVER);
	LOAD_INTERFACE(filesystem,IFileSystem,FILESYSTEM_INTERFACE_VERSION);
	LOAD_INTERFACE(helpers,IServerPluginHelpers,INTERFACEVERSION_ISERVERPLUGINHELPERS);
	LOAD_INTERFACE(enginetrace,IEngineTrace,INTERFACEVERSION_ENGINETRACE_SERVER);
	LOAD_GAME_SERVER_INTERFACE(servergameents,IServerGameEnts,INTERFACEVERSION_SERVERGAMEENTS);
	LOAD_GAME_SERVER_INTERFACE(g_pEffects,IEffects,IEFFECTS_INTERFACE_VERSION);
	LOAD_GAME_SERVER_INTERFACE(g_pBotManager,IBotManager,INTERFACEVERSION_PLAYERBOTMANAGER);

#ifndef __linux__
    LOAD_INTERFACE(debugoverlay,IVDebugOverlay,VDEBUG_OVERLAY_INTERFACE_VERSION);
#endif
	LOAD_INTERFACE(gameeventmanager,IGameEventManager2,INTERFACEVERSION_GAMEEVENTSMANAGER2)
	LOAD_INTERFACE(gameeventmanager1,IGameEventManager,INTERFACEVERSION_GAMEEVENTSMANAGER)

	if ( (servergamedll = (IServerGameDLL*)gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL,NULL)) == NULL ) 
	{ 
		Msg("[RCBOT] Cannot open latest game server interface for TF2\nChecking for older version...");

		if ( (servergamedll = (IServerGameDLL*)gameServerFactory("ServerGameDLL006",NULL)) == NULL )
		{
			Msg("[RCBOT] Cannot open older game server interface\nChecking for older version...");

			if ( (servergamedll = (IServerGameDLL*)gameServerFactory("ServerGameDLL005",NULL)) == NULL )
			{
				Warning("[RCBOT] Cannot open older game server interface\n");
				return false;
			}
			else
				Msg("[RCBOT] Found older game server interface");
		}
		else
			Msg("[RCBOT] Found older game server interface");
	}
	else
		Msg("[RCBOT] Found interface %s for TF2\n",INTERFACEVERSION_SERVERGAMEDLL); 

	//LOAD_GAME_SERVER_INTERFACE(servergamedll,IServerGameDLL,"ServerGameDLL006");

	LOAD_GAME_SERVER_INTERFACE(gameclients,IServerGameClients,INTERFACEVERSION_SERVERGAMECLIENTS);

/*	int* vptr =  *(int**)gameclients;

	ProcessUserCmds = (HKPROCESSUSERCMDS)vptr[9];

	int src = (int)&MyProcessUsercmds;
	memcpy(&(vptr[9]),&src,sizeof(int));

	//ProcessUserCmds = gameclients->ProcessUsercmds;
	//initUserCmdHook(gameclients);*/

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	ConVar_Register( 0 );
	//InitCVars( interfaceFactory ); // register any cvars we have defined

	srand( (unsigned)time(NULL) );  // initialize the random seed
	irand.seed( (unsigned)time(NULL) );

	eventListener2 = new CRCBotEventListener();

	// Initialize bot variables
	CBotProfiles::setupProfiles();
	CBotGlobals::gameStart();	
	//CBotEvents::setupEvents();
	CWaypointTypes::setup();
	CWaypoints::setupVisibility();

	CBotConfigFile::reset();	
	CBotConfigFile::load();

	CRCBotPlugin::ShowLicense();	

	RandomSeed((unsigned int)time(NULL));

	return true;
}

void CRCBotPlugin::ShowLicense ( void )
{
	
 Msg ("-----------------------------------------------------------------\n");
 Msg (" RCBOT LICENSE\n");
 Msg ("-----------------------------------------------------------------\n");
 Msg ("RCBot by Paul Murphy adapted from Botman's HPB Bot 2 template.\n\n");

 Msg ("RCBot is free software; you can redistribute it and/or modify it\n");
 Msg ("under the terms of the GNU General Public License as published by the\n");
 Msg ("Free Software Foundation; either version 2 of the License, or (at\n");
 Msg ("your option) any later version.\n\n");

 Msg ("RCBot is distributed in the hope that it will be useful, but\n");
 Msg ("WITHOUT ANY WARRANTY; without even the implied warranty of\n");
 Msg ("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n");
 Msg ("General Public License for more details.\n\n");

 Msg ("You should have received a copy of the GNU General Public License\n");
 Msg ("along with RCBot; if not, write to the Free Software Foundation,\n");
 Msg ("Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\n");

 Msg ("In addition, as a special exception, the author gives permission to\n");
 Msg ("link the code of this program with the Half-Life Game Engine (\"HL\"\n");
 Msg ("Engine\") and Modified Game Libraries (\"MODs\") developed by Valve,\n");
 Msg ("L.L.C (\"Valve\").  You must obey the GNU General Public License in all\n");
 Msg ("respects for all of the code used other than the HL Engine and MODs\n");
 Msg ("from Valve.  If you modify this file, you may extend this exception\n");
 Msg ("to your version of the file, but you are not obligated to do so.  If\n");
 Msg ("you do not wish to do so, delete this exception statement from your\n");
 Msg ("version.\n");
 Msg ("-----------------------------------------------------------------\n");
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CRCBotPlugin::Unload( void )
{
	//unloadUserCmdHook(gameclients);

	CBots::freeAllMemory();
	CStrings::freeAllMemory();
	CBotGlobals::freeMemory();
	CBotMods::freeMemory();
	CAccessClients::freeMemory();
	CBotEvents::freeMemory();
	CWaypoints::freeMemory();
	CWaypointTypes::freeMemory();
	CBotProfiles::deleteProfiles();
	CWeapons::freeMemory();

	//ConVar_Unregister();

	gameeventmanager1->RemoveListener( this ); // make sure we are unloaded from the event system
	if ( gameeventmanager )
	{
		if ( eventListener2 )
		{
			gameeventmanager->RemoveListener( eventListener2 );
			delete eventListener2;
		}
	}

	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is paused (i.e should stop running but isn't unloaded)
//---------------------------------------------------------------------------------
void CRCBotPlugin::Pause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unpaused (i.e should start executing again)
//---------------------------------------------------------------------------------
void CRCBotPlugin::UnPause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *CRCBotPlugin::GetPluginDescription( void )
{
	return "RCBot2 Plugin, by Cheeseh";
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CRCBotPlugin::LevelInit( char const *pMapName )
{
	// Must set this
	CBotGlobals::setMapName(pMapName);

	Msg( "Level \"%s\" has been loaded\n", pMapName );

	CWaypoints::precacheWaypointTexture();

	CWaypointDistances::reset();

	CProfileTimers::reset();

	CWaypoints::init();
	CWaypoints::load();

	CBotGlobals::setMapRunning(true);
	CBotConfigFile::reset();
	
	//ConVar *pTeamplay = (ConVar*)rcbotd_command.GetCommands()->FindCommand("mp_teamplay");

	//if ( pTeamplay )
	//	CBotGlobals::setTeamplay(pTeamplay->GetBool());
	//else
		CBotGlobals::setTeamplay(false);

	gameeventmanager1->AddListener( this, true );	

	CBotEvents::setupEvents();

	CBots::mapInit();

	CBotMod *pMod = CBotGlobals::getCurrentMod();
	
	if ( pMod )
		pMod->mapInit();
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void CRCBotPlugin::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	Msg( "clientMax is %d\n", clientMax );

	CAccessClients::load();

	CBotGlobals::setClientMax(clientMax);
}

void CRCBotPlugin::PreClientUpdate(bool simulating)
{
	//if ( simulating && CBotGlobals::IsMapRunning() )
	//{
	//	CBots::runPlayerMoveAll();
	//}
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CRCBotPlugin::GameFrame( bool simulating )
{
	if ( simulating && CBotGlobals::IsMapRunning() )
	{
		CBots::botThink();
		//gameclients->PostClientMessagesSent();
		CBots::handleAutomaticControl();
		CClients::clientThink();

		if ( CWaypoints::getVisiblity()->needToWorkVisibility() )
		{
			CWaypoints::getVisiblity()->workVisibility();
		}

		// Profiling
#ifdef _DEBUG
		if ( CClients::clientsDebugging(BOT_DEBUG_PROFILE) )
		{
			CProfileTimers::updateAndDisplay();
		}
#endif

		// Config Commands
		CBotConfigFile::doNextCommand();
	}
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CRCBotPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	CWaypointDistances::save();
	CBots::freeMapMemory();	
	CWaypoints::init();

	CBotGlobals::setMapRunning(false);
	CBotEvents::freeMemory();

	gameeventmanager1->RemoveListener( this );

	if ( gameeventmanager )
		gameeventmanager->RemoveListener( eventListener2 );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CRCBotPlugin::ClientActive( edict_t *pEntity )
{
	CClients::clientActive(pEntity);
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CRCBotPlugin::ClientDisconnect( edict_t *pEntity )
{
	CClients::clientDisconnected(pEntity);
}

//---------------------------------------------------------------------------------
// Purpose: called on client being added to this server
//---------------------------------------------------------------------------------
void CRCBotPlugin::ClientPutInServer( edict_t *pEntity, char const *playername )
{
	if ( CBots::controlBots() )
		CBots::handlePlayerJoin(pEntity,playername);

	CClients::clientConnected(pEntity);
}

CRCBotEventListener *CRCBotPlugin:: getEventListener ( void )
{
	return eventListener2;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CRCBotPlugin::SetCommandClient( int index )
{
	m_iClientCommandIndex = index;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CRCBotPlugin::ClientSettingsChanged( edict_t *pEdict )
{
	/*
	if ( playerinfomanager )
	{
		IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo( pEdict );
		
		const char *name = engine->GetClientConVarValue( engine->IndexOfEdict(pEdict), "name" );

		if ( playerinfo && name && playerinfo->GetName() && 
			 Q_stricmp( name, playerinfo->GetName()) ) // playerinfo may be NULL if the MOD doesn't support access to player data 
													   // OR if you are accessing the player before they are fully connected
		{
			char msg[128];
			Q_snprintf( msg, sizeof(msg), "Your name changed to \"%s\" (from \"%s\"\n", name, playerinfo->GetName() ); 
			engine->ClientPrintf( pEdict, msg ); // this is the bad way to check this, the better option it to listen for the "player_changename" event in FireGameEvent()
												// this is here to give a real example of how to use the playerinfo interface
		}
	}*/
}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT CRCBotPlugin::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
	CClients::init(pEntity);

	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT CRCBotPlugin::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	const char *pcmd = args.Arg(0);
	CBotMod *pMod;
	//const char *pcmd = engine->Cmd_Argv(0);

	if ( !pEntity || pEntity->IsFree() ) 
	{
		return PLUGIN_CONTINUE;
	}

	CClient *pClient = CClients::get(pEntity);

	// is bot command?
	if ( CBotGlobals::m_pCommands->isCommand(pcmd) )
	{		
		//eBotCommandResult iResult = CBotGlobals::m_pCommands->execute(pClient,engine->Cmd_Argv(1),engine->Cmd_Argv(2),engine->Cmd_Argv(3),engine->Cmd_Argv(4),engine->Cmd_Argv(5),engine->Cmd_Argv(6));
		eBotCommandResult iResult = CBotGlobals::m_pCommands->execute(pClient,args.Arg(1),args.Arg(2),args.Arg(3),args.Arg(4),args.Arg(5),args.Arg(6));

		if ( iResult == COMMAND_ACCESSED )
		{
			// ok
		}
		else if ( iResult == COMMAND_REQUIRE_ACCESS )
		{
			CBotGlobals::botMessage(pEntity,0,"You do not have access to this command");
		}
		else if ( iResult == COMMAND_NOT_FOUND )
		{
			CBotGlobals::botMessage(pEntity,0,"bot command not found");	
		}
		else if ( iResult == COMMAND_ERROR )
		{
			CBotGlobals::botMessage(pEntity,0,"bot command returned an error");	
		}

		return PLUGIN_STOP; // we handled this function
	}


	pMod = CBotGlobals::getCurrentMod();

	pMod->clientCommand(pEntity,args.ArgC(),pcmd,args.Arg(1),args.Arg(2));

	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT CRCBotPlugin::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
{
	return PLUGIN_CONTINUE;
}

void CRCBotPlugin::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue )
{
	return;
}


//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------
void CRCBotPlugin::FireGameEvent( KeyValues * event )
{
	if ( CBotGlobals :: isEventVersion (1) )
	{
		const char *type = event->GetName();

		CBotEvents::executeEvent((void*)event,TYPE_KEYVALUES);	

		if ( CClients::clientsDebugging() )
		{
			CClients::clientDebugMsg(BOT_DEBUG_GAME_EVENT,type);

			for ( KeyValues *pk = event->GetFirstTrueSubKey(); pk; pk = pk->GetNextTrueSubKey())
			{
				CClients::clientDebugMsg(BOT_DEBUG_GAME_EVENT,pk->GetName());
			}
			for ( KeyValues *pk = event->GetFirstValue(); pk; pk = pk->GetNextValue() )
			{		
				char szMsg[512];

				sprintf(szMsg,"%s = %s",pk->GetName(),pk->GetString());
				CClients::clientDebugMsg(BOT_DEBUG_GAME_EVENT,szMsg);
			}
		}
	}
}

void CRCBotEventListener::FireGameEvent( IGameEvent * event )
{
	if ( CBotGlobals :: isEventVersion (2) )
	{
		CBotEvents::executeEvent((void*)event,TYPE_IGAMEEVENT);
	}
}



///////////////////
//useful functions
// defined in const more efficiently
/*int round ( float f )
{
	f = f-(float)((int)f);

	if ( f >= 0.5 )
		return (int)f+1;
	
	return (int)f;
}*/
//defined in const more efficiently
/*int RANDOM_INT(int min, int max)
{    
    return min + round(((float)rand()/RAND_MAX)*(float)(max-min));
}*/

//////////////////////


bool FStrEq(const char *sz1, const char *sz2)
{
	return(Q_stricmp(sz1, sz2) == 0);
}

/*edict_t* INDEXENT( int iEdictNum )		
{ 
	return engine->PEntityOfEntIndex(iEdictNum); 
}

#ifndef GAME_DLL
// get entity index
int ENTINDEX( edict_t *pEdict )		
{ 
	return engine->IndexOfEdict(pEdict);
}
#endif*/

int Ceiling ( float fVal )
{
	int loVal = (int)fVal;

	fVal -= (float)loVal;

	if ( fVal == 0.0 )
		return loVal;
	
	return loVal+1;
}

float VectorDistance(Vector &vec)
{
	return (float)sqrt(((vec.x*vec.x) + (vec.y*vec.y) + (vec.z*vec.z)));
}

// Testing the three types of "entity" for nullity
bool FNullEnt(const edict_t* pent)
{ 
	return pent == NULL || ENTINDEX((edict_t*)pent) == 0; 
}

/**
 * Searches for a named Server Class.
 *
 * @param name		Name of the top-level server class.
 * @return 		Server class matching the name, or NULL if none found.
 */
ServerClass *UTIL_FindServerClass(const char *name)
{
	ServerClass *pClass = servergamedll->GetAllServerClasses();
	while (pClass)
	{
		if (strcmp(pClass->m_pNetworkName, name) == 0)
		{
			return pClass;
		}
		pClass = pClass->m_pNext;
	}
	return NULL;
}

/**
 * Recursively looks through a send table for a given named property.
 *
 * @param pTable	Send table to browse.
 * @param name		Property to search for.
 * @return 		SendProp pointer on success, NULL on failure.
 */
SendProp *UTIL_FindSendProp(SendTable *pTable, const char *name)
{
	int count = pTable->GetNumProps();
	//SendTable *pTable;
	SendProp *pProp;
	for (int i=0; i<count; i++)
	{
		pProp = pTable->GetProp(i);
		if (strcmp(pProp->GetName(), name) == 0)
		{
			return pProp;
		}
		if (pProp->GetDataTable())
		{
			if ((pProp=UTIL_FindSendProp(pProp->GetDataTable(), name)) != NULL)
			{
				return pProp;
			}
		}
	}
 
	return NULL;
}
/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

struct sm_sendprop_info_t
{
	SendProp *prop;					/**< Property instance. */
	unsigned int actual_offset;		/**< Actual computed offset. */
};

bool UTIL_FindInSendTable(SendTable *pTable, 
						  const char *name,
						  sm_sendprop_info_t *info,
						  unsigned int offset)
{
	const char *pname;
	int props = pTable->GetNumProps();
	SendProp *prop;

	for (int i=0; i<props; i++)
	{
		prop = pTable->GetProp(i);
		pname = prop->GetName();
		if (pname && strcmp(name, pname) == 0)
		{
			info->prop = prop;
			info->actual_offset = offset + info->prop->GetOffset();
			return true;
		}
		if (prop->GetDataTable())
		{
			if (UTIL_FindInSendTable(prop->GetDataTable(), 
				name,
				info,
				offset + prop->GetOffset())
				)
			{
				return true;
			}
		}
	}

	return false;
}

bool UTIL_FindSendPropInfo(ServerClass *pInfo, const char *szType, unsigned int *offset)
{
	if ( !pInfo )
	{
		return false;
	}

	sm_sendprop_info_t temp_info;

	if (!UTIL_FindInSendTable(pInfo->m_pTable, szType, &temp_info, 0))
	{
		return false;
	}

	*offset = temp_info.actual_offset;

	return true;
}

int CClassInterface ::getScore (edict_t *edict)
{
	static unsigned int offset = 0;
	edict_t *res;
 
	if (!offset)
		offset = findOffset("m_iTotalScore","CTFPlayerResource");
	
	if (!offset)
		return 0;

	res = CTeamFortress2Mod::findResourceEntity();

	IServerUnknown *pUnknown = (IServerUnknown *)res->GetUnknown();

	if (!pUnknown)
	{
		return 0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int *)((char *)pEntity + (offset + (ENTINDEX(edict)*4)));

}

int CClassInterface :: getEffects ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_fEffects","CBaseEntity");
	
	if (!offset)
		return 0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int *)((char *)pEntity + offset);
}

void CClassInterface ::test()
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("ProcessUsercmds","CServerGameClients");
	
	if (!offset)
		return;

	//return offset;
}

int CClassInterface :: isTeleporterMode ( edict_t *edict, eTeleMode mode )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_iObjectMode","CObjectTeleporter");
	
	if (!offset)
		return 0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return (*(int *)((char *)pEntity + offset))==(int)mode;
	
}

bool CClassInterface :: getMedigunHealing ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_bHealing","CWeaponMedigun");
	
	if (!offset)
		return 0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(bool *)((char *)pEntity + offset);
}

int CClassInterface :: getMedigunTarget ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_hHealingTarget","CWeaponMedigun");
	
	if (!offset)
		return 0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return NULL;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int*)((char *)pEntity + offset);
}

void CClassInterface :: setTickBase ( edict_t *edict, int tick )
{
	static unsigned int offset = 0;
	int *tickbase;
 
	if (!offset)
		offset = findOffset("m_nTickBase","CBasePlayer");
	
	if (!offset)
		return;

	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	tickbase = (int *)((char *)pEntity + offset);

	*tickbase = tick;
}

int CClassInterface :: getFlags ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_fFlags","CBaseEntity");
	
	if (!offset)
		return 0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int *)((char *)pEntity + offset);
}

int CClassInterface :: getTeam ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_iTeamNum","CBaseEntity");
	
	if (!offset)
		return 0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int *)((char *)pEntity + offset);
}

float CClassInterface :: getHealth ( edict_t *edict )
{
	static unsigned int offset1 = 0;

	if (!offset1)
		offset1 = findOffset("m_iHealth","CBasePlayer");

	if (!offset1)
		return 1.0f;

	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 1.0f;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return (float)(*(int *)((char *)pEntity + offset1));
}

int CClassInterface :: getTF2Class ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_PlayerClass","CTFPlayer")+4;
	
	if (!offset)
		return 0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int *)((char *)pEntity + offset);
}

Vector *CClassInterface :: getVelocity ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_vecAbsVelocity","CBaseEntity");
	
	if (!offset)
		return NULL;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return NULL;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return (Vector *)((char *)pEntity + offset);
}

int *CClassInterface :: getAmmoList ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_iAmmo","CBasePlayer");
	
	if (!offset)
		return NULL;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return NULL;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return (int *)((char *)pEntity + offset);
}

unsigned int CClassInterface :: findOffset(const char *szType,const char *szClass)
{
	unsigned int offset = 0;
	ServerClass *sc = UTIL_FindServerClass(szClass);

	
	//SendProp *pProp = UTIL_FindSendProp(sc->m_pTable, szType);

	if ( sc )
	{
		if ( UTIL_FindSendPropInfo(sc,szType,&offset) )
			return offset;
		else
			return 0;
	}
	//if ( pProp )
	//	return pProp->GetOffset();

	return 0;
}

int CClassInterface :: getTF2NumHealers ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_nNumHealers","CTFPlayer")+4;
	
	if (!offset)
		return NULL;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return NULL;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int *)((char *)pEntity + offset);
}

float CClassInterface :: getTF2SpyCloakMeter ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_flCloakMeter","CTFPlayer")+4;
	
	if (!offset)
		return 0.0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 0.0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(float *)((char *)pEntity + offset);
}

bool CClassInterface :: getTF2SpyDisguised( edict_t *edict, int *_class, int *_team, int *_index, int *_health )
{
	static unsigned int offset[4] = {0,0,0,0};
 
	if (!offset[0])
		offset[0] = findOffset("m_nDisguiseTeam","CTFPlayer");
	if (!offset[1])
		offset[1] = findOffset("m_nDisguiseClass","CTFPlayer");
	if (!offset[2])
		offset[2] = findOffset("m_iDisguiseTargetIndex","CTFPlayer");
	if (!offset[3])
		offset[3] = findOffset("m_iDisguiseHealth","CTFPlayer");
	
	if (!offset[0] || !offset[1] || !offset[2] || !offset[3])
		return false;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return false;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	*_team = *(int *)((char *)pEntity + offset[0]);
	*_class = *(int *)((char *)pEntity + offset[1]);
	*_index = *(int *)((char *)pEntity + offset[2]);
	*_health = *(int *)((char *)pEntity + offset[3]);

	 return true;
}

int CClassInterface :: getTF2Conditions ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_nPlayerCond","CTFPlayer");
	
	if (!offset)
		return 0;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 0;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int *)((char *)pEntity + offset);
}

