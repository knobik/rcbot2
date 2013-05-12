/*
 *    This file is part of RCBot2. Taken from RCBot1
 *
 *    RCBot1 by Paul Murphy adapted from botman's template 3.
 *	  RCBot2 by Paul Murphy adapted from Botman's HPB Bot 2 template.
 *
 *    RCBot2 is free software; you can redistribute it and/or modify it
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
/****************************************************
 * RCBOT Bot Menu's                                 *
 *                                                  *
 * Code by : Paul Murphy							*
 *                       {cheeseh - at - rcbot.net} *
 *                                                  *
 ****************************************************
 *
 * Paul M
 * If using any of these components please keep this tag
 * pa..please .. I know its a nice way of using menus :-p
 *
 ****************************************************
*/

#ifndef __BOT_MENU_H__
#define __BOT_MENU_H__

class CClient;
class CBotMenu;

// menu types
typedef enum
{
	BOT_MENU_NONE = 0,
	BOT_MENU_WAYPOINT_MAIN,
	BOT_MENU_WAYPOINT_TEAM_SPEC,
	BOT_MENU_WAYPOINT_GIVE_FLAGS,
	BOT_MENU_WAYPOINT_GIVE_FLAGS2,
	BOT_MENU_WAYPOINT_GIVE_FLAGS3,
	BOT_MENU_WAYPOINT_GIVE_FLAGS4,
	BOT_MENU_WAYPOINT_GIVE_FLAGS5,
	BOT_MENU_WAYPOINT_GIVE_FLAGS6,
	BOT_MENU_WAYPOINT_EDIT_PATHS,
	BOT_MENU_WAYPOINT_CONFIRM_DELETE,
	BOT_MENU_BOT_MAIN,
	BOT_MENU_BOT_ADDBOT_TEAM,
	BOT_MENU_SQUAD,
	BOT_MENU_SQUAD_FORMATION,
	BOT_MENU_SQUAD_SPREAD,
	BOT_MENU_SQUAD_OPTIONS1,
	BOT_MENU_SQUAD_FORMATION2,
	BOT_MENU_SQUAD_MODES1,
	BOT_MENU_KICKBOT_TEAM,
	/* Need 'max items' item, will tell the program how many menus to make */
	BOT_MENU_WAYPOINT_GIVE_FLAGS7,
	BOT_MENU_MAX_ITEMS
}eBotMenus;

#define MAX_MENU_CAPTION_LENGTH 64

class CMenuCaption
{
public:
	CMenuCaption ( const char *szCaption );

	virtual char *getCaption ( CMenu *pParentMenu );
protected:
	char m_szCaption[MAX_MENU_CAPTION_LENGTH]; // maximum length
};

class CDynamicMenuCaption : CMenuCaption
{
public:
	virtual char *getCaption ( CMenu *pParentMenu ) = 0;
};

class CAreaMenuCaption : CDynamicMenuCaption
{
public:
	char *getCaption ( CMenu *pParentMenu )
	{
		// waypoint id
		pParentMenu->getInt();

		return Area;
	}
};
//////////////////////////////
// MENU HANDLING

// Could have made this more oo'd, just uses polymorphic functions instead.
class CBotMenuItem
{
public:

	~CBotMenuItem();

	void Init (void)
	{
		m_szMenuCaption = NULL;
		m_pNextMenu = NULL;
		m_pMenuFunction = NULL;
	}

	CBotMenuItem()
	{
		this->Init();
	}

	CBotMenuItem(const char *szMenuCaption);

	CBotMenuItem( const char *szMenuCaption, CBotMenu *pNextMenu );

	CBotMenuItem( const char *szMenuCaption, void (*pMenuFunction)(CClient*) );

	inline BOOL HasNextMenu ( void )
	{
		return (m_pNextMenu != NULL);
	}

	void Activate ( CClient *pClient );

	inline const char *GetCaption ( void )
	{
		return m_szMenuCaption;
	}

private:
	char *m_szMenuCaption;

	CBotMenu *m_pNextMenu;

	void (*m_pMenuFunction)(CClient *);
};

class CBotMenu
{
public:

	~CBotMenu();

	CBotMenu()
	{
		this->InitMenu();
		//memset(m_Menus,0,sizeof(CBotMenuItem)*10);
	}

	void DestroyMenu(void);

	void InitMenu (void);

	CBotMenu( const char *szCaption );

	void AddExitMenuItem ( int iMenuNum )
	{
		//if ( m_Menus[iMenuNum] )
		//	delete m_Menus[iMenuNum];

		m_Menus[iMenuNum] = new CBotMenuItem("Exit");
	}

	void AddMenuItem ( int iMenuNum, const char *szMenuCaption, CBotMenu *pNextMenu )
	{
		//if ( m_Menus[iMenuNum] )
		//	delete m_Menus[iMenuNum];

		m_Menus[iMenuNum] = new CBotMenuItem(szMenuCaption,pNextMenu);
	}

	void AddMenuItem ( int iMenuNum, const char *szMenuCaption, void (*m_pMenuFunction)(CClient*) )
	{
		//if ( m_Menus[iMenuNum] )
		//	delete m_Menus[iMenuNum];

		m_Menus[iMenuNum] = new CBotMenuItem(szMenuCaption,m_pMenuFunction);
	}

	void Activate ( int iMenuItem, CClient *pClient )
	{
		if ( (iMenuItem < 0) || (iMenuItem >= 10) )
			return;

		if ( m_Menus[iMenuItem] )
			m_Menus[iMenuItem]->Activate(pClient);
	}

	void Render ( CClient *pClient );

private:

//	int m_iExitItem;

	CBotMenuItem *m_Menus[10]; // Maximum of ten sub-menu's

	CMenuCaption *m_szCaption;
};

class CBotMenuInfo : public CBotMenu
{
public:
	CBotMenuInfo ( m_iValue, CBotMenuCaption *caption );
private:
	int m_iValue;
}
