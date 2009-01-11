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

#define BOT_UTIL_BUILDSENTRY   0
#define BOT_UTIL_BUILDDISP     1
#define BOT_UTIL_BUILDTELENT   2
#define BOT_UTIL_BUILDTELEXT   3
#define BOT_UTIL_UPGSENTRY     4
#define BOT_UTIL_UPGDISP       5
#define BOT_UTIL_UPGTELENT     6
#define BOT_UTIL_UPGTELEXT     7
#define BOT_UTIL_UPGTMSENTRY   8
#define BOT_UTIL_UPGTMDISP	   9
#define BOT_UTIL_UPGTMTELENT  10
#define BOT_UTIL_UPGTMTELEXT  11
#define BOT_UTIL_GOTODISP		12
#define BOT_UTIL_GOTORESUPPLY_FOR_HEALTH   13
#define BOT_UTIL_GETAMMOKIT   14
#define BOT_UTIL_GETAMMOTMDISP 15
#define BOT_UTIL_GETAMMODISP 16
#define BOT_UTIL_GETFLAG 17
#define BOT_UTIL_GETHEALTHKIT 18
#define BOT_UTIL_GETFLAG_LASTKNOWN 19
#define BOT_UTIL_SNIPE 20
#define BOT_UTIL_ROAM 21
#define BOT_UTIL_CAPTURE_FLAG 22
#define BOT_UTIL_GOTORESUPPLY_FOR_AMMO 23
#define BOT_UTIL_FIND_NEAREST_HEALTH 24
#define BOT_UTIL_FIND_NEAREST_AMMO 25

class CBotUtility
{
public:
	CBotUtility ( int id, bool bCanDo, float fUtil )
	{
		m_fUtility = fUtil;
		m_id = id;
		m_bCanDo = bCanDo;
	}

	inline float getUtility () { return m_fUtility; }

	inline int getId () { return m_id; }

	inline bool canDo () { return m_bCanDo; }

private:
	float m_fUtility;
	bool m_bCanDo;
	int m_id;
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