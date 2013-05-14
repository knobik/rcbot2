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

#include <vector>
using namespace std;

class CClient;
class CBotMenu;
class CBotMenuItem;
class WptColor;

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

class CBotMenuItem
{
public:
	virtual const char *getCaption ( CClient *pClient, WptColor &color );

	virtual void activate ( CClient *pClient ) = 0;

	void setCaption ( const char *szCaption ) 
	{
		strncpy(m_szCaption,szCaption,63);
		m_szCaption[63] = 0;
	}

protected:
	char m_szCaption[64];
};

class CWaypointFlagMenuItem : public CBotMenuItem
{
public:
	CWaypointFlagMenuItem ( int iFlag )
	{
		m_iFlag = iFlag;
	}

	const char *getCaption ( CClient *pClient, WptColor &color );

	void activate ( CClient *pClient );

private:
	int m_iFlag;
};

class CBotGotoMenuItem : public CBotMenuItem
{
public:
	CBotGotoMenuItem ( const char *szCaption, CBotMenu *pPrevMenu )  // caption = back / more etc
	{
		strncpy(m_szCaption,szCaption,63);
		m_szCaption[63] = 0;
		m_pPrevMenu = pPrevMenu;
	}

	void activate ( CClient *pClient )
	{
		pClient->setCurrentMenu(m_pPrevMenu);
	}
private:
	CBotMenu *m_pPrevMenu;
};

class CBotExitMenuItem : public CBotMenuItem
{
	const char *getCaption ( CClient *pClient, WptColor &color )
	{
		return "Exit";
	}

	void activate ( CClient *pClient )
	{
		pClient->setCurrentMenu(NULL);
	}
};

class CWaypointAreaMenuItem : public CBotMenuItem
{
public:
	const char *getCaption ( CClient *pClient, WptColor &color );

	void activate ( CClient *pClient );
};

class CBotMenu : public CBotMenuItem
{
public:

	void freeMemory ();

	virtual const char *getCaption ( CClient *pClient, WptColor &color )
	{
		return CBotMenuItem::getCaption(pClient,color);
	}// returns the caption (may be dynamic)

	void activate ( CClient *pClient );

	Color getColor ( CClient *pClient ); // gets the colour of the caption

	virtual void addMenuItem ( CBotMenuItem *item )
	{
		m_MenuItems.push_back(item);
	}

	void render ( CClient *pClient );

	void selectedMenu ( CClient *pClient, unsigned int iMenu );

private:
	vector<CBotMenuItem*> m_MenuItems;
};

class CWaypointFlagMenu : public CBotMenu
{
public:
	CWaypointFlagMenu (CBotMenu *pParent);
	//CWaypointFlagMenu ( int iShow );
	const char *getCaption(CClient *pClient,WptColor &color );
};

typedef enum eWptMenu2Lev
{
	WPT_MENU_MAIN = 0,
	WPT_MENU_AREA,
	WPT_MENU_YAW,
	WPT_MENU_TYPES,
	WPT_MENU_SHOW,
	WPT_MENU_PATHS,
};

class CWaypointRadiusIncrease : public CBotMenuItem
{
public:
	CWaypointRadiusIncrease() { setCaption("Increase Radius (+)"); }

	void activate ( CClient *pClient );
	//const char *getCaption(CClient *pClient, WptColor &color);
};

class CWaypointRadiusDecrease : public CBotMenuItem
{
public:
	CWaypointRadiusDecrease() { setCaption("Decrease Radius (-)"); }

	void activate ( CClient *pClient );
	//const char *getCaption(CClient *pClient, WptColor &color);
};

class CWaypointRadiusMenu : public CBotMenu
{
public:
	CWaypointRadiusMenu( CBotMenu *pParent )
	{
		addMenuItem(new CWaypointRadiusIncrease());
		addMenuItem(new CWaypointRadiusDecrease());
		addMenuItem(new CBotGotoMenuItem("Back",pParent));
	}

	const char *getCaption ( CClient *pClient, WptColor &color );
};

class CWaypointAreaIncrease : public CBotMenuItem
{
public:
	CWaypointAreaIncrease() { setCaption("Increase Area (+)"); }

	void activate ( CClient *pClient );
	//const char *getCaption(CClient *pClient, WptColor &color);
};

class CWaypointAreaDecrease : public CBotMenuItem
{
public:
	CWaypointAreaDecrease() { setCaption("Decrease Area (-)"); }

	void activate ( CClient *pClient );
	//const char *getCaption(CClient *pClient, WptColor &color);
};

class CWaypointAreaMenu : public CBotMenu
{
public:
	CWaypointAreaMenu( CBotMenu *pParent )
	{
		addMenuItem(new CWaypointAreaIncrease());
		addMenuItem(new CWaypointAreaDecrease());
		addMenuItem(new CBotGotoMenuItem("Back",pParent));
	}

	const char *getCaption ( CClient *pClient, WptColor &color );
};

class CWaypointYawMenuItem : public CBotMenuItem
{
	const char *getCaption ( CClient *pClient, WptColor &color );

	void activate ( CClient *pClient );
};

class CWaypointCutMenuItem : public CBotMenuItem
{
	const char *getCaption ( CClient *pClient, WptColor &color );

	void activate ( CClient *pClient );
};

class CWaypointCopyMenuItem : public CBotMenuItem
{
	const char *getCaption ( CClient *pClient, WptColor &color );

	void activate ( CClient *pClient );
};

class CWaypointPasteMenuItem : public CBotMenuItem
{
	const char *getCaption ( CClient *pClient, WptColor &color );

	void activate ( CClient *pClient );
};

class CWaypointMenu : public CBotMenu
{
public:
	// needs 
	// Waypoint Menu ID []
	// 1 Waypoint Flags []
	// 2 area []
	// 3 yaw [] degrees
	// 4 remove all flags
	// 5 clear waypoints
	// 6 copy
	// 7 cut
	// 8 paste
	// 9 Exit
	CWaypointMenu ()
	{
		addMenuItem(new CWaypointFlagMenu(this));
		addMenuItem(new CWaypointAreaMenu(this));
		addMenuItem(new CWaypointRadiusMenu(this));
		addMenuItem(new CWaypointYawMenuItem());
		addMenuItem(new CWaypointCutMenuItem());
		addMenuItem(new CWaypointCopyMenuItem());
		addMenuItem(new CWaypointPasteMenuItem());
		addMenuItem(new CBotGotoMenuItem("Exit",NULL));
	}

	//CWaypointFlagMenu ( int iShow );
	const char *getCaption(CClient *pClient,WptColor &color );
};

typedef enum 
{
	BOT_MENU_WPT = 0,
	BOT_MENU_MAX
};

class CBotMenuList
{
public:
	CBotMenuList ()
	{
		memset(m_MenuList,0,sizeof(CBotMenu*)*BOT_MENU_MAX);
	}

	static void setupMenus ();

	static void freeMemory ();
	
	static void render ( CClient *pClient ); // render

	static void selectedMenu ( CClient *pClient, unsigned int iMenu );

	static CBotMenu *getMenu ( int id ) { return m_MenuList[id]; }

private:
	static CBotMenu *m_MenuList[BOT_MENU_MAX];
};
/*
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

	void init ( const char *szCaption, const char *szCommand );

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

protected:

	virtual const char *getCaption ();

//	int m_iExitItem;

	CBotMenuItem *m_Menus[10]; // Maximum of ten sub-menu's

	char m_szCaption[128];
	char m_szCommand[32]; // e.g. waypoint_menu
};

class CBotWaypointMenu : public CBotMenu
{
	CBotWaypointMenu ()
	{
		init("Waypoint Menu","waypoint_menu");
		addMenuItem();
	}

private:
	virtual const char *getCaption ()
	{
		m_szCaption;
	}

	int m_iWaypointID;
};

class CBotMenus
{
	CBotMenus()
	{
		m_Menus.push_back(new CBotWaypointMenu());
	}

	void check ();
private:
	int m_iActiveMenu;
	vector<CBotMenu*> m_Menus;
};

class CBotMenuInfo : public CBotMenu
{
public:
	CBotMenuInfo ( m_iValue, CBotMenuCaption *caption );
private:
	int m_iValue;
}
*/

#endif
