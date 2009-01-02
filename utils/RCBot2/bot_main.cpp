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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ICvar *s_pCVar;

ConVar bot_attack( "rcbot_flipout", "0", 0, "Rcbots all attack" );

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

//---------------------------------------------------------------------------------
// Purpose: an example of how to implement a new command
//---------------------------------------------------------------------------------
static ConVar empty_cvar(BOT_VER_CVAR, BOT_VER, 0, BOT_NAME_VER);

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

#define LOAD_INTERFACE(var,type,version) if ( (var = (type*)interfaceFactory(version,NULL)) == NULL ) { Warning("[RCBOT] Cannot open interface "## #version ##" "## #type ##" "## #var ##"\n"); return false; } else { Msg("[RCBOT] Found interface "## #version ##" "## #type ##" "## #var ## "\n"); }
#define LOAD_GAME_SERVER_INTERFACE(var,type,version) if ( (var = (type*)gameServerFactory(version,NULL)) == NULL ) { Warning("[RCBOT] Cannot open game server interface "## #version ##" "## #type ##" "## #var ##"\n"); return false; } else { Msg("[RCBOT] Found interface "## #version ##" "## #type ##" "## #var ## "\n"); }

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool CRCBotPlugin::Load( CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
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

	LOAD_INTERFACE(debugoverlay,IVDebugOverlay,VDEBUG_OVERLAY_INTERFACE_VERSION);
	LOAD_INTERFACE(gameeventmanager,IGameEventManager2,INTERFACEVERSION_GAMEEVENTSMANAGER2)
	LOAD_INTERFACE(gameeventmanager1,IGameEventManager,INTERFACEVERSION_GAMEEVENTSMANAGER)
	LOAD_GAME_SERVER_INTERFACE(servergamedll,IServerGameDLL,INTERFACEVERSION_SERVERGAMEDLL);

	LOAD_GAME_SERVER_INTERFACE(gameclients,IServerGameClients,INTERFACEVERSION_SERVERGAMECLIENTS);

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	ConVar_Register( 0 );
	//InitCVars( interfaceFactory ); // register any cvars we have defined

	srand( (unsigned)time(NULL) );  // initialize the random seed

	eventListener2 = new CRCBotEventListener();

	// Initialize bot variables
	CBotProfiles::setupProfiles();
	CBotGlobals::gameStart();	
	//CBotEvents::setupEvents();
	CWaypointTypes::setup();
	CWaypoints::setupVisibility();

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

	CWaypoints::init();
	CWaypoints::load();

	CBotGlobals::setMapRunning(true);
	
	
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

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CRCBotPlugin::GameFrame( bool simulating )
{
	if ( simulating && CBotGlobals::IsMapRunning() )
	{
		CBots::botThink();
		CClients::clientThink();	

		if ( CWaypoints::getVisiblity()->needToWorkVisibility() )
		{
			CWaypoints::getVisiblity()->workVisibility();
		}
	}
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CRCBotPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
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
	}
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
/*
class CPluginConVarAccessor : public IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase( ConCommandBase *pCommand )
	{
		pCommand->AddFlags( FCVAR_PLUGIN );

		// Unlink from plugin only list
		pCommand->SetNext( 0 );

		// Link to engine's list instead
		s_pCVar->RegisterConCommand( pCommand );
		return true;
	}

};

CPluginConVarAccessor g_ConVarAccessor;

void InitCVars( CreateInterfaceFn cvarFactory )
{
	s_pCVar = (ICvar*)cvarFactory( CVAR_INTERFACE_VERSION, NULL);//VENGINE_CVAR_INTERFACE_VERSION, NULL );
	if ( s_pCVar )
	{		
		g_ConVarAccessor.RegisterConCommandBase(&rcbotd_command);
		ConVar_Register(FCVAR_PLUGIN,&g_ConVarAccessor);

	    ConCommandBaseMgr::OneTimeInit( &g_ConVarAccessor );
	}
}*/



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

int offsets[T_OFFSETMAX];

char *offsetnames[3] = 
{
	"m_iHealth",
	"m_iTeam",
	"m_iAmmo"
};


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

int CClassInterface :: getHealth ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_iHealth","CBaseEntity");
	
	if (!offset)
		return 100;
 
	IServerUnknown *pUnknown = (IServerUnknown *)edict->GetUnknown();

	if (!pUnknown)
	{
		return 100;
	}
 
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();

	return *(int *)((char *)pEntity + offset);
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
	ServerClass *sc = UTIL_FindServerClass(szClass);
	SendProp *pProp = UTIL_FindSendProp(sc->m_pTable, szType);

	if ( pProp )
		return pProp->GetOffset();

	return 0;
}

int CClassInterface :: getTF2NumHealers ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_nNumHealers","CTFPlayer");
	
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

int CClassInterface :: getTF2Conditions ( edict_t *edict )
{
	static unsigned int offset = 0;
 
	if (!offset)
		offset = findOffset("m_nPlayerCond","CTFPlayer");
	
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

