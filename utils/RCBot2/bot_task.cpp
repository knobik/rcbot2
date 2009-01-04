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
//#include "vstdlib/random.h" // for random functions

#include "bot_mtrand.h"
#include "bot.h"
#include "bot_schedule.h"
#include "bot_task.h"
#include "bot_navigator.h"
#include "bot_waypoint_locations.h"
#include "bot_globals.h"
#include "in_buttons.h"
#include "bot_weapons.h"
#include "bot_fortress.h"

////////////////////////////////////////////////////////////////////////////////////////////
// Tasks

CBotTF2MedicHeal :: CBotTF2MedicHeal ()
{

}

void CBotTF2MedicHeal::execute(CBot *pBot,CBotSchedule *pSchedule)
{
	edict_t *m_pHeal;
	
	if ( !pBot->isTF() )
	{
		fail();
		return;
	}

	m_pHeal = ((CBotFortress*)pBot)->getHealingEntity();

	if ( !m_pHeal )
		fail();
	else if ( !CBotGlobals::entityIsValid(m_pHeal) || !CBotGlobals::entityIsAlive(m_pHeal) )
	{
		fail();
	}
	else if ( pBot->getCurrentWeapon() == NULL )
	{
		fail();
	}
	else if ( pBot->getCurrentWeapon()->getWeaponInfo()->getID() != TF2_WEAPON_MEDIGUN )
	{
		pBot->select_CWeapon( CWeapons::getWeapon(TF2_WEAPON_MEDIGUN) );
	}
	else if ( pBot->isVisible(m_pHeal) )
	{
		Vector vOrigin = CBotGlobals::entityOrigin(m_pHeal);

		if ( pBot->distanceFrom(vOrigin) > 200 )
		{
			pBot->setMoveTo(vOrigin,2);
		}
		else
			pBot->stopMoving();

		pBot->lookAtEdict(m_pHeal);
		pBot->setLookAtTask(LOOK_EDICT,2);
		// unselect weapon
		pBot->selectWeapon(0);

		pBot->primaryAttack(true);

		if ( pBot->hasEnemy() && pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
		{
			// uber if ready
			pBot->secondaryAttack();
		}

	}
	else
		fail();
}

CBotTF2WaitHealthTask :: CBotTF2WaitHealthTask ( Vector vOrigin )
{
	m_vOrigin = vOrigin;
	m_fWaitTime = 0;
}

void CBotTF2WaitHealthTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( !m_fWaitTime )
		m_fWaitTime = engine->Time() + 40.0f;

	 if ( pBot->getHealthPercent() > 0.9 )
		complete();
	 else if ( m_fWaitTime < engine->Time() )
		 fail();
	else
	{
		pBot->setLookAtTask(LOOK_AROUND);

		if ( pBot->distanceFrom(m_vOrigin) > 50 )
			pBot->setMoveTo(m_vOrigin,2);
		else
			pBot->stopMoving();

		if ( pBot->isTF() )
			((CBotTF2*)pBot)->taunt();
	}
}

void CBotTF2WaitHealthTask :: debugString ( char *string )
{
	sprintf(string,"Wait Health (%0.4f,%0.4f,%0.4f)",m_vOrigin.x,m_vOrigin.y,m_vOrigin.z);
}


CBotTF2WaitFlagTask :: CBotTF2WaitFlagTask ( Vector vOrigin, bool bFind )
{
	m_vOrigin = vOrigin;
	m_fWaitTime = 0;
	m_bFind = bFind;
}

void CBotTF2WaitFlagTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( !m_fWaitTime )
	{
		if ( m_bFind )
			m_fWaitTime = engine->Time() + 5.0f;
		else
			m_fWaitTime = engine->Time() + 10.0f;
	}

	if ( ((CBotTF2*)pBot)->hasFlag() )
		complete();
	else if ( pBot->getHealthPercent() < 0.2 )
	{
		fail();
	}
	else if ( m_fWaitTime < engine->Time() )
	{
		((CBotFortress*)pBot)->flagReset();
		fail();
	}
	else if ( !pBot->isTF() )
	{
		fail();
	}
	else
	{		
		if ( ((CBotFortress*)pBot)->seeFlag(false) != NULL )
		{
			edict_t *m_pFlag = ((CBotFortress*)pBot)->seeFlag(false);

			if ( CBotGlobals::entityIsValid(m_pFlag) )
			{
				pBot->lookAtEdict(m_pFlag);
				pBot->setLookAtTask(LOOK_EDICT,2);
				m_vOrigin = CBotGlobals::entityOrigin(m_pFlag);
				m_fWaitTime = engine->Time() + 5.0f;
			}
			else
				((CBotFortress*)pBot)->seeFlag(true);
		}
		else
			pBot->setLookAtTask(LOOK_AROUND);

		if ( pBot->distanceFrom(m_vOrigin) > 48 )
			pBot->setMoveTo(m_vOrigin,2);
		else
		{
			if ( (((CBotFortress*)pBot)->getClass() == TF_CLASS_SPY) && ((CBotFortress*)pBot)->isDisguised() )
			{
				pBot->primaryAttack();
			}
			pBot->stopMoving(2);
		}

		((CBotTF2*)pBot)->taunt();
	
	}
}

void CBotTF2WaitFlagTask :: debugString ( char *string )
{
	sprintf(string,"Wait Flag (%0.4f,%0.4f,%0.4f)",m_vOrigin.x,m_vOrigin.y,m_vOrigin.z);
}

///////////
CBotTF2UpgradeBuilding :: CBotTF2UpgradeBuilding ( edict_t *pBuilding )
{
	m_pBuilding = pBuilding;
	m_fTime = 0.0f;
}

void CBotTF2UpgradeBuilding :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if (!m_fTime )
		m_fTime = engine->Time() + randomFloat(9.0f,11.0f);
	
	if ( m_fTime<engine->Time() )
		complete();
	else if ( CBotGlobals::entityIsValid(m_pBuilding) && CBotGlobals::entityIsAlive(m_pBuilding) )
	{
		Vector vOrigin = CBotGlobals::entityOrigin(m_pBuilding);

		CBotWeapon *pWeapon = pBot->getCurrentWeapon();

		if ( !pWeapon )
			fail();
		else if ( pWeapon->getID() != TF2_WEAPON_WRENCH )
		{
			if ( !pBot->select_CWeapon(CWeapons::getWeapon(TF2_WEAPON_WRENCH)) )
				fail();
		}
		else if ( pBot->distanceFrom(vOrigin) > 100 )
		{
			pBot->setMoveTo(vOrigin,3);			
		}
		else
		{
			pBot->primaryAttack();
		}

		pBot->lookAtEdict(m_pBuilding);
		pBot->setLookAtTask(LOOK_EDICT,3);
		
	}
	else
		fail();
}

void CBotTF2UpgradeBuilding:: debugString ( char *string )
{
	sprintf(string,"CBotTF2UpgradeBuilding");
}

///////////////////////////////////////////////////////////////////////
/*
// Protect SG from Enemy
CBotTFEngiTankSentry :: CBotTFEngiTankSentry ( edict_t *pSentry, edict_t *pEnemy )
{
	m_pEnemy = pEnemy;
	m_pSentry = pSentry;
}

void CBotTFEngiTankSentry :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	
	CBotFortress *tfBot;

	if ( !pBot->isTF() )
	{
		fail();
		return;
	}
	
	tfBot = (CBotFortress*)pBot;


	if ( !CBotGlobals::entityIsAlive(m_pEnemy) )
		complete();
	else if ( !CBotGlobals::entityIsAlive(m_pSentry) || !CBotGlobals::entityIsValid(m_pSentry) || !CTeamFortress2Mod::isSentry(m_pSentry) )
		fail();
	else
	{
		Vector vOrigin;
		Vector vComp;

		vComp = CBotGlobals::entityOrigin(m_pEnemy) - CBotGlobals::entityOrigin(m_pSentry);
		vComp = vComp / vComp.Length(); // Normalise

		// find task position behind sentry
		vOrigin = CBotGlobals::entityOrigin(m_pSentry) - (vComp*80);

		if ( pBot->distanceFrom(vOrigin) > 32  )
		{
			// get into position!
			pBot->setMoveTo(vOrigin,2);
		}
		else
		{
			if ( !pBot->currentWeapon("tf_wrench") )
				pBot->selectWeaponName("tf_wrench");
			else
			{
				setTaskEntity();
				pBot->setLookAt(TSK_ENTITY,2);

				// Tank!!!
				pBot->duck();
				pBot->tapButton(IN_ATTACK);
			}

		}
	}
		
}
*/

////////////////////////

////////////////////////


CBotTF2WaitAmmoTask :: CBotTF2WaitAmmoTask ( int iWeaponId, Vector vOrigin )
{
	m_vOrigin = vOrigin;
	m_fWaitTime = 0.0f;
	m_iWeaponId = iWeaponId;
}
	
void CBotTF2WaitAmmoTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon;
	CBotWeapons *pWeapons;

	pWeapons = pBot->getWeapons();

	if ( !m_fWaitTime )
		m_fWaitTime = engine->Time() + randomFloat(10.0f,15.0f);

	if ( !pWeapons )
		fail();
	else 
	{
		pWeapon = pWeapons->getWeapon(CWeapons::getWeapon(m_iWeaponId));

		if (!pWeapon )
			fail();
		else
		{
			if ( pWeapon->getAmmo(pBot) >= 200 )
			{
				complete();
			}
			else if ( m_fWaitTime < engine->Time() )
				fail();
			else
			{
				pBot->stopMoving(2);
			}
		}
	}
}

void CBotTF2WaitAmmoTask :: debugString ( char *string )
{
	sprintf(string,"CBotTF2WaitAmmoTask");
}




////////////////////////
CBotTFEngiBuildTask :: CBotTFEngiBuildTask ( eEngiBuild iObject, Vector vOrigin )
{
	m_iObject = iObject;
	m_vOrigin = vOrigin;
	m_iState = 0;
	m_fTime = 0;
	m_iTries = 0;
}
	
void CBotTFEngiBuildTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	CBotFortress *tfBot;

	if ( !pBot->isTF() )
		fail();

	tfBot = (CBotFortress*)pBot;

	if ( tfBot->getClass() != TF_CLASS_ENGINEER )
	{
		fail();
		return;
	}
	else if ( pBot->hasEnemy() && pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{
		fail();
		return;
	}

	if ( pBot->distanceFrom(m_vOrigin) > 100 )
	{
		if ( !CBotGlobals::isVisible(pBot->getEdict(),pBot->getEyePosition(),m_vOrigin) )
			fail();
		else
			pBot->setMoveTo(m_vOrigin,2);
	}
	else
	{
		if ( m_iState == 0 )
		{
			if ( tfBot->hasEngineerBuilt(m_iObject) )
			{
				tfBot->engineerBuild(m_iObject,ENGI_DESTROY);
			}

			m_iState = 1;
		}
		else if ( m_iState == 1 )
		{
			// unselect current weapon
			tfBot->selectWeapon(0);
			tfBot->engineerBuild(m_iObject,ENGI_BUILD);
			m_iState ++;
		}
		else if ( m_iState == 2 )
		{
			pBot->tapButton(IN_ATTACK2);
			m_iState ++;
		}
		else if ( m_iState == 3 )
		{
			pBot->tapButton(IN_ATTACK2);
			m_iState ++;
		}
		else if ( m_iState == 4 )
		{
			pBot->tapButton(IN_ATTACK);

			m_fTime = engine->Time() + 1.0f;

			m_iState++;
		}
		else if ( m_iState == 5 )
		{
			// Check if sentry built OK
			// Wait for Built object message

			if ( tfBot->hasEngineerBuilt(m_iObject) )
			{
				m_iState++;				
				// OK, set up whacking time!
				m_fTime = engine->Time() + randomFloat(5.0f,12.0f);
			}
			else if ( m_fTime < engine->Time() )
			{
				if ( m_iTries > 3 )
					fail();
				else
				{
					m_iState = 4;
					m_iTries ++;
				}
			}
		}
		else
		{
			// whack it for a while
			if ( m_fTime < engine->Time() )
				complete();
			else
				pBot->tapButton(IN_ATTACK);
		}
	}
}

void CBotTFEngiBuildTask :: debugString ( char *string )
{
	sprintf(string,"CBotTFEngiBuildTask (%d,%0.4f,%0.4f,%0.4f)",m_iObject,m_vOrigin.x,m_vOrigin.y,m_vOrigin.z);
}


///////////////////////////////////////////////////////

CFindGoodHideSpot :: CFindGoodHideSpot ( edict_t *pEntity )		
{
	m_vHideFrom = CBotGlobals::entityOrigin(pEntity);
}

CFindGoodHideSpot :: CFindGoodHideSpot ( Vector vec )
{
	m_vHideFrom = vec;
}

void CFindGoodHideSpot :: init ()
{
	// not required, should have been constructed properly
	//m_vHideFrom = Vector(0,0,0);
}

void CFindGoodHideSpot :: execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	Vector vFound;

	if ( !pBot->getNavigator()->getHideSpotPosition(m_vHideFrom,&vFound) )
		fail();
	else
	{		
		pSchedule->passVector( vFound );
		complete();
	}
}

void CFindPathTask :: init ()
{
	m_bNoInterruptions = false;
	m_bGetPassedVector = false;
	m_iInt = 0;
	
	//setFailInterrupt(CONDITION_SEE_CUR_ENEMY);
}

CFindPathTask :: CFindPathTask ( edict_t *pEdict )
{
	m_pEdict = pEdict;
	m_vVector = CBotGlobals::entityOrigin(pEdict);
}

void CFindPathTask :: debugString ( char *string )
{
	sprintf(string,"Find Path (%d) (%0.4f,%0.4f,%0.4f)",m_iInt,m_vVector.x,m_vVector.y,m_vVector.z);
}

void CFindPathTask :: execute ( CBot *pBot, CBotSchedule *pSchedule )
{	
	bool bFail = false;

	if ( pSchedule->hasPassVector() )
	{
		m_vVector = pSchedule->passedVector();
		pSchedule->clearPass();
	}

	if ( (m_iInt == 0) || (m_iInt == 1) )
	{
		if ( pBot->getNavigator()->workRoute(pBot->getOrigin(),m_vVector,&bFail,(m_iInt==0),m_bNoInterruptions) )
			m_iInt = 2;
		else
			m_iInt = 1;

		pBot->debugMsg(BOT_DEBUG_NAV,"Trying to work out route");
	}

	if ( bFail )
	{
		pBot->debugMsg(BOT_DEBUG_NAV,"Route failed");
		fail();
	}
	else if ( m_iInt == 2 )
	{		
		if ( m_bNoInterruptions )
		{
			pBot->debugMsg(BOT_DEBUG_NAV,"Found route");
			complete(); // ~fin~
		}

		if ( !pBot->getNavigator()->hasNextPoint() )
		{
			pBot->debugMsg(BOT_DEBUG_NAV,"Nowhere to go");
			complete(); // reached goal
		}
		else
		{			
			

			if ( pBot->moveFailed() )
			{
				pBot->debugMsg(BOT_DEBUG_NAV,"moveFailed() == true");
				fail();
			}

			if ( m_pEdict )
			{
				if ( CBotGlobals::entityIsValid(m_pEdict) )
				{
					if ( pBot->distanceFrom(m_pEdict) < pBot->distanceFrom(pBot->getNavigator()->getNextPoint()) )
						complete();
				}
				else
					fail();
			}

			// running path
			pBot->setLookAtTask(LOOK_WAYPOINT);
		}
	}
}

void CMoveToTask :: init () 
{ 
	fPrevDist = CWaypointLocations::HALF_MAX_MAP_SIZE; 
	m_vVector = Vector(0,0,0);
	m_pEdict = NULL;
}

void CMoveToTask :: debugString ( char *string )
{
	sprintf(string,"MoveToTask (%0.4f,%0.4f,%0.4f)",m_vVector.x,m_vVector.y,m_vVector.z);	
}

void CMoveToTask :: execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	static Vector vOrigin = Vector(0,0,0);

	float fDistance;
	
	if ( m_pEdict != NULL ) // going to an edict?
	{
		vOrigin = CBotGlobals::entityOrigin(m_pEdict);
	}

	fDistance = pBot->distanceFrom(vOrigin);

	// sort out looping move to origins by using previous distance check
	if ( (fDistance < BOT_WPT_TOUCH_DIST) || (fPrevDist > fDistance) )
	{
		complete();
		return;
	}
	else
	{		
		pBot->setMoveTo(vOrigin,2);

		if ( pBot->moveFailed() )
			fail();
	}

	fPrevDist = fDistance;
}
////////////////////////////////////////////////////



///////////////////////////////////////////////////

CBotTFRocketJump :: CBotTFRocketJump ()
{
	m_fTime = 0.0f;
}

void CBotTFRocketJump :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	CBotWeapon *pBotWeapon;
	CWeapon *pWeapon;

	pBotWeapon = pBot->getCurrentWeapon();

	if ( !pBotWeapon )
	{
		fail();
		return;
	}

	pWeapon = pBotWeapon->getWeaponInfo();

	if (pWeapon == NULL)
	{
		fail();
		return;
	}

	if ( !pBot->isTF() || (((CBotFortress*)pBot)->getClass() != TF_CLASS_SOLDIER) || (pBot->getHealthPercent() < 0.3) )
	{
		fail();
	}
	else if (pWeapon->getID() != TF2_WEAPON_ROCKETLAUNCHER )
	{
		if ( !pBot->select_CWeapon(CWeapons::getWeapon(TF2_WEAPON_ROCKETLAUNCHER)) )
		{
			fail();
		}
	}
	else
	{
		if ( !m_fTime )
		{
			m_fTime = engine->Time()+randomFloat(0.3f,0.6f);
		}

		pBot->setLookAtTask(LOOK_GROUND,4);

		if ( m_fTime < engine->Time() )
		{
			pBot->jump();
			pBot->primaryAttack();
			complete();
		}
	}
}

void CBotTFRocketJump :: debugString ( char *string )
{
	sprintf(string,"CBotTFRocketJump");
}


//////////////////////////////////////////////////////

CBotTFDoubleJump :: CBotTFDoubleJump ()
{
	m_fTime = 0.0f;
}

void CBotTFDoubleJump ::execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( m_fTime == 0.0f )
	{
		pBot->tapButton(IN_JUMP);

		m_fTime = engine->Time() + 0.4;
	}
	else if ( m_fTime < engine->Time() )
	{
		pBot->jump();
		complete();
	}
}

void CBotTFDoubleJump :: debugString ( char *string )
{
	sprintf(string,"CbotTFDoublejump");
}
////////////////////////////////////////////////////

CBotTF2Snipe :: CBotTF2Snipe (  )
{
	m_fTime = 0.0f;
}
	
void CBotTF2Snipe :: execute (CBot *pBot,CBotSchedule *pSchedule)
{

	CBotWeapon *pBotWeapon;
	CWeapon *pWeapon;

	if ( m_fTime == 0.0f )
	{
		m_fTime = engine->Time() + randomFloat(40.0f,90.0f);
		pBot->secondaryAttack();
	}

	pBotWeapon = pBot->getCurrentWeapon();

	if ( !pBotWeapon )
	{
		fail();
		return;
	}

	pWeapon = pBotWeapon->getWeaponInfo();

	if (pWeapon == NULL)
	{
		fail();
		return;
	}

	if ( !pBot->isTF() || (((CBotFortress*)pBot)->getClass() != TF_CLASS_SNIPER) || (pBot->getHealthPercent() < 0.2) )
	{
		fail();
	}
	else if (pWeapon->getID() != TF2_WEAPON_SNIPERRIFLE )
	{
		if ( !pBot->select_CWeapon(CWeapons::getWeapon(TF2_WEAPON_SNIPERRIFLE)) )
		{
			fail();
		}
	}
	else if ( pBotWeapon->getAmmo(pBot) < 1 )
	{
		pBot->secondaryAttack();
		complete();
	}
	else
	{
		//if ( !CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->getEdict()) )
		//	pBot->secondaryAttack();

		pBot->stopMoving(2);
		pBot->setLookAtTask(LOOK_SNIPE);

		if (m_fTime<engine->Time() )
		{
			pBot->secondaryAttack();
			complete();
		}
	}
}


/////////////////////////////////////////////////////
CBotTFUseTeleporter :: CBotTFUseTeleporter ( edict_t *pTele )
{// going to use this 
	
	m_pTele = pTele;
	m_fTime = 0.0;
}

void CBotTFUseTeleporter :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( !m_pTele || !CBotGlobals::entityIsValid(m_pTele) )
		fail();

	if ( !m_fTime )
		m_fTime = engine->Time() + 12.0f;

	if ( m_fTime < engine->Time() )
		fail();
	else
	{
		Vector vTele = CBotGlobals::entityOrigin(m_pTele);

		if ( pBot->distanceFrom(vTele) > 50 )
			pBot->setMoveTo(vTele,3);
		else
			pBot->stopMoving(3);

		if ( (m_vLastOrigin - pBot->getOrigin()).Length() > 100 )
		{
			pBot->updatePosition();
			complete();
		}
	}
}

void CBotTFUseTeleporter :: debugString ( char *string )
{
	sprintf(string,"CBotTFUseTeleporter %x",(int)m_pTele);
}

///////////////////////////////////////////////////

CAttackEntityTask :: CAttackEntityTask ( edict_t *pEdict )
{
	m_pEdict = pEdict;
}

void CAttackEntityTask :: debugString ( char *string )
{
	int id = -1;

	if ( m_pEdict )
      id = ENTINDEX(m_pEdict);

	sprintf(string,"CAttackEntityTask (%d)",id);	
}

void CAttackEntityTask :: init ()
{
	//setFailInterrupt ( CONDITION_ENEMY_OBSCURED );
	//setCompleteInterrupt ( CONDITION_ENEMY_DEAD );
}

void CAttackEntityTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon;

	if ( m_pEdict == NULL )
	{
		fail();
		return;
	}

	if ( !pBot->isEnemy(m_pEdict) )
	{
		complete();
		return;
	}

	if ( !pBot->isVisible(m_pEdict) )
	{
		fail();
		return;
	}

	if ( pBot->hasSomeConditions(CONDITION_ENEMY_DEAD) )
	{
		fail();
		return;
	}

	pWeapon = pBot->getBestWeapon(m_pEdict);

	if ( (pWeapon != NULL) && (pWeapon != pBot->getCurrentWeapon()) && pWeapon->getWeaponIndex() )
	{
		pBot->selectWeapon(pWeapon->getWeaponIndex());
	}

	pBot->setEnemy(m_pEdict);

	pBot->setLookAtTask(LOOK_ENEMY,2);

	if ( !pBot->handleAttack ( pWeapon, m_pEdict ) )
		fail();
}

///

void CAutoBuy :: init () 
{ 
	m_bTimeset = false;
}

void CAutoBuy :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( !m_bTimeset )
	{
		m_bTimeset = true;
		m_fTime = engine->Time() + randomFloat(2.0,4.0);
	}
	else if ( m_fTime < engine->Time() )
	{
		engine->SetFakeClientConVarValue(pBot->getEdict(),"cl_autobuy","m4a1 ak47 famas galil p90 mp5 primammo secammo defuser vesthelm vest");
		//helpers->ClientCommand(pBot->getEdict(),"setinfo cl_autobuy \"m4a1 ak47 famas galil p90 mp5 primammo secammo defuser vesthelm vest\"\n");
		helpers->ClientCommand(pBot->getEdict(),"autobuy\n");	
		complete();
	}
}

CFindLastEnemy::CFindLastEnemy ()
{
}

void CFindLastEnemy::execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	if ( pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
		complete();
	if ( !pBot->moveToIsValid() || pBot->moveFailed() )
		fail();
	pBot->setLookAtTask(LOOK_LAST_ENEMY);
}

CHideTask :: CHideTask( Vector vHideFrom )
{
	m_vHideFrom = vHideFrom;
}


void CHideTask :: debugString ( char *string )
{
	sprintf(string,"CHideTask (%0.4f,%0.4f,%0.4f)",m_vHideFrom.x,m_vHideFrom.y,m_vHideFrom.z);	
}

void CHideTask :: init ()
{
	m_fHideTime = 0;	
}

void CHideTask :: execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	pBot->stopMoving();	
	pBot->setLookAt(m_vHideFrom);
	pBot->duck(true);

	if ( m_fHideTime == 0 )
		m_fHideTime = engine->Time() + randomFloat(5.0,10.0);

	if ( m_fHideTime < engine->Time() )
		complete();
}

////////////////////////////////////////////////////////////////////////////////////////////
// Base Task
CBotTask :: CBotTask ()
{
	_init();
}

bool CBotTask :: timedOut ()
{
	return (this->m_fTimeOut != 0) && (engine->Time() < this->m_fTimeOut);
}

eTaskState CBotTask :: isInterrupted (CBot *pBot)
{
	if ( m_iCompleteInterruptConditions )
	{
		if ( pBot->hasSomeConditions(m_iCompleteInterruptConditions) )
			return STATE_COMPLETE;
	}

	if ( m_iFailInterruptConditions )
	{
		if ( pBot->hasSomeConditions(m_iFailInterruptConditions) )
			return STATE_FAIL;
	}

	return STATE_RUNNING;
}

void CBotTask :: _init()
{
	m_iFlags = 0;	
	m_iState = STATE_IDLE;
	m_fTimeOut = 0;
//	m_pEdict = NULL;
//	m_fFloat = 0;
//	m_iInt = 0;
//	m_vVector = Vector(0,0,0);
	m_iFailInterruptConditions = 0;
	m_iCompleteInterruptConditions = 0;
	init();
}

void CBotTask :: init ()
{
	return;
}

void CBotTask :: execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	return;
}

bool CBotTask :: hasFailed ()
{
	return m_iState == STATE_FAIL;
}

bool CBotTask :: isComplete ()
{
	return m_iState == STATE_COMPLETE;
}
/*
void CBotTask :: setVector ( Vector vOrigin )
{
	m_vVector = vOrigin;
}

void CBotTask :: setFloat ( float fFloat )
{
	m_fFloat = fFloat;
}

void CBotTask :: setEdict ( edict_t *pEdict )
{
	m_pEdict = pEdict;
}
*/
void CBotTask :: setCompleteInterrupt ( int iInterrupt )
{
	m_iCompleteInterruptConditions = iInterrupt;
}

void CBotTask :: setFailInterrupt ( int iInterrupt )
{
	m_iFailInterruptConditions = iInterrupt;
}

void CBotTask :: fail ()
{
	m_iState = STATE_FAIL;
}

void CBotTask :: complete ()
{
	m_iState = STATE_COMPLETE;
}