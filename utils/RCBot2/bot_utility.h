/*
 *    part of https://rcbot2.svn.sourceforge.net/svnroot/rcbot2
 *
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

#ifndef __BOT_UTILITY_H__
#define __BOT_UTILITY_H__

#include "bot_genclass.h"

#include <vector>
using namespace std;


typedef enum
{
 BOT_UTIL_BUILDSENTRY = 0,
 BOT_UTIL_BUILDDISP,
 BOT_UTIL_BUILDTELENT,
 BOT_UTIL_BUILDTELEXT,
 BOT_UTIL_UPGSENTRY,
 BOT_UTIL_UPGDISP,
 BOT_UTIL_UPGTELENT,
 BOT_UTIL_UPGTELEXT,
 BOT_UTIL_UPGTMSENTRY,
 BOT_UTIL_UPGTMDISP,
 BOT_UTIL_UPGTMTELENT,
 BOT_UTIL_UPGTMTELEXT,
 BOT_UTIL_GOTODISP,
 BOT_UTIL_GOTORESUPPLY_FOR_HEALTH,
 BOT_UTIL_GETAMMOKIT,
 BOT_UTIL_GETAMMOTMDISP,
 BOT_UTIL_GETAMMODISP,
 BOT_UTIL_GETFLAG,
 BOT_UTIL_GETHEALTHKIT,
 BOT_UTIL_GETFLAG_LASTKNOWN,
 BOT_UTIL_SNIPE,
 BOT_UTIL_ROAM,
 BOT_UTIL_CAPTURE_FLAG,
 BOT_UTIL_GOTORESUPPLY_FOR_AMMO,
 BOT_UTIL_FIND_NEAREST_HEALTH,
 BOT_UTIL_FIND_NEAREST_AMMO,
 BOT_UTIL_ATTACK_POINT,
 BOT_UTIL_DEFEND_POINT,
 BOT_UTIL_DEFEND_FLAG,
 BOT_UTIL_ENGI_LOOK_AFTER_SENTRY,
 BOT_UTIL_DEFEND_FLAG_LASTKNOWN,
 BOT_UTIL_GOTO_PAYLOAD_BOMB,
 BOT_UTIL_MAX
}eBotAction;

class CBotUtility
{
public:
	CBotUtility ( eBotAction id, bool bCanDo, float fUtil )
	{
		m_fUtility = fUtil;
		m_id = id;
		m_bCanDo = bCanDo;
	}

	inline float getUtility () { return m_fUtility; }

	inline eBotAction getId () { return m_id; }

	inline bool canDo () { return m_bCanDo; }

private:
	float m_fUtility;
	bool m_bCanDo;
	eBotAction m_id;
};


typedef struct util_node_s
{
  CBotUtility *util;
  struct util_node_s *next;
}util_node_t;


typedef struct
{
	util_node_t *head;
}util_list;

class CBotUtilities
{
public:

	CBotUtilities ()
	{
		m_pBest.head = NULL;
	}

	void freeMemory ();

	inline void addUtility ( CBotUtility p ) { m_Utilities.push_back(p); }

	void execute ();

	CBotUtility *nextBest ();

private:
	vector<CBotUtility> m_Utilities;

	util_list m_pBest;
};




#endif