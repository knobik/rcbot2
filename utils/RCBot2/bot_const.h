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
#ifndef __BOT_CONST_H__
#define __BOT_CONST_H__

#include "shareddefs.h"

#define BOT_DEFAULT_FOV 75.0f

#define to_lower(a) (((a)>='A')&&((a)<='Z'))?('a'+((a)-'A')):(a)
#define strlow(str) { int len = strlen(str); int i;	for ( i = 0; i < len; i ++ ) { str[i] = to_lower(str[i]); } }
#define round(a) (((a-(int)a) >= 0.5) ? ((int)a+1) : ((int)a))
//#define RANDOM_INT(min,max) (min + round(((float)rand()/RAND_MAX)*(float)(max-min)))
//#define RANDOM_FLOAT(min,max) (min + ((float)rand()/RAND_MAX)*(float)(max-min))

#define DEFAULT_BOT_NAME "RCBot"

#define BOT_WPT_TOUCH_DIST 72 // distance for bot to touch waypoint

#define BOT_DEBUG_GAME_EVENT 1
#define BOT_DEBUG_NAV 2
#define BOT_DEBUG_SPEED 4
#define BOT_DEBUG_VIS 8
#define BOT_DEBUG_TASK 16
#define BOT_DEBUG_BUTTONS 32
#define BOT_DEBUG_USERCMD 64
#define BOT_DEBUG_UTIL 128
#define BOT_DEBUG_PROFILE 256

typedef enum
{
	LOOK_NONE = 0,
	LOOK_VECTOR,
	LOOK_WAYPOINT,
	LOOK_AROUND,
	LOOK_ENEMY,
	LOOK_LAST_ENEMY,
	LOOK_HURT_ORIGIN,
	LOOK_EDICT,
	LOOK_GROUND,
	LOOK_SNIPE,
	LOOK_WAYPOINT_AIM,
	LOOK_BUILD,
	LOOK_NOISE
}eLookTask;

typedef enum
{
	TELE_ENTRANCE = 0,
	TELE_EXIT
}eTeleMode;

#define BOT_CONFIG_FOLDER "config"
#define BOT_MOD_FILE "bot_mods"
#define BOT_ACCESS_CLIENT_FILE "accessclients"
#define BOT_PROFILE_FOLDER "profiles"
#define BOT_WAYPOINT_FOLDER "waypoints"
#define BOT_CONFIG_EXTENSION "ini"
#define BOT_SCRIPT_FOLDER "scripts"
#define BOT_SCRIPT_EXTENSION "rcs"
#define BOT_WAYPOINT_EXTENSION "rcw" // extension for waypoint files
#define BOT_WAYPOINT_FILE_TYPE "RCBot2\0" // for waypoint file header

#define BOT_TAG "[RCBot] " // for printing messages

typedef enum
{
	MOD_UNSUPPORTED = 0,
	MOD_HLDM2,
	MOD_CSS,
	MOD_FF,
	MOD_TF2,
	MOD_SVENCOOP2,
	MOD_TIMCOOP,
	MOD_HL1DMSRC,
	MOD_NS2,
	MOD_CUSTOM,
	MOD_ANY,
	MOD_MAX
}eModId;

#define BOT_JUMP_HEIGHT 45

#undef INDEXENT
#define INDEXENT(iEdictNum) engine->PEntityOfEntIndex(iEdictNum)

#undef ENTINDEX
#define ENTINDEX(pEdict) engine->IndexOfEdict(pEdict)

#define BOT_NAME "RCBot"
#ifdef __linux__
#define BOT_VER "TF2 0.56 Beta linux 146 (BUILD " __DATE__ "-" __TIME__ ")" //bir3yk
#else
#define BOT_VER "TF2 0.56 Beta (BUILD " ## __DATE__ ## "-" ## __TIME__ ## ")"
#endif
#define BOT_NAME_VER "RCbot version"
#define BOT_VER_CVAR "rcbot_ver"
#define BOT_FOLDER "rcbot2"

typedef enum
{
	BOT_FUNC_FAIL = 0,
    BOT_FUNC_CONTINUE,
	BOT_FUNC_COMPLETE,
}eBotFuncState;

//////////////////////////////////
#define CONDITION_ENEMY_OBSCURED 1
#define CONDITION_NO_WEAPON		 2
#define CONDITION_OUT_OF_AMMO	 4 
#define CONDITION_SEE_CUR_ENEMY  8
#define CONDITION_ENEMY_DEAD     16
#define CONDITION_SEE_WAYPOINT   32
#define CONDITION_NEED_AMMO		 64
#define CONDITION_NEED_HEALTH	 128
#define CONDITION_SEE_LOOK_VECTOR 256
#define CONDITION_POINT_CAPTURED 512
////////////////////////

///////////////////////
typedef enum 
{
	STATE_IDLE = 0,
	STATE_RUNNING,
	STATE_FAIL,
	STATE_COMPLETE
}eTaskState;
////////////////////
#endif