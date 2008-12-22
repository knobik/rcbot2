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
#include "igameevents.h"
#include "bot.h"
#include "bot_event.h"
#include "bot_strings.h"
#include "bot_globals.h"

vector<CBotEvent*> CBotEvents :: m_theEvents;
///////////////////////////////////////////////////////

void CRoundStartEvent :: execute ( IBotEventInterface *event )
{
	CBots::roundStart();
}

void CPlayerHurtEvent :: execute ( IBotEventInterface *event )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);
	edict_t *pAttacker = CBotGlobals::playerByUserId(event->getInt("attacker"));

	if ( pBot )
		pBot->hurt(pAttacker,event->getInt("health"));

	pBot = CBots::getBotPointer(pAttacker);

	if ( pBot )
		pBot->shot(m_pActivator);
}

void CPlayerDeathEvent :: execute ( IBotEventInterface *event )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);

	edict_t *pAttacker = CBotGlobals::playerByUserId(event->getInt("attacker"));

	if ( pBot )
		pBot->died(pAttacker);

	pBot = CBots::getBotPointer(pAttacker);

	if ( pBot )
		pBot->killed(m_pActivator);
}

void CBombPickupEvent :: execute ( IBotEventInterface *event )
{
}

void CPlayerFootstepEvent :: execute ( IBotEventInterface *event )
{
}

void CBombDroppedEvent :: execute ( IBotEventInterface *event )
{
}

void CWeaponFireEvent :: execute ( IBotEventInterface *event )
{
}

void CBulletImpactEvent :: execute ( IBotEventInterface *event )
{
	CBot *pBot = CBots::getBotPointer(m_pActivator);

	if ( pBot )
	{
		pBot->shotmiss();
	}
}
///////////////////////////////////////////////////////

void CBotEvent :: setType ( char *szType )
{
	m_szType = CStrings::getString(szType);
}
bool CBotEvent :: forCurrentMod ()
{
	return ((m_iModId == MOD_ANY) || (CBotGlobals::isMod(m_iModId)));
}
// should we execute this ??
bool CBotEvent :: isType ( const char *szType )
{
	return forCurrentMod() && FStrEq(m_szType,szType);
}

///////////////////////////////////////////////////////
void CBotEvents :: setupEvents ()
{
	addEvent(new CRoundStartEvent());
	addEvent(new CPlayerHurtEvent());
	addEvent(new CPlayerDeathEvent());
	addEvent(new CBombPickupEvent());
	addEvent(new CPlayerFootstepEvent());
	addEvent(new CBombDroppedEvent());
	addEvent(new CWeaponFireEvent());
	addEvent(new CBulletImpactEvent());
}

void CBotEvents :: addEvent ( CBotEvent *event )
{
	extern IGameEventManager2 *gameeventmanager;
	extern CRCBotPlugin g_RCBOTServerPlugin;

	if ( gameeventmanager )
		gameeventmanager->AddListener( g_RCBOTServerPlugin.getEventListener(), event->getName(), true );

	m_theEvents.push_back(event);
}

void CBotEvents :: freeMemory ()
{
	for ( unsigned int i = 0; i < m_theEvents.size(); i ++ )
	{
		delete m_theEvents[i];
		m_theEvents[i] = NULL;	
	}
	m_theEvents.clear();
}

void CBotEvents :: executeEvent( void *event, eBotEventType iType )
{
	CBotEvent *pFound;
	int iEventId = -1; 
	bool bFound;

	IBotEventInterface *pInterface = NULL;

	if ( iType == TYPE_KEYVALUES )
		pInterface = new CGameEventInterface1((KeyValues*)event);
	else if ( iType == TYPE_IGAMEEVENT )
		pInterface = new CGameEventInterface2((IGameEvent*)event);

	if ( pInterface == NULL )
		return;

	if ( iType != TYPE_IGAMEEVENT )
		iEventId = pInterface->getInt("eventid");

	for ( unsigned int i = 0; i < m_theEvents.size(); i ++ )
	{
		pFound = m_theEvents[i];

		// if it has an event id stored just check that
		//if ( ( iType != TYPE_IGAMEEVENT ) && pFound->hasEventId() )
		//	bFound = pFound->isEventId(iEventId);
		//else
		bFound = pFound->isType(pInterface->getName());

		if ( bFound )	
		{
			// set event id for quick checking
			pFound->setEventId(iEventId);

			pFound->setActivator(CBotGlobals::playerByUserId(pInterface->getInt("userid")));

			pFound->execute(pInterface);

			break;
		}
	}

	delete pInterface;
}
