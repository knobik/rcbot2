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
#include "in_buttons.h"
#include "bot.h"
#include "bot_hldm_bot.h"
#include "bot_buttons.h"
#include "bot_globals.h"
#include "bot_profile.h"

#include "bot_mtrand.h"
//#include "vstdlib/random.h" // for random functions

void CHLDMBot :: init ()
{
	CBot::init();
}

void CHLDMBot :: setup ()
{
	CBot::setup();
}


bool CHLDMBot :: startGame ()
{
	return true;
}

void CHLDMBot :: killed ( edict_t *pVictim )
{
	return;
}

void CHLDMBot :: died ( edict_t *pKiller )
{
	spawnInit();

	if ( randomInt(0,1) )
		m_pButtons->attack();
}

void CHLDMBot :: spawnInit ()
{
	CBot::spawnInit();
}

bool CHLDMBot :: isEnemy ( edict_t *pEdict,bool bCheckWeapons )
{
	extern ConVar rcbot_notarget;

	if ( rcbot_notarget.GetBool() && (ENTINDEX(pEdict) == 1) )
		return false;

	if ( pEdict == m_pEdict )
		return false;

	if ( !ENTINDEX(pEdict) || (ENTINDEX(pEdict) > CBotGlobals::maxClients()) )
		return false;

	if ( CBotGlobals::getTeamplayOn() )
	{
		if ( CBotGlobals::getTeam(pEdict) == getTeam() )
			return false;
	}

	return true;	
}

void CHLDMBot :: modThink ()
{
	// dont use physcannon for now... switch to pistol
	if ( m_pPlayerInfo->GetWeaponName() && *(m_pPlayerInfo->GetWeaponName()) )
	{
		if ( !strcmp("weapon_physcannon",m_pPlayerInfo->GetWeaponName()) )
		{			
			m_pController->SetActiveWeapon("weapon_smg1");			
		}
		else if ( !FStrEq(m_pProfile->getWeapon(),"default") && !FStrEq(m_pProfile->getWeapon(),m_pPlayerInfo->GetWeaponName()))
		{
			m_pController->SetActiveWeapon(m_pProfile->getWeapon());
		}
	}

	if ( m_fLastSeeEnemy && ((m_fLastSeeEnemy + 5.0)<engine->Time()) )
	{
		m_fLastSeeEnemy = 0;
		m_pButtons->tap(IN_RELOAD);
	}
}