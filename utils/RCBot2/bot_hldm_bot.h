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
#ifndef __HLDM_RCBOT_H__
#define __HLDM_RCBOT_H__

#include "bot_utility.h"

// bot for HLDM
class CHLDMBot : public CBot
{
public:

	bool isHLDM () { return true; }

	void modThink ();

	void init ();
	void setup ();

	bool startGame ();

	void died ( edict_t *pKiller );
	void killed ( edict_t *pVictim );

	void spawnInit ();

	bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	void getTasks (unsigned int iIgnore=0);
	bool executeAction ( eBotAction iAction );

	float getArmorPercent () { return (0.01f * m_pPlayerInfo->GetArmorValue()); }

	void setVisible ( edict_t *pEntity, bool bVisible );

	virtual unsigned int maxEntityIndex ( ) { return gpGlobals->maxEntities; }

private:
	// blah blah
	MyEHandle m_NearestPhysObj;
	float m_flSprintTime;
	MyEHandle m_pHealthKit;
	MyEHandle m_pAmmoKit;
	MyEHandle m_pBattery;
	edict_t *m_pCurrentWeapon;

	CBaseHandle *m_Weapons;
	CBaseHandle *m_hCurrentWeapon;

	int m_iClip1;
	int m_iClip2;
};

#endif