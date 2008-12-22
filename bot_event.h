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
#ifndef __BOT_EVENT_H__
#define __BOT_EVENT_H__

#include <vector>
using namespace std;

class CBotEventInterface;
class IBotEventInterface;

class KeyValues;
class IGameEvent;

class CBotEvent
{
public:
	CBotEvent()
	{
		m_iEventId = -1;
		m_szType = NULL;
		m_iModId = MOD_ANY;
	}

	void setMod ( eModId iModId )
	{
		m_iModId = iModId;
	}

	bool forCurrentMod ();

	void setType ( char *szType );

	bool isType ( const char *szType );

	inline void setActivator ( edict_t *pEdict ) { m_pActivator = pEdict;}

	virtual void execute ( IBotEventInterface *pEvent ) { return; }

	inline void setEventId ( int iEventId )
	{
		m_iEventId = iEventId;
	}

	inline bool isEventId ( int iEventId )
	{
		return forCurrentMod() && (m_iEventId == iEventId);
	}

	inline bool hasEventId ()
	{
		return (m_iEventId != -1);
	}

	const char *getName ()
	{
		return m_szType;
	}
protected:
	edict_t *m_pActivator;
private:
	char *m_szType;
	int m_iEventId;	
	eModId m_iModId;
};

class CRoundStartEvent : public CBotEvent
{
public:
	CRoundStartEvent()
	{
		setType("round_start");
		setMod(MOD_CSS);
	}

	void execute ( IBotEventInterface *event );
};

class CPlayerHurtEvent : public CBotEvent
{
public:
	CPlayerHurtEvent()
	{
		setType("player_hurt");
	}

	void execute ( IBotEventInterface *event );
};

class CPlayerDeathEvent : public CBotEvent
{
public:
	CPlayerDeathEvent()
	{
		setType("player_death");
	}

	void execute ( IBotEventInterface *event );
};

class CBombPickupEvent : public CBotEvent
{
public:
	CBombPickupEvent()
	{
		setType("bomb_pickup");
		setMod(MOD_CSS);
	}

	void execute ( IBotEventInterface *event );
};

class CPlayerFootstepEvent : public CBotEvent
{
public:
	CPlayerFootstepEvent()
	{
		setType("player_footstep");
	}

	void execute ( IBotEventInterface *event );
};

class CBombDroppedEvent : public CBotEvent
{
public:
	CBombDroppedEvent()
	{
		setType("bomb_dropped");
		setMod(MOD_CSS);
	}

	void execute ( IBotEventInterface *event );
};


class CWeaponFireEvent : public CBotEvent
{
public:
	CWeaponFireEvent()
	{
		setType("weapon_fire");
	}

	void execute ( IBotEventInterface *event );
};

class CBulletImpactEvent : public CBotEvent
{
public:
	CBulletImpactEvent()
	{
		setType("bullet_impact");
	}

	void execute ( IBotEventInterface *event );
};

typedef enum
{
	TYPE_KEYVALUES = 0,
	TYPE_IGAMEEVENT = 1
}eBotEventType;

class IBotEventInterface
{
public:
	virtual float getFloat ( const char *keyName = 0, float defaultValue = 0 ) = 0;
	virtual int getInt ( const char *keyName = 0, int defaultValue = 0 ) = 0;
	virtual const char *getString ( const char *keyName = 0, const char *defaultValue = 0 ) = 0;
	virtual const char *getName () = 0;
};

class CGameEventInterface1 : public IBotEventInterface
{
public:
	CGameEventInterface1 ( KeyValues *pEvent )
	{
		m_pEvent = pEvent;
	}

	float getFloat ( const char *keyName = 0, float defaultValue = 0 )
	{
		return m_pEvent->GetFloat(keyName,defaultValue);
	}
	int getInt ( const char *keyName = 0, int defaultValue = 0 )
	{
		return m_pEvent->GetInt(keyName,defaultValue);
	}
	const char *getString ( const char *keyName = 0, const char *defaultValue = 0 )
	{
		return m_pEvent->GetString(keyName,defaultValue);
	}
	const char *getName ()
	{
		return m_pEvent->GetName();
	}

private:
	KeyValues *m_pEvent;
};

class CGameEventInterface2 : public IBotEventInterface
{
public:
	CGameEventInterface2 ( IGameEvent *pEvent )
	{
		m_pEvent = pEvent;
	}

	float getFloat ( const char *keyName = 0, float defaultValue = 0 )
	{
		return m_pEvent->GetFloat(keyName,defaultValue);
	}
	int getInt ( const char *keyName = 0, int defaultValue = 0 )
	{
		return m_pEvent->GetInt(keyName,defaultValue);
	}
	const char *getString ( const char *keyName = 0, const char *defaultValue = 0 )
	{
		return m_pEvent->GetString(keyName,defaultValue);
	}

	const char *getName ()
	{
		return m_pEvent->GetName();
	}
private:
	IGameEvent *m_pEvent;
};

class CBotEvents
{
public:
	static void setupEvents ();

	static void executeEvent( void *event, eBotEventType iType );

	static void freeMemory ();

	static void addEvent ( CBotEvent *event );

private:
	static vector<CBotEvent*> m_theEvents;
};
#endif