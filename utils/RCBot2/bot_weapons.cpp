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

#include <queue>
using namespace std;

#include "bot.h"
#include "bot_globals.h"
#include "bot_weapons.h"

const char *g_szTF2Weapons[] =
{
"tf_weapon_bat",
"tf_weapon_bonesaw",
"tf_weapon_bottle",
"tf_weapon_builder",
"tf_weapon_club",
"tf_weapon_fireaxe",
"tf_weapon_fists",
"tf_weapon_flamethrower",
"tf_weapon_grenadelauncher",
"tf_weapon_invis",
"tf_weapon_knife",
"tf_weapon_medigun",
"tf_weapon_minigun",
"tf_weapon_objectselection",
"tf_weapon_pda_engineer_build",
"tf_weapon_pda_engineer_destroy",
"tf_weapon_pda_spy",
"tf_weapon_pipebomblauncher",
"tf_weapon_pistol",
"tf_weapon_pistol_scout",
"tf_weapon_revolver",
"tf_weapon_rocketlauncher",
"tf_weapon_scattergun",
"tf_weapon_shotgun_hwg",
"tf_weapon_shotgun_primary",
"tf_weapon_shotgun_pyro",
"tf_weapon_shotgun_soldier",
"tf_weapon_shovel",
"tf_weapon_smg",
"tf_weapon_sniperrifle",
"tf_weapon_syringegun_medic",
"tf_weapon_wrench"
};

int m_TF2AmmoIndices[] =
{
	0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,2,2,2,2,1,1,2,1,2,2,0,2,1,1,3
};


TF2WeaponsData_t TF2Weaps[] =
{
/*
	slot, id , weapon name, flags, min dist, max dist, ammo index, preference
*/
	{3,TF2_WEAPON_BAT,				g_szTF2Weapons[0],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,180,m_TF2AmmoIndices[0],1},
	{3,TF2_WEAPON_BONESAW,			g_szTF2Weapons[1],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,180,m_TF2AmmoIndices[1],1},
	{3,TF2_WEAPON_BOTTLE,				g_szTF2Weapons[2],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,180,m_TF2AmmoIndices[2],1},
	{0,TF2_WEAPON_BUILDER,			g_szTF2Weapons[3],	WEAP_FL_NONE,0,100,m_TF2AmmoIndices[3],1},
	{3,TF2_WEAPON_CLUB,				g_szTF2Weapons[4],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,180,m_TF2AmmoIndices[4],1},
	{3,TF2_WEAPON_FIREAXE,			g_szTF2Weapons[5],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,180,m_TF2AmmoIndices[5],1},
	{3,TF2_WEAPON_FISTS,				g_szTF2Weapons[6],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,180,m_TF2AmmoIndices[6],1},
	{1,TF2_WEAPON_FLAMETHROWER,		g_szTF2Weapons[7],	WEAP_FL_DEFLECTROCKETS|WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_HOLDATTACK|WEAP_FL_SPECIAL,0,400,m_TF2AmmoIndices[7],3},
	{1,TF2_WEAPON_GRENADELAUNCHER,	g_szTF2Weapons[8],	WEAP_FL_PRIM_ATTACK|WEAP_FL_EXPLOSIVE|WEAP_FL_UNDERWATER,80,1200,m_TF2AmmoIndices[8],2},
	{0,TF2_WEAPON_INVIS,				g_szTF2Weapons[9],	WEAP_FL_NONE,0,100,m_TF2AmmoIndices[9],1},
	{3,TF2_WEAPON_KNIFE,				g_szTF2Weapons[10],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,220,m_TF2AmmoIndices[10],2},
	{2,TF2_WEAPON_MEDIGUN,			g_szTF2Weapons[11],	WEAP_FL_NONE,0,100,m_TF2AmmoIndices[11],1},
	{1,TF2_WEAPON_MINIGUN,			g_szTF2Weapons[12],	WEAP_FL_PRIM_ATTACK|WEAP_FL_HOLDATTACK,0,1800,m_TF2AmmoIndices[12],2},
	{0,TF2_WEAPON_OBJECTSSELECTION,	g_szTF2Weapons[13],	WEAP_FL_NONE,0,100,m_TF2AmmoIndices[13],1},
	{0,TF2_WEAPON_PDA_ENGI_BUILD,		g_szTF2Weapons[14],	WEAP_FL_NONE,0,100,m_TF2AmmoIndices[14],1},
	{0,TF2_WEAPON_PDA_ENGI_DESTROY,	g_szTF2Weapons[15],	WEAP_FL_NONE,0,100,m_TF2AmmoIndices[15],1},
	{0,TF2_WEAPON_PDA_SPY,			g_szTF2Weapons[16],	WEAP_FL_NONE,0,100,m_TF2AmmoIndices[16],1},
	{2,TF2_WEAPON_PIPEBOMBS,			g_szTF2Weapons[17],	WEAP_FL_NONE,0,1000,m_TF2AmmoIndices[17],1},
	{2,TF2_WEAPON_PISTOL,				g_szTF2Weapons[18],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,2000,m_TF2AmmoIndices[18],1},
	{2,TF2_WEAPON_PISTOL_SCOUT,		g_szTF2Weapons[19],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,1800,m_TF2AmmoIndices[19],2},
	{1,TF2_WEAPON_REVOLVER,			g_szTF2Weapons[20],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,1400,m_TF2AmmoIndices[20],1},
	{1,TF2_WEAPON_ROCKETLAUNCHER,		g_szTF2Weapons[21],	WEAP_FL_PRIM_ATTACK|WEAP_FL_EXPLOSIVE|WEAP_FL_UNDERWATER,300,4000,m_TF2AmmoIndices[21],3},
	{1,TF2_WEAPON_SCATTERGUN,			g_szTF2Weapons[22],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,800,m_TF2AmmoIndices[22],3},
	{2,TF2_WEAPON_SHOTGUN_HWG,		g_szTF2Weapons[23],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,800,m_TF2AmmoIndices[23],2},
	{1,TF2_WEAPON_SHOTGUN_PRIMARY,	g_szTF2Weapons[24],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,800,m_TF2AmmoIndices[24],2},
	{2,TF2_WEAPON_SHOTGUN_PYRO,		g_szTF2Weapons[25],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,800,m_TF2AmmoIndices[25],2},
	{2,TF2_WEAPON_SHOTGUN_SOLDIER,	g_szTF2Weapons[26],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,500,m_TF2AmmoIndices[26],2},
	{3,TF2_WEAPON_SHOVEL,				g_szTF2Weapons[27],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,180,m_TF2AmmoIndices[27],1},
	{2,TF2_WEAPON_SMG,				g_szTF2Weapons[28],	WEAP_FL_KILLPIPEBOMBS|WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,1000,m_TF2AmmoIndices[28],2},
	{1,TF2_WEAPON_SNIPERRIFLE,		g_szTF2Weapons[29],	WEAP_FL_PRIM_ATTACK,1000,4000,m_TF2AmmoIndices[29],3},
	{1,TF2_WEAPON_SYRINGEGUN,			g_szTF2Weapons[30],	WEAP_FL_PRIM_ATTACK|WEAP_FL_UNDERWATER,0,1000,m_TF2AmmoIndices[30],2},
	{3,TF2_WEAPON_WRENCH,				g_szTF2Weapons[31],	WEAP_FL_PRIM_ATTACK|WEAP_FL_MELEE|WEAP_FL_UNDERWATER,0,180,m_TF2AmmoIndices[31],1}
};


// static init (all weapons in game)
vector<CWeapon*> CWeapons :: m_theWeapons;

int CBotWeapon :: getAmmo (CBot *pBot)
{
	return pBot->getAmmo(m_pWeaponInfo->getAmmoIndex());
}

// Bot Weapons
CBotWeapons :: CBotWeapons ( CBot *pBot ) 
{
	m_pBot = pBot;

	for ( int i = 0; i < MAX_WEAPONS; i ++ )
	{
		// find weapon info from weapon id
		m_theWeapons[i].setWeapon(CWeapons::getWeapon(i));
	}
}

CBotWeapon *CBotWeapons :: getBestWeapon ( edict_t *pEnemy )
{
	CBotWeapon *m_theBestWeapon = NULL;
	CBotWeapon *m_FallbackMelee = NULL;
	int iBestPreference = 0;
	Vector vEnemyOrigin;

	if ( !pEnemy )
		return NULL;

	vEnemyOrigin = CBotGlobals::entityOrigin(pEnemy);

	float flDist = 0;

	flDist = m_pBot->distanceFrom(vEnemyOrigin);

	for ( unsigned int i = 0; i < MAX_WEAPONS; i ++ )
	{
		CBotWeapon *pWeapon = &(m_theWeapons[i]);

		if ( !pWeapon )
			continue;

		if ( !pWeapon->hasWeapon() )
			continue;

		if ( !pWeapon->isMelee() || pWeapon->isSpecial() )
		{
			if ( pWeapon->outOfAmmo(m_pBot) )
				continue;
		}

		if ( !pWeapon->canAttack() )
			continue;

		if ( m_pBot->isUnderWater() && !pWeapon->canUseUnderWater() )
			continue;

		if ( !pWeapon->primaryInRange(flDist) )
		{
			if ( pWeapon->isMelee() && !pWeapon->isSpecial() )
				m_FallbackMelee = pWeapon;

			continue;
		}

		if ( pWeapon->getPreference() > iBestPreference )
		{
			iBestPreference = pWeapon->getPreference();
			m_theBestWeapon = pWeapon;
		}
	}

	if ( (m_theBestWeapon == NULL) && (flDist < 512) && (fabs(vEnemyOrigin.z-m_pBot->getOrigin().z)<BOT_JUMP_HEIGHT) )
		m_theBestWeapon = m_FallbackMelee;

	return m_theBestWeapon;
}

void CBotWeapons :: addWeapon ( int iId )
{

	int i = 0;
	Vector origin;
	edict_t *pEnt;
	const char *classname;
	CWeapon *pWeapon;

	m_theWeapons[iId].setHasWeapon(true);

	pWeapon = m_theWeapons[iId].getWeaponInfo();

	if ( !pWeapon )
		return;

	classname = pWeapon->getWeaponName();

	origin = m_pBot->getOrigin();

	for ( i = (gpGlobals->maxClients+1); i <= gpGlobals->maxEntities; i ++ )
	{
		pEnt = INDEXENT(i);

		if ( pEnt && CBotGlobals::entityIsValid(pEnt) )
		{
			if ( strcmp(pEnt->GetClassName(),classname) == 0 )
			{
				if ( CBotGlobals::entityOrigin(pEnt) == origin )
				{
					m_theWeapons[iId].setWeaponIndex(i);
					break;
				}
			}
		}
	}
}

CBotWeapon *CBotWeapons :: getWeapon ( CWeapon *pWeapon )
{
	for ( unsigned int i = 0; i < MAX_WEAPONS; i ++ )
	{
		if ( m_theWeapons[i].getWeaponInfo() == pWeapon )
			return &(m_theWeapons[i]);
	}

	return NULL;
}

void CBotWeapons :: clearWeapons ()
{
	for ( unsigned int i = 0; i < MAX_WEAPONS; i ++ )
		m_theWeapons[i].setHasWeapon(false);	
}

CBotWeapon *CBotWeapons :: getActiveWeapon ( const char *szWeaponName )
{
	if ( szWeaponName && *szWeaponName )
	{
		CWeapon *pWeapon = CWeapons::getWeapon(szWeaponName);

		if ( pWeapon )
			return &m_theWeapons[pWeapon->getID()];
	}

	return NULL;
}

bool CBotWeapon :: outOfAmmo(CBot *pBot)
{
	return getAmmo(pBot)==0;
}
/*
bool CBotWeapon :: needToReload(CBot *pBot)
{
	return getAmmo(pBot)==0;
}*/
////////////////////////////////////////
// CWeapons

class IWeaponFunc
{
public:
	virtual void execute ( CWeapon *pWeapon ) = 0;
};

class CGetWeapID : public IWeaponFunc
{
public:
	CGetWeapID ( int iId )
	{
		m_iId = iId;
		m_pFound = NULL;
	}

	void execute ( CWeapon *pWeapon )
	{
		if ( m_iId == pWeapon->getID() )
			m_pFound = pWeapon;
	}

	CWeapon *get ()
	{
		return m_pFound;
	}

private:
	int m_iId;
	CWeapon *m_pFound;
};

class CGetWeapCName : public IWeaponFunc
{
public:
	CGetWeapCName ( const char *szWeapon )
	{
        m_szWeapon = szWeapon;
	}

	void execute ( CWeapon *pWeapon )
	{
		if ( pWeapon->isWeaponName(m_szWeapon) )
			m_pFound = pWeapon;
	}

	CWeapon *get ()
	{
		return m_pFound;
	}
private:
	const char *m_szWeapon;
	CWeapon *m_pFound;
};

CWeapon *CWeapons :: getWeapon ( const int iId )
{
	CGetWeapID pFunc = CGetWeapID(iId);
	eachWeapon(&pFunc);
	return pFunc.get();
}

CWeapon *CWeapons :: getWeapon ( const char *szWeapon )
{
	CGetWeapCName pFunc = CGetWeapCName(szWeapon);
	eachWeapon(&pFunc);
	return pFunc.get();
}

void CWeapons :: eachWeapon ( IWeaponFunc *pFunc )
{
	for ( unsigned int i = 0; i < m_theWeapons.size(); i ++ )
	{
		pFunc->execute(m_theWeapons[i]);
	}
}

void CWeapons :: freeMemory ()
{
	for ( unsigned int i = 0; i < m_theWeapons.size(); i ++ )
	{
		delete m_theWeapons[i];
		m_theWeapons[i] = NULL;
	}

	m_theWeapons.clear();
}
