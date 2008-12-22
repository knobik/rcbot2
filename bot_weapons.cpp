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

// static init (all weapons in game)
vector<CWeapon*> CWeapons :: m_theWeapons;

// compare preference sorting
class CompareWeapPref
{
public:
	bool operator()(CBotWeapon *a, CBotWeapon *b)
	{
		if ( a->outOfAmmo() )
			return false;//b
		if ( b->outOfAmmo() )
			return true;//a
		if ( a->needToReload() )
			return false;//b
		if ( b->needToReload() )
			return true;//a

		return a->getPreference() > b->getPreference();
	}
}; 

// Bot Weapons
CBotWeapons :: CBotWeapons ( CBot *pBot ) 
{
	m_pBot = pBot;

	for ( int i = 0; i < MAX_WEAPONS; i ++ )
		m_theWeapons[i].setWeapon(CWeapons::getWeapon(i));
}

CBotWeapon *CBotWeapons :: getBestWeapon ()
{
	priority_queue<CBotWeapon*,vector<CBotWeapon*>,CompareWeapPref> m_theBestWeapons;

	float flDist = 0;

	if ( m_pBot->hasEnemy() )
		flDist = m_pBot->distanceFrom(CBotGlobals::entityOrigin(m_pBot->getEnemy()));

	for ( unsigned int i = 0; i < MAX_WEAPONS; i ++ )
	{
		CBotWeapon *pWeapon = &(m_theWeapons[i]);

		if ( !pWeapon )
			continue;

		if ( pWeapon->outOfAmmo() )
			continue;

		if ( !pWeapon->primaryInRange(flDist) && !pWeapon->secondaryInRange(flDist) )
			continue;

		m_theBestWeapons.push(pWeapon);
	}

	return m_theBestWeapons.top();
}

void CBotWeapons :: addWeapon ( int iId )
{
	m_theWeapons[iId].setHasWeapon(true);	
}

void CBotWeapons :: addWeapon ( const char *szWeaponName )
{
	CWeapon *pWeapon = CWeapons::getWeapon(szWeaponName);

	m_theWeapons[pWeapon->getID()].setHasWeapon(true);
	//m_theWeapons(pWeapon->get)
	//m_theWeapons.push_back(pWeapon);
}
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
	m_theWeapons.clear();
}
