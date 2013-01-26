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

#include "bot_wpt_dist.h"

#include "bot_configfile.h"

#include "bot_getprop.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ICvar *s_pCVar;

ConVar bot_general_difficulty("rcbot_skill","0.5",0,"General difficulty of the bots. 0.5 = stock, < 0.5 easier, > 0.5 = harder");
ConVar bot_sv_cheats_auto("rcbot_sv_cheats_auto","0",0,"automatically put sv_cheats on and off for when adding bots only");
ConVar bot_visrevs_clients("rcbot_visrevs_clients","4",0,"how many revs the bot searches for visible players and enemies, lower to reduce cpu usage");
ConVar bot_spyknifefov("rcbot_spyknifefov","80",0,"the FOV from the enemy that spies must backstab from");
ConVar bot_visrevs("rcbot_visrevs","9",0,"how many revs the bot searches for visible monsters, lower to reduce cpu usage min:5");
ConVar bot_pathrevs("rcbot_pathrevs","40",0,"how many revs the bot searches for a path each frame, lower to reduce cpu usage, but causes bots to stand still more");
ConVar bot_command("rcbot_cmd","",0,"issues a command to all bots");
ConVar bot_attack( "rcbot_flipout", "0", 0, "Rcbots all attack" );
ConVar bot_scoutdj( "rcbot_scoutdj", "0.28", 0, "time scout uses to double jump" );
ConVar bot_anglespeed( "rcbot_anglespeed", "0.19", 0, "smaller number will make bots turn slower (1 = instant turn but may overshoot)" );
ConVar bot_stop( "rcbot_stop", "0", 0, "Make bots stop thinking!");
ConVar bot_waypointpathdist("rcbot_wpt_pathdist","400",0,"Length for waypoints to automatically add paths at");
ConVar bot_rj("rcbot_rj","0.01",0,"time for soldier to fire rocket after jumping");
ConVar bot_defrate("rcbot_defrate","0.24",0,"rate for bots to defend");
ConVar bot_beliefmulti("rcbot_beliefmulti","20.0",0,"multiplier for increasing bot belief");
ConVar bot_belief_fade("rcbot_belief_fade","0.75",0,"the multiplayer rate bot belief decreases");
ConVar bot_change_class("rcbot_change_classes","0",0,"bots change classes at random intervals");
ConVar bot_use_vc_commands("rcbot_voice_cmds","1",0,"bots use voice commands e.g. medic/spy etc");
ConVar bot_use_disp_dist("rcbot_disp_dist","800.0",0,"distance that bots will go back to use a dispenser");
ConVar bot_max_cc_time("rcbot_max_cc_time","240",0,"maximum time for bots to consider changing class <seconds>");
ConVar bot_min_cc_time("rcbot_min_cc_time","60",0,"minimum time for bots to consider changing class <seconds>");
ConVar bot_avoid_radius("rcbot_avoid_radius","80",0,"radius in units for bots to avoid things");
ConVar bot_avoid_strength("rcbot_avoid_strength","100",0,"strength of avoidance (0 = disable)");
ConVar bot_messaround("rcbot_messaround","1",0,"bots mess around at start up");
ConVar bot_heavyaimoffset("rcbot_heavyaimoffset","0.1",0,"fraction of how much the heavy aims at a diagonal offset");
ConVar bot_aimsmoothing("rcbot_aimsmooting","1",0,"(0 = no smoothing)");
ConVar bot_bossattackfactor("rcbot_bossattackfactor","1.0",0,"the higher the more often the bots will shoot the boss");
ConVar rcbot_enemyshootfov("rcbot_enemyshootfov","0.6",0,"the fov dot product before the bot shoots an enemy 0.7 = 45 degrees");
ConVar rcbot_enemyshoot_gravgun_fov("rcbot_enemyshoot_gravgun_fov","0.98",0,"the fov dot product before the bot shoots an enemy 0.98 = 11 degrees");
ConVar rcbot_wptplace_width("rcbot_wpt_width","48",0,"width of the player, automatic paths won't connect unless there is enough space for a player");
ConVar rcbot_wpt_autoradius("rcbot_wpt_autoradius","0",0,"every waypoint added is given this radius, 0 = no radius");
ConVar rcbot_wpt_autotype("rcbot_wpt_autotype","1",0,"If 1, types will be automatically added to waypoints when they are added (only for resupply/health/capture/flag etc)\nIf 2: types will autoamtically be added even if the waypoint is cut/paste");
ConVar rcbot_move_sentry_time("rcbot_move_sentry_time","120",0,"seconds for bots to start thinking about moving sentries");
ConVar rcbot_move_sentry_kpm("rcbot_move_sentry_kpm","1",0,"kpm = kills per minute, if less than this, bots will think about moving the sentry");
ConVar rcbot_smoke_time("rcbot_smoke_time","10",0,"seconds a smoke grenade stays active");
ConVar rcbot_move_disp_time("rcbot_move_disp_time","120",0,"seconds for bots to start thinking about moving dispensers");
ConVar rcbot_move_disp_healamount("rcbot_move_disp_healamount","100",0,"if dispenser heals less than this per minute, bot will move the disp");
ConVar rcbot_demo_runup_dist("rcbot_demo_runup","99.0",0,"distance the demo bot will take to run up for a pipe jump");
ConVar rcbot_demo_jump("rcbot_enable_pipejump","1",0,"Enable experimental pipe jumping at rocket jump waypoints");
ConVar rcbot_move_tele_time("rcbot_move_tele_time","120",0,"seconds for bots to start thinking about moving teleporters");
ConVar rcbot_move_tele_tpm("rcbot_move_tele_tpm","1",0,"if no of players teleported per minute is less than this, bot will move the teleport");
ConVar rcbot_tf2_protect_cap_time("rcbot_tf2_prot_cap_time","12.5",0,"time that the bots will spen more attention to the cap point if attacked");
ConVar rcbot_move_dist("rcbot_move_dist","800",0,"minimum distance to move objects to");

ConVar rcbot_move_obj("rcbot_move_obj","1",0,"if 1 rcbot engineers will move objects around");
ConVar rcbot_taunt("rcbot_taunt","1",0,"enable/disable bots taunting");
ConVar rcbot_notarget("rcbot_notarget","0",0,"bots don't shoot the host!");

ConVar rcbot_jump_obst_dist("rcbot_jump_obst_dist","80",0,"the distance from an obstacle the bot will jump");
ConVar rcbot_jump_obst_speed("rcbot_jump_obst_speed","100",0,"the speed of the bot for the bot to jump an obstacle");

ConVar rcbot_melee_only("rcbot_melee_only","0",0,"if 1 bots will only use melee weapons");
ConVar rcbot_debug_iglev("rcbot_debug_iglev","0",0,"bot think ignores functions to test cpu speed");
ConVar rcbot_dont_move("rcbot_dontmove","0",0,"if 1 , bots will all move forward");
ConVar rcbot_runplayercmd("rcbot_runplayer_cmd","416",0,"offset of the PlayerRunCommand function");
ConVar rcbot_runplayercmd_hookonce("rcbot_runplayer_hookonce","1",0,"function will hook only once, if 0 it will unook and rehook after every map");
ConVar rcbot_ladder_offs("rcbot_ladder_offs","38",0,"difference in height for bot to think it has touched the ladder waypoint");
/*CON_COMMAND( enginetime, "get engine time" )
{
	Msg("%f\n",engine->Time());
}
CON_COMMAND( maptime, "get map time" )
{
	Msg("CDODMod::getMapStartTime() = %f\n",CDODMod::getMapStartTime());
	Msg("engine->Time() = %f\n",engine->Time());
	Msg("diff = %f\n",engine->Time() - CDODMod::getMapStartTime());
	Msg("gpGlobals->realtime = %f\n",gpGlobals->realtime);
	Msg("gpGlobals->curtime = %f\n",gpGlobals->curtime);
}*/
//ConVar rcbot_override("rcbot_override","1",0,"if 1 the plugin will override other bots runplayer functions");
ConVar *sv_gravity = NULL;
ConVar *sv_cheats = NULL;//("sv_cheats");
ConVar *mp_teamplay = NULL;

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
//CBaseEntityList * g_pEntityList;
// 
// The plugin is a static singleton that is exported as an interface
//
CRCBotPlugin g_RCBOTServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CRCBotPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_RCBOTServerPlugin );

// Needed to hook virtual tables and member functions
DWORD VirtualTableHook( DWORD* pdwNewInterface, int vtable, DWORD newInterface )
{
    DWORD dwOld, dwStor = 0x0;
#ifndef __linux
    VirtualProtect( &pdwNewInterface[vtable], 4, PAGE_EXECUTE_READWRITE, &dwOld );
#else
	mprotect(&pdwNewInterface[vtable], 4, PROT_EXEC|PROT_READ|PROT_WRITE);
#endif
    dwStor = pdwNewInterface[vtable];
    *(DWORD*)&pdwNewInterface[vtable] = newInterface;
#ifndef __linux
    VirtualProtect(&pdwNewInterface[vtable], 4, dwOld, &dwOld);
#else
	mprotect(&pdwNewInterface[vtable], 4, PROT_EXEC|PROT_READ);
#endif

    return dwStor;
}
// needed to call the original playerruncommand function
void (CBaseEntity::*pPlayerRunCommand)(CUserCmd*, IMoveHelper*) = 0x0;
DWORD* pdwNewInterface = 0;

// PlayerRunCommmand Hook 
// Some Mods have their own puppet bots that run around and override RCBOT if this is not here
// this function overrides the puppet bots movements
void __fastcall nPlayerRunCommand( CBaseEntity *_this, void *unused, CUserCmd* pCmd, IMoveHelper* pMoveHelper)
{
	edict_t *pEdict = servergameents->BaseEntityToEdict(_this);

	static CBot *pBot;

	pBot = CBots::getBotPointer(pEdict);
	
	if ( pBot )
	{
		static CBotCmd *cmd;
		
		cmd = pBot->getUserCMD();

		// put the bot's commands into this move frame
		pCmd->buttons = cmd->buttons;
		pCmd->forwardmove = cmd->forwardmove;
		pCmd->impulse = cmd->impulse;
		pCmd->sidemove = cmd->sidemove;
		pCmd->upmove = cmd->upmove;
		pCmd->viewangles = cmd->viewangles;
		pCmd->weaponselect = cmd->weaponselect;
		pCmd->weaponsubtype = cmd->weaponsubtype;
		pCmd->tick_count = cmd->tick_count;
	}

    (*_this.*pPlayerRunCommand)(pCmd, pMoveHelper);
}

// begin hook
void HookPlayerRunCommand ( edict_t *edict )
{
	if ( CBots::controlBots() ) //rcbot_override.GetBool() )
	{
		CBaseEntity *BasePlayer = (CBaseEntity *)(edict->GetUnknown()->GetBaseEntity());

		if(BasePlayer)
		{
			int vtable = rcbot_runplayercmd.GetInt();
	#ifndef WIN32
			++vtable;
	#endif
	       // hook it
			if ( pPlayerRunCommand == 0x0 )
			{
				pdwNewInterface = ( DWORD* )*( DWORD* )BasePlayer;
				*(DWORD*)&pPlayerRunCommand = VirtualTableHook( pdwNewInterface, vtable, ( DWORD )nPlayerRunCommand );
			}
		}
	}
}

// end hook
void UnhookPlayerRunCommand ()
{
	if ( pPlayerRunCommand && pdwNewInterface )
	{
		int vtable = rcbot_runplayercmd.GetInt();
	#ifndef WIN32
		++vtable;
	#endif
	       
		VirtualTableHook( pdwNewInterface, vtable, *(DWORD*)&pPlayerRunCommand );
		pdwNewInterface = NULL;
		pPlayerRunCommand = NULL;
	}
}


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
	/*
#ifdef _DEBUG
	static char msg[256];

	if ( CClients::clientsDebugging(BOT_DEBUG_EDICTS) )
	{
		sprintf(msg,"%x : %s\n",(int)edict,edict->GetClassName());

		CClients::clientDebugMsg(BOT_DEBUG_EDICTS,msg);
	}
#endif*/
}

void CRCBotPlugin::OnEdictFreed( const edict_t *edict  )
{/*
#ifdef _DEBUG
	static char msg[256];

	if ( CClients::clientsDebugging(BOT_DEBUG_EDICTS) )
	{
		sprintf(msg,"%x : %s\n",(int)edict,edict->GetClassName());

		CClients::clientDebugMsg(BOT_DEBUG_EDICTS,msg);
	}
#endif*/
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

#define FILESYSTEM_INT FILESYSTEM_INTERFACE_VERSION
#define FILESYSTEM_MAXVER 19

#define RCBot_LoadUndefinedInterface(var,type,vername,maxver,minver) { \
	int ver = maxver; \
	char str [256]; \
	char tempver[8]; \
	do{ \
		strcpy(str,vername); \
		sprintf(tempver,"%03d",ver); \
		strcat(str,tempver); \
		ver--; \
		var = (type*)interfaceFactory(str,NULL); \
		Msg("Trying... %s\n",str); \
	}while((var==NULL)&&(ver>minver)); \
    if ( var == NULL ) \
    { \
	Warning("[RCBOT] Cannot open interface "## #vername ##" "## #type ##" "## #var ##" (Max ver: "## #maxver ##") Min ver: ("## #minver ##") \n"); \
		return false; \
	} else { \
	Msg("[RCBOT] Found interface "## #vername ##" "## #type ##" "## #var ##", ver = %03d\n",ver+1); \
	}\
}

#define RCBot_LoadUndefinedGameInterface(var,type,vername,maxver,minver) { \
	int ver = maxver; \
	char str [256]; \
	char tempver[8]; \
	do{ \
		strcpy(str,vername); \
		sprintf(tempver,"%03d",ver); \
		strcat(str,tempver); \
		ver--; \
		var = (type*)gameServerFactory(str,NULL); \
		Msg("Trying... %s\n",str); \
	}while((var==NULL)&&(ver>minver)); \
    if ( var == NULL ) \
    { \
	Warning("[RCBOT] Cannot open interface "## #vername ##" "## #type ##" "## #var ##" (Max ver: "## #maxver ##") Min ver: ("## #minver ##") \n"); \
		return false; \
	} else { \
	Msg("[RCBOT] Found interface "## #vername ##" "## #type ##" "## #var ##", ver = %03d\n",ver+1); \
	}\
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool CRCBotPlugin::Load( CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	extern MTRand_int32 irand;
	// TODO: check rcbot isn't already loaded already

	ConVar *rcbot_instance;

	ConnectTier1Libraries( &interfaceFactory, 1 );
	ConnectTier2Libraries( &interfaceFactory, 1 );

	rcbot_instance = cvar->FindVar("rcbot_ver");

	if ( rcbot_instance != NULL )
	{
		Msg("An instance of RCBOT is already running. Can't load!");
		DisconnectTier2Libraries( );
		DisconnectTier1Libraries( );
		return false;
	}

	LOAD_GAME_SERVER_INTERFACE(playerinfomanager,IPlayerInfoManager,INTERFACEVERSION_PLAYERINFOMANAGER);

	gpGlobals = playerinfomanager->GetGlobalVars();	

	LOAD_INTERFACE(engine,IVEngineServer,INTERFACEVERSION_VENGINESERVER);
	RCBot_LoadUndefinedInterface(filesystem,IFileSystem,"VFileSystem",20,1);

	if ( !CBotGlobals::gameStart() )
		return false;

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

	RCBot_LoadUndefinedGameInterface(servergamedll,IServerGameDLL,"ServerGameDLL",8,2);
	RCBot_LoadUndefinedGameInterface(gameclients,IServerGameClients,"ServerGameClients",4,1);

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	ConVar_Register( 0 );
	//InitCVars( interfaceFactory ); // register any cvars we have defined

	srand( (unsigned)time(NULL) );  // initialize the random seed
	irand.seed( (unsigned)time(NULL) );

	eventListener2 = new CRCBotEventListener();

	// Initialize bot variables
	CBotProfiles::setupProfiles();


	//CBotEvents::setupEvents();
	CWaypointTypes::setup();
	CWaypoints::setupVisibility();

	CBotConfigFile::reset();	
	CBotConfigFile::load();

	CRCBotPlugin::ShowLicense();	

	RandomSeed((unsigned int)time(NULL));

	CClassInterface::init();

	sv_cheats = cvar->FindVar("sv_cheats");
	sv_gravity = cvar->FindVar("sv_gravity");

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
	UnhookPlayerRunCommand();

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

	if ( gameeventmanager1 )
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
	
	mp_teamplay = cvar->FindVar("mp_teamplay");

	if ( mp_teamplay )
		CBotGlobals::setTeamplay(mp_teamplay->GetBool());
	else
		CBotGlobals::setTeamplay(false);

	gameeventmanager1->AddListener( this, true );	

	CBotEvents::setupEvents();

	CBots::mapInit();

	CBotMod *pMod = CBotGlobals::getCurrentMod();
	
	if ( pMod )
		pMod->mapInit();

	CClients::setListenServerClient(NULL);
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
	static CBotMod *currentmod;

	if ( simulating && CBotGlobals::IsMapRunning() )
	{
		CBots::botThink();
		if ( !CBots::controlBots() )
			gameclients->PostClientMessagesSent();
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
		currentmod = CBotGlobals::getCurrentMod();

		currentmod->modFrame();
	}
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CRCBotPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	CClients::initall();
	CWaypointDistances::save();

	if ( !rcbot_runplayercmd_hookonce.GetBool() )
		UnhookPlayerRunCommand();

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
	// this is sorted in CClients::clientDisconnected()
	//CBot *pBot = CBots::getBotPointer(pEntity);
	//pBot->freeMapMemory();

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
