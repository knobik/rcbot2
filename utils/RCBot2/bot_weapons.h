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
#ifndef __BOT_WEAPONS_H__
#define __BOT_WEAPONS_H__

#include <vector>
using namespace std;

#include "shareddefs.h"

class CBot;
/*
	CBaseCombatWeapon*	Weapon_Create( const char *pWeaponName );
	virtual Activity	Weapon_TranslateActivity( Activity baseAct, bool *pRequired = NULL );
	void				Weapon_SetActivity( Activity newActivity, float duration );
	void				Weapon_FrameUpdate( void );
	void				Weapon_HandleAnimEvent( animevent_t *pEvent );
	CBaseCombatWeapon*	Weapon_OwnsThisType( const char *pszWeapon, int iSubType = 0 ) const;  // True if already owns a weapon of this class
	virtual bool		Weapon_CanUse( CBaseCombatWeapon *pWeapon );		// True is allowed to use this class of weapon
	virtual void		Weapon_Equip( CBaseCombatWeapon *pWeapon );			// Adds weapon to player
	virtual bool		Weapon_EquipAmmoOnly( CBaseCombatWeapon *pWeapon );	// Adds weapon ammo to player, leaves weapon
	bool				Weapon_Detach( CBaseCombatWeapon *pWeapon );		// Clear any pointers to the weapon.
	virtual void		Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = NULL, const Vector *pVelocity = NULL );
	virtual	bool		Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 );		// Switch to given weapon if has ammo (false if failed)
	virtual	Vector		Weapon_ShootPosition( );		// gun position at current position/orientation
	bool				Weapon_IsOnGround( CBaseCombatWeapon *pWeapon );
	CBaseEntity*		Weapon_FindUsable( const Vector &range );			// search for a usable weapon in this range
	virtual	bool		Weapon_CanSwitchTo(CBaseCombatWeapon *pWeapon);
	virtual bool		Weapon_SlotOccupied( CBaseCombatWeapon *pWeapon );
	virtual CBaseCombatWeapon *Weapon_GetSlot( int slot ) const;
	CBaseCombatWeapon	*Weapon_GetWpnForAmmo( int iAmmoIndex );
*/

#define WEAP_FL_PRIM_ATTACK (1<<0)
#define WEAP_FL_SEC_ATTACK	(1<<1)
#define WEAP_FL_EXPLOSIVE	(1<<2)
#define WEAP_FL_MELEE       (1<<3)
#define WEAP_FL_UNDERWATER  (1<<4)

class CWeapon
{
public:
	CWeapon( const char *szWeaponName, int iFlags = 0 )
	{
		setID(0);
		setName(szWeaponName);
		setModel(NULL);
		setFlags(iFlags);
	}

	inline void setName ( const char *szWeaponName )
	{
		m_szWeaponName = szWeaponName;
	}

	inline bool isWeaponName ( const char *szWeaponName )
	{
		return !strcmp(szWeaponName,getWeaponName());
	}

	inline void setModel ( const char *szModelName )
	{
		m_szModelName = szModelName;
	}

	inline void setID ( const int iId )
	{
		m_iWeaponId = iId;
	}

	inline void setFlags ( const int iFlags )
	{
		m_iFlags = iFlags;
	}

	inline const char *getWeaponName () const
	{
		return m_szWeaponName;
	}

	inline const int getID () const
	{
		return m_iWeaponId;
	}

	inline bool hasAllFlags ( int iFlags ) const
	{
		return (m_iFlags & iFlags)==iFlags;
	}

	inline bool hasSomeFlags ( int iFlags ) const
	{
		return (m_iFlags & iFlags)>0;
	}

private:
	const char *m_szWeaponName; // classname
	const char *m_szModelName;  // model file
	int m_iWeaponId;			// identification
	int m_iFlags;				// flags
};

class IWeaponFunc;

class CWeapons
{
public:
	static void addWeapon ( CWeapon *pWeapon );

	static CWeapon *getWeapon ( const int iId );

	static CWeapon *getWeapon ( const char *szWeapon );

	static void eachWeapon ( IWeaponFunc *pFunc );

	static void freeMemory ();
private:
	// available weapons in game
	static vector<CWeapon*> m_theWeapons;
};

////////////////////////////////////////////////////////////
// Weapon but with bot holding it and ammo information etc
////////////////////////////////////////////////////////////
class CBotWeapon
{
public:
	CBotWeapon ()
	{
		m_pWeaponInfo = NULL;

		// ammo left
		m_iPrimAmmo = 1;
		m_iSecAmmo = 0;
		m_iPrimReserve = 0;
		m_iSecReserve = 0;

		// shoot distance (default)
		m_fPrimMinWeaponShootDist = 0.0f;
		m_fPrimMaxWeaponShootDist = 8192.0f;

		m_fSecMinWeaponShootDist = 0.0f;
		m_fSecMaxWeaponShootDist = 8192.0f;

		m_iPreference = 0;	// bots preference to weapon
		m_bHasWeapon = false;
	}

	inline void setWeapon ( CWeapon *pWeapon )
	{
		m_pWeaponInfo = pWeapon;
	}

	inline bool primaryInRange ( float fDistance )
	{
		return (fDistance>m_fPrimMinWeaponShootDist)&&(fDistance<m_fPrimMaxWeaponShootDist);
	}

	inline bool secondaryInRange ( float fDistance )
	{
		return (fDistance>m_fSecMinWeaponShootDist)&&(fDistance<m_fSecMaxWeaponShootDist);
	}

	inline int getPreference ()
	{
		return m_iPreference;
	}

	inline void setPreference ( int iPreference )
	{
		m_iPreference = iPreference;
	}

	inline void setPrimaryRange ( float fMinRange, float fMaxRange )
	{
		m_fPrimMinWeaponShootDist = fMinRange; 
		m_fPrimMaxWeaponShootDist = fMaxRange;
	}

	inline void setSecondaryRange ( float fMinRange, float fMaxRange )
	{
		m_fSecMinWeaponShootDist = fMinRange;
		m_fSecMaxWeaponShootDist = fMaxRange;
	}

	inline bool outOfAmmo ()
	{
		return !m_iPrimAmmo && !m_iSecAmmo && !m_iPrimReserve && !m_iSecReserve;
	}

	inline bool needToReload ()
	{
		return !m_pWeaponInfo->hasAllFlags(WEAP_FL_MELEE) && ((m_iPrimAmmo == 0)&&(m_iPrimReserve>0));
	}

	inline void setHasWeapon ( bool bHas )
	{
		m_bHasWeapon = bHas;
	}

	inline bool hasWeapon ()
	{
		return m_bHasWeapon;
	}

	CWeapon *getWeaponInfo () { return m_pWeaponInfo; }
private:
	// link to weapon info
	CWeapon *m_pWeaponInfo;

	// ammo left
	int m_iPrimAmmo;
	int m_iSecAmmo;
	int m_iPrimReserve;
	int m_iSecReserve;

	// shoot distance
	float m_fPrimMinWeaponShootDist;
	float m_fPrimMaxWeaponShootDist;

	float m_fSecMinWeaponShootDist;
	float m_fSecMaxWeaponShootDist;

	int m_iPreference;	// bots preference to weapon

	bool m_bHasWeapon;
};

// Weapons that
class CBotWeapons 
{
public:
/////////////////////////////////////
	CBotWeapons ( CBot *pBot );    // // constructor
/////////////////////////////////////
	CBotWeapon *getBestWeapon ();

	void addWeapon ( int iId );
	void addWeapon ( const char *szWeaponName );
private:
	// bot that has these weapons
	CBot *m_pBot;

	// weapons local to the bot only 
	// (holds ammo/preference etc and link to actual weapon)
	CBotWeapon m_theWeapons[MAX_WEAPONS];
};

#endif