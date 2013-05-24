
#include "filesystem.h"
#include "interface.h"
#include "engine/iserverplugin.h"
#include "iplayerinfo.h"
#include "vplane.h"
#include "eiface.h"

#include "ndebugoverlay.h"

#include "bot_client.h"
#include "bot_waypoint.h"
#include "bot_menu.h"
#include "bot_wpt_color.h"
#include "bot_globals.h"

extern IVDebugOverlay *debugoverlay;

CBotMenu *CBotMenuList :: m_MenuList[BOT_MENU_MAX];

void CWaypointFlagMenuItem :: activate ( CClient *pClient )
{
	int iWpt = pClient->currentWaypoint();
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);
	CWaypointType *type = CWaypointTypes::getTypeByIndex(m_iFlag);

	if ( pWpt )
	{
		if ( pWpt->hasFlag(type->getBits()) )
			pWpt->removeFlag(type->getBits());
		else
			pWpt->addFlag(type->getBits());
	}
}

const char *CWaypointFlagMenu :: getCaption(CClient *pClient,WptColor &color )
{
	pClient->updateCurrentWaypoint();

	int iWpt = pClient->currentWaypoint();
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);

	if ( pWpt )
	{
		color = CWaypointTypes::getColour(pWpt->getFlags());
		sprintf(m_szCaption,"Waypoint Flags ID = [%d]",iWpt);
	}
	else
	{
		color = WptColor::white;
		sprintf(m_szCaption,"No Waypoint");
	}

	return m_szCaption;
}

const char *CWaypointFlagMenuItem :: getCaption ( CClient *pClient, WptColor &color )
{
	pClient->updateCurrentWaypoint();

	int iWpt = pClient->currentWaypoint();
	CWaypoint * pWpt = CWaypoints::getWaypoint(iWpt);

	CWaypointType *type = CWaypointTypes::getTypeByIndex(m_iFlag);

	color = type->getColour();

	sprintf(m_szCaption,"[%s] %s",(pWpt!=NULL)?(pWpt->hasFlag(type->getBits())?"x":" "):"No Waypoint",type->getName());

	return m_szCaption;
}

CWaypointFlagMenu :: CWaypointFlagMenu ( CBotMenu *pPrev )
{
	int iMod = CBotGlobals::getCurrentMod()->getModId();
	// check the number of waypoint types available
	// caption
	// 1.
	// 2.
	// 3.
	// 4.
	// 5.
	// 6.
	// 7.
	// 8. More...
	// 9. Go Back

	int iNumTypes = CWaypointTypes::getNumTypes();

	int iNumAdded = 0;
	CBotMenu *pParent;
	CBotMenu *pCurrent;

	int i;

	pCurrent = this;
	pParent = pPrev;

	for ( i = 0; i < iNumTypes; i ++ )
	{
		if ( !CWaypointTypes::getTypeByIndex(i)->forMod(iMod) )
			continue;

		pCurrent->addMenuItem(new CWaypointFlagMenuItem(i));
		iNumAdded++;

		if ( (iNumAdded > 7) || (i == (iNumTypes-1)) )
		{
			CBotMenuItem *back = new CBotGotoMenuItem("Back...",pParent);

			pParent = pCurrent;

			if ( (iNumAdded > 7) && (i < (iNumTypes-1)) )
			{
				pCurrent = new CBotMenu();
				pCurrent->setCaption("Waypoint Flags (More)");
				pParent->addMenuItem(new CBotGotoMenuItem("More...",pCurrent));
			}

			pParent->addMenuItem(back);

		//	make a new menu

			iNumAdded = 0; // reset

		}
	}

}

void CBotMenuList :: setupMenus ()
{
	m_MenuList[BOT_MENU_WPT] = new CWaypointMenu(); //new CWaypointFlagMenu(NULL);
}

const char *CWaypointRadiusMenu :: getCaption ( CClient *pClient, WptColor &color )
{
	int iWpt = pClient->currentWaypoint();
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);
	float fRadius = 0;

	if ( pWpt )
	{
		fRadius = pWpt->getRadius();
	}

	sprintf(m_szCaption,"Waypoint Radius (%0.1f)",fRadius);
	color = WptColor::white;

	return m_szCaption;
}

const char *CWaypointAreaMenu :: getCaption ( CClient *pClient, WptColor &color )
{
	int iWpt = pClient->currentWaypoint();
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);
	int iArea = 0;

	if ( pWpt )
	{
		iArea = pWpt->getArea();
	}

	sprintf(m_szCaption,"Waypoint Area (%d)",iArea);
	color = WptColor::white;

	return m_szCaption;
}

const char *CWaypointMenu::getCaption(CClient *pClient,WptColor &color )
{
	int iWpt = pClient->currentWaypoint();
	
	if ( iWpt == -1 )
		sprintf(m_szCaption,"Waypoint Menu - No waypoint - Walk towards a waypoint");
	else
		sprintf(m_szCaption,"Waypoint Menu [%d]",iWpt);

	color = WptColor::white;

	return m_szCaption;
}

const char *CWaypointYawMenuItem :: getCaption ( CClient *pClient, WptColor &color )
{
	CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

	if ( pWpt )
		sprintf(m_szCaption,"Yaw = %d degrees (press to update)",(int)pWpt->getAimYaw());
	else
		sprintf(m_szCaption,"No Waypoint");

	return m_szCaption;
}

void CWaypointYawMenuItem :: activate ( CClient *pClient )
{
	CWaypoint *pWpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

	if ( pWpt )
		pWpt->setAim(CBotGlobals::playerAngles(pClient->getPlayer()).y);
}

void CWaypointAreaIncrease :: activate ( CClient *pClient )
{
	int iWpt = pClient->currentWaypoint();
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);

	if ( pWpt )
	{
		pWpt->setArea(pWpt->getArea()+1);
	}
}

void CWaypointAreaDecrease :: activate ( CClient *pClient )
{
	int iWpt = pClient->currentWaypoint();
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);

	if ( pWpt )
	{
		pWpt->setArea(pWpt->getArea()-1);
	}
}

void CWaypointRadiusIncrease :: activate ( CClient *pClient )
{
	int iWpt = pClient->currentWaypoint();
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);

	if ( pWpt )
	{
		float fRadius = pWpt->getRadius();

		if ( fRadius < 200.0f )
			pWpt->setRadius(fRadius+32.0f);
		else
			pWpt->setRadius(200.0f);
	}
}

void CWaypointRadiusDecrease :: activate ( CClient *pClient )
{
	int iWpt = pClient->currentWaypoint();
	CWaypoint *pWpt = CWaypoints::getWaypoint(iWpt);

	if ( pWpt )
	{
		float fRadius = pWpt->getRadius();

		if ( fRadius > 32.0f )
			pWpt->setRadius(fRadius-32.0f);
		else
			pWpt->setRadius(0.0f);
	}
}


const char *CWaypointCutMenuItem :: getCaption ( CClient *pClient, WptColor &color )
{
	sprintf(m_szCaption,"Cut Waypoint");
	color = WptColor::white;

	return m_szCaption;
}

void CWaypointCutMenuItem :: activate ( CClient *pClient )
{
	pClient->updateCurrentWaypoint();

	CWaypoint *pwpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

	if ( pwpt )
	{
		pClient->setWaypointCut(pwpt);
		CWaypoints::deleteWaypoint(pClient->currentWaypoint());
	}
}

const char *CWaypointCopyMenuItem :: getCaption ( CClient *pClient, WptColor &color )
{
	sprintf(m_szCaption,"Copy Waypoint");
	color = WptColor::white;

	return m_szCaption;
}

void CWaypointCopyMenuItem :: activate ( CClient *pClient )
{
		pClient->updateCurrentWaypoint();

		CWaypoint *pwpt = CWaypoints::getWaypoint(pClient->currentWaypoint());

		if ( pwpt )
		{
			pClient->setWaypointCopy(pwpt);
		}
}

const char *CWaypointPasteMenuItem :: getCaption ( CClient *pClient, WptColor &color )
{
	sprintf(m_szCaption,"Paste Waypoint");
	color = WptColor::white;

	return m_szCaption;
}

void CWaypointPasteMenuItem :: activate ( CClient *pClient )
{
	CWaypoints::addWaypoint(pClient,NULL,NULL,NULL,NULL,true);
}

void CBotMenu ::render (CClient *pClient)
{
	CBotMenuItem *item;
	WptColor color;
	unsigned int i;
	Vector vOrigin;
	Vector vForward;
	Vector vRight;
	QAngle angles;
	const char *pszCaption;
	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo(pClient->getPlayer());
	CBotCmd lastCmd = pPlayerInfo->GetLastUserCommand();
	extern ConVar rcbot_menu_update_time1;
	float fUpdateTime = rcbot_menu_update_time1.GetFloat();

	angles = lastCmd.viewangles;

	vOrigin = pPlayerInfo->GetAbsOrigin();

	AngleVectors(angles,&vForward);

	vForward = vForward / vForward.Length();

	vRight = vForward.Cross(Vector(0,0,1));

	vOrigin = vOrigin + (vForward * 100) - (vRight * 100);
	vOrigin.z += 72.0f;

	pszCaption = getCaption(pClient,color);

	debugoverlay->AddTextOverlayRGB(vOrigin,0,fUpdateTime,color.r,color.g,color.b,color.a,pszCaption);
	debugoverlay->AddTextOverlayRGB(vOrigin,1,fUpdateTime,color.r,color.g,color.b,color.a,"----------------");
/*
	Vector screen;
	Vector point = Vector(0,0,0);

	debugoverlay->ScreenPosition(0.5f, 0.5f, screen);
	debugoverlay->ScreenPosition(point,screen);*/

	for ( i = 0; i < m_MenuItems.size(); i ++ )
	{
		item = m_MenuItems[i];

		pszCaption = item->getCaption(pClient,color);

		debugoverlay->AddTextOverlayRGB(vOrigin,i+2,fUpdateTime,color.r,color.g,color.b,color.a,"%d. %s",(i==9)?(0):(i+1),pszCaption);
	}
}

const char *CBotMenuItem :: getCaption ( CClient *pClient, WptColor &color )
{
	color.r = 255;
	color.g = 255;
	color.b = 255;
	color.a = 255;

	return m_szCaption;
}

void CBotMenuList :: render ( CClient *pClient ) // render
{
	CBotMenu *pMenu = pClient->getCurrentMenu();

	pMenu->render(pClient);
	//m_MenuList[iMenu]->render(pClient);
}

void CBotMenuList :: selectedMenu ( CClient *pClient, unsigned int iMenu )
{
	CBotMenu *pMenu = pClient->getCurrentMenu();

	pMenu->selectedMenu(pClient, iMenu);
}

void CBotMenu :: activate ( CClient *pClient )
{
	pClient->setCurrentMenu(this);
}

void CBotMenu :: selectedMenu ( CClient *pClient, unsigned int iMenu )
{
	if ( iMenu < m_MenuItems.size() )
		m_MenuItems[iMenu]->activate(pClient);
}

CWaypointFlagShowMenu :: CWaypointFlagShowMenu (CBotMenu *pParent)
{
	int iMod = CBotGlobals::getCurrentMod()->getModId();
	// check the number of waypoint types available
	// caption
	// 1.
	// 2.
	// 3.
	// 4.
	// 5.
	// 6.
	// 7.
	// 8. More...
	// 9. Go Back

	int iNumTypes = CWaypointTypes::getNumTypes();
	int iNumAdded = 0;
	CBotMenu *pCurrent;

	int i;

	pCurrent = this;

	for ( i = 0; i < iNumTypes; i ++ )
	{
		if ( !CWaypointTypes::getTypeByIndex(i)->forMod(iMod) )
			continue;

		pCurrent->addMenuItem(new CWaypointFlagShowMenuItem(i));
		iNumAdded++;

		if ( (iNumAdded > 7) || (i == (iNumTypes-1)) )
		{
			CBotMenuItem *back = new CBotGotoMenuItem("Back...",pParent);
		//	make a new menu
			pParent = pCurrent;

			if ( (iNumAdded > 7) && (i < (iNumTypes-1)) )
			{
				pCurrent = new CBotMenu();
				pCurrent->setCaption("Show Waypoint Flags (More)");
				pParent->addMenuItem(new CBotGotoMenuItem("More...",pCurrent));
				
			}

			pParent->addMenuItem(back);

			iNumAdded = 0; // reset

		}
	}
}

const char *CWaypointFlagShowMenu::getCaption(CClient *pClient,WptColor &color )
{
	if ( pClient->isShowingAllWaypoints() )
	{
		sprintf(m_szCaption,"Showing All Waypoints (change)");
		color = WptColor::white;
	}
	else
	{
		sprintf(m_szCaption,"Showing Only Some Waypoints (change)");
		color = CWaypointTypes::getColour(pClient->getShowWaypointFlags());
	}

	return m_szCaption;
}


const char *CWaypointFlagShowMenuItem :: getCaption ( CClient *pClient, WptColor &color )
{
	CWaypointType *type = CWaypointTypes::getTypeByIndex(m_iFlag);

	color = type->getColour();

	sprintf(m_szCaption,"[%s] %s",(pClient->isShowingAllWaypoints()||pClient->isShowingWaypoint(type->getBits()))?"showing":"hiding",type->getName());

	return m_szCaption;
}

void CWaypointFlagShowMenuItem::activate ( CClient *pClient )
{
	CWaypointType *type = CWaypointTypes::getTypeByIndex(m_iFlag);

	// toggle
	if ( pClient->isShowingWaypoint(type->getBits()) )
		pClient->dontShowWaypoints(type->getBits());
	else
		pClient->showWaypoints(type->getBits());
}

void CBotMenuItem :: freeMemory ()
{
	// do nothing
}

void CBotMenu :: freeMemory ()
{
	for ( unsigned int i = 0; i < m_MenuItems.size(); i ++ )
	{
		CBotMenuItem *temp = m_MenuItems[i];

		temp->freeMemory();

		delete temp;
	}
}

void CBotMenuList :: freeMemory ()
{
	for ( unsigned int i = 0; i < BOT_MENU_MAX; i ++ )
	{
		CBotMenu *temp = m_MenuList[i];

		temp->freeMemory();

		delete temp;
	}
}
