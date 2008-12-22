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
#include "bot_fortress.h"
#include "bot_buttons.h"
#include "bot_globals.h"
#include "bot_profile.h"

#include "vstdlib/random.h" // for random functions


void CBotFortress :: init (bool bVarInit)
{
	CBot::init(bVarInit);
}

void CBotFortress :: setup ()
{
	CBot::setup();
}

bool CBotFortress :: startGame()
{
	if ( m_pPlayerInfo->GetTeamIndex() == 0 )
	{
		selectTeam();
	}
	else if ( (m_pPlayerInfo->GetWeaponName() == NULL) || (*(m_pPlayerInfo->GetWeaponName()) == 0) )
	{
		selectClass();
	}
	else
		return true;

	return false;
}

void CBotFortress :: killed ( edict_t *pVictim )
{
 return;
}

void CBotFortress :: died ( edict_t *pKiller )
{
	spawnInit();

	if ( RandomInt(0,1) )
		m_pButtons->attack();
}

void CBotFortress :: spawnInit ()
{
	CBot::spawnInit();
}

bool CBotFortress :: isEnemy ( edict_t *pEdict )
{
	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	return true;	
}


void CBotFortress :: modThink ()
{

}


void CBotFortress :: selectTeam ()
{
	char buffer[32];

	int team = RandomInt(1,2);

	sprintf(buffer,"jointeam %d",team);

	helpers->ClientCommand(m_pEdict,buffer);
}

void CBotFortress :: selectClass ()
{
	char buffer[32];

	TF_Class _class = (TF_Class)RandomInt(1,9);

	if ( _class == TF_CLASS_SCOUT )
		sprintf(buffer,"joinclass scout");
	else if ( _class == TF_CLASS_ENGINEER )
		sprintf(buffer,"joinclass engineer");
	else if ( _class == TF_CLASS_DEMOMAN )
		sprintf(buffer,"joinclass demoman");
	else if ( _class == TF_CLASS_SOLDIER )
		sprintf(buffer,"joinclass soldier");
	else if ( _class == TF_CLASS_HWGUY )
		sprintf(buffer,"joinclass heavyweapons");
	else if ( _class == TF_CLASS_MEDIC )
		sprintf(buffer,"joinclass medic");
	else if ( _class == TF_CLASS_SPY )
		sprintf(buffer,"joinclass spy");
	else if ( _class == TF_CLASS_PYRO )
		sprintf(buffer,"joinclass pyro");
	else
		sprintf(buffer,"joinclass sniper");

	helpers->ClientCommand(m_pEdict,buffer);
}

/////////////////////////////////////////////////////////////////////////////
// TEAM FORTRESS 2


void CBotTF2 :: taunt ()
{
	helpers->ClientCommand(m_pEdict,"taunt");
}

void CBotTF2 :: engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd )
{
	char buffer[16];
	char cmd[256];

	if ( iEngiCmd == ENGI_BUILD )
		strcpy(buffer,"build");
	else
		strcpy(buffer,"destroy");

	sprintf(cmd,"%s %d",buffer,iBuilding);

	helpers->ClientCommand(m_pEdict,cmd);
}

void CBotTF2 :: spyDisguise ( int iTeam, int iClass )
{
	char cmd[256];

	sprintf(cmd,"disguise %d %d",iClass,iTeam);

	helpers->ClientCommand(m_pEdict,cmd);
}

bool CBotTF2 :: hasEngineerBuilt ( eEngiBuild iBuilding )
{
	switch ( iBuilding )
	{
	case ENGI_SENTRY:
		return false; // TODO
		break;
	case ENGI_DISP:
		return false; // TODO
		break;
	case ENGI_ENTRANCE:
		return false; // TODO
		break;
	case ENGI_EXIT:
		return false; // TODO
		break;
	}

	return false;
}

void CBotTF2 :: modThink ()
{
// mod specific think code here
}

bool CBotTF2 :: isEnemy ( edict_t *pEdict )
{
	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	return true;	
}

/////////////////////////////////////////////////////////////////////////
// FORTRESS FOREVER


void CBotFF :: modThink ()
{
// mod specific think code here
}

bool CBotFF :: isEnemy ( edict_t *pEdict )
{
	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( CBotGlobals::getTeam(pEdict) == getTeam() )
		return false;

	return true;	
}
