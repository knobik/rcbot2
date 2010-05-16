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
#include "bot_profiling.h"

////////////////////////////////////////////////////////////////////////////////////////////
// Tasks

CBotTF2MedicHeal :: CBotTF2MedicHeal ()
{
	m_pHeal = NULL;
}

void CBotTF2MedicHeal::execute(CBot *pBot,CBotSchedule *pSchedule)
{	
	edict_t *pHeal;

	pBot->wantToShoot(false);

	if ( !pBot->isTF() )
	{
		fail();
		return;
	}

	pHeal = ((CBotFortress*)pBot)->getHealingEntity();

	if ( !pHeal )
	{
		pBot->getNavigator()->rollBackPosition();
		fail();
	}
	else if ( !CBotGlobals::entityIsValid(pHeal) || !CBotGlobals::entityIsAlive(pHeal) )
	{
		((CBotFortress*)pBot)->clearHealingEntity();
		pBot->getNavigator()->rollBackPosition();
		fail();
	}/*
	else if ( !pBot->isVisible(pHeal) )
	{
		pBot->getNavigator()->rollBackPosition();
		((CBotFortress*)pBot)->clearHealingEntity();
		fail();
	}*/
	else if ( pBot->distanceFrom(pHeal) > 200 )
	{
		((CBotFortress*)pBot)->clearHealingEntity();
		pBot->getNavigator()->rollBackPosition();
		fail();
	}
	else if ( pBot->getCurrentWeapon() == NULL )
	{
		((CBotFortress*)pBot)->clearHealingEntity();
		pBot->getNavigator()->rollBackPosition();
		fail();
	}
	else if ( pBot->getCurrentWeapon()->getWeaponInfo()->getID() != TF2_WEAPON_MEDIGUN )
	{
		pBot->select_CWeapon( CWeapons::getWeapon(TF2_WEAPON_MEDIGUN) );
	}
	else 
	{
		pBot->clearFailedWeaponSelect();

		if ( !((CBotFortress*)pBot)->healPlayer(pHeal,m_pHeal) )
		{
			pBot->getNavigator()->rollBackPosition();
			((CBotFortress*)pBot)->clearHealingEntity();
			fail();
		}


	}


	
m_pHeal = pHeal;
}

CBotTF2WaitHealthTask :: CBotTF2WaitHealthTask ( Vector vOrigin )
{
	m_vOrigin = vOrigin;
	m_fWaitTime = 0;
}

void CBotTF2WaitHealthTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( !m_fWaitTime )
		m_fWaitTime = engine->Time() + randomFloat(5.0f,10.0f);

	 if ( !pBot->hasSomeConditions(CONDITION_NEED_HEALTH) )
		complete();
	 else if ( m_fWaitTime < engine->Time() )
		 fail();
	else
	{
		// TO DO
		/*edict_t *pOtherPlayer = CBotGlobals::findNearestPlayer(m_vOrigin,50.0,pBot->getEdict());

		if ( pOtherPlayer )
		{
			fail();
			return;
		}*/

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
		((CBotFortress*)pBot)->waitForFlag(&m_vOrigin,&m_fWaitTime);
	}
}

void CBotTF2WaitFlagTask :: debugString ( char *string )
{
	sprintf(string,"Wait Flag (%0.4f,%0.4f,%0.4f)",m_vOrigin.x,m_vOrigin.y,m_vOrigin.z);
}
//////////

CBotTF2AttackPoint :: CBotTF2AttackPoint ( int iArea, Vector vOrigin, int iRadius )
{
	m_vOrigin = vOrigin;
	m_fAttackTime = 0;
	m_fTime = 0;
	m_iArea = iArea;
	m_iRadius = iRadius;
}

void CBotTF2AttackPoint :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	vector<int> areas;
	unsigned int i;
	bool found = false;

	((CBotTF2*)pBot)->getAttackArea(&areas);
	
	i = 0;

	while ( (i < areas.size()) && !found )
	{
		found = areas[i] == m_iArea;
		i++;
	}

	if ( !found )	
		complete();
	else if ( m_fAttackTime == 0 )
		m_fAttackTime = engine->Time() + randomFloat(30.0,60.0);
	else if ( m_fAttackTime < engine->Time() )
		complete();
	else
	{
		if ( m_fTime == 0 )
		{
			float fdist = pBot->distanceFrom(m_vMoveTo);
			m_fTime = engine->Time() + randomFloat(5.0,10.0);
			m_vMoveTo = m_vOrigin + Vector(randomFloat(-m_iRadius,m_iRadius),randomFloat(-m_iRadius,m_iRadius),0);

			if ( (((CBotTF2*)pBot)->getClass() == TF_CLASS_SPY) && (((CBotTF2*)pBot)->isDisguised()))
				pBot->primaryAttack(); // remove disguise to capture

			if ( fdist < 32 )
			{
				pBot->stopMoving(5);
			}
			else if ( fdist > 400 )
				fail();
			else
			{				
				pBot->setMoveTo(m_vMoveTo,3);
			}

			pBot->setLookAtTask(LOOK_AROUND,5);
		}
		else if ( m_fTime < engine->Time() )
		{
			m_fTime = 0;
		}
	}
}

void CBotTF2AttackPoint :: debugString ( char *string )
{
	sprintf(string,"CBotTF2AttackPoint (%d,%0.1f,%0.1f,%0.1f,%d)",m_iArea,m_vOrigin.x,m_vOrigin.y,m_vOrigin.z,m_iRadius);
}

////////////////////////////

CBotTF2PushPayloadBombTask :: CBotTF2PushPayloadBombTask (edict_t * pPayloadBomb)
{
	m_pPayloadBomb = pPayloadBomb;
	m_fPushTime = 0;
	m_fTime = 0;
	m_vRandomOffset = Vector(0,0,0);
}

void CBotTF2PushPayloadBombTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( m_fPushTime == 0 )
	{
		m_fPushTime = engine->Time() + randomFloat(10.0,30.0);
		m_vRandomOffset = Vector(randomFloat(-50,50),randomFloat(-50,50),0);
	}
	else if ( m_fPushTime < engine->Time() )
	{
		complete();
	}
	else if(m_pPayloadBomb == NULL)
	{
		complete();
		return;
	}

	else if(CBotGlobals::entityIsValid(m_pPayloadBomb) && CBotGlobals::entityIsAlive(m_pPayloadBomb))
	{

		m_vOrigin = CBotGlobals::entityOrigin(m_pPayloadBomb);
		//m_vMoveTo = m_vOrigin + Vector(randomFloat(-10,10),randomFloat(-10,10),0);
		m_vMoveTo = m_vOrigin + m_vRandomOffset;

		if ( pBot->distanceFrom(m_vMoveTo) < 80 )
		{	
			if ( (((CBotFortress*)pBot)->getClass() == TF_CLASS_SPY) && ((CBotFortress*)pBot)->isDisguised() )
			{
				pBot->primaryAttack();
			}
		}
		else
			pBot->setMoveTo(m_vMoveTo,3);

		pBot->setLookAtTask(LOOK_AROUND,5);
	}
	else
		complete();

}

void CBotTF2PushPayloadBombTask :: debugString ( char *string )
{
	sprintf(string,"CBotTF2PushPayloadBombTask (%0.1f,%0.1f,%0.1f)",m_vOrigin.x,m_vOrigin.y,m_vOrigin.z);
}
////////////////////////////////////////////////////////////////////////

CBotTF2DefendPayloadBombTask :: CBotTF2DefendPayloadBombTask (edict_t * pPayloadBomb)
{
	m_pPayloadBomb = pPayloadBomb;
	m_fDefendTime = 0;
	m_fTime = 0;
	m_vRandomOffset = Vector(0,0,0);
}

void CBotTF2DefendPayloadBombTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( m_fDefendTime == 0 )
	{
		m_fDefendTime = engine->Time() + randomFloat(10.0f,30.0f);
		m_vRandomOffset = Vector(randomFloat(-150.0f,150.0f),randomFloat(-150.0f,150.0f),0);
	}
	else if ( m_fDefendTime < engine->Time() )
	{
		complete();
	}
	else if(m_pPayloadBomb == NULL)
	{
		complete();
		return;
	}
	else if(CBotGlobals::entityIsValid(m_pPayloadBomb) && CBotGlobals::entityIsAlive(m_pPayloadBomb))
	{
		m_vOrigin = CBotGlobals::entityOrigin(m_pPayloadBomb);
		m_vMoveTo = m_vOrigin + m_vRandomOffset;

		if ( pBot->distanceFrom(m_vMoveTo) > 200 )
			pBot->setMoveTo(m_vMoveTo,3);
		else
			pBot->stopMoving(3);

		pBot->setLookAtTask(LOOK_EDICT,3);
		pBot->lookAtEdict(m_pPayloadBomb);
	}
	else
	{
		complete();
	}
}

void CBotTF2DefendPayloadBombTask :: debugString ( char *string )
{
	sprintf(string,"CBotTF2DefendPayloadBombTask (%0.1f,%0.1f,%0.1f)",m_vOrigin.x,m_vOrigin.y,m_vOrigin.z);
}
//////////////////////
CBotTF2DefendPoint :: CBotTF2DefendPoint ( int iArea, Vector vOrigin, int iRadius )
{
	m_vOrigin = vOrigin;
	m_fDefendTime = 0;
	m_fTime = 0;
	m_iArea = iArea;
	m_iRadius = iRadius;
}

void CBotTF2DefendPoint :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	vector<int> areas;
	unsigned int i;
	bool found = false;

	((CBotTF2*)pBot)->getDefendArea(&areas);
	
	i = 0;

	while ( (i < areas.size()) && !found )
	{
		found = areas[i] == m_iArea;
		i++;
	}

	if ( !found )
		complete();
	else if ( m_fDefendTime == 0 )
		m_fDefendTime = engine->Time() + randomFloat(30.0,60.0);
	else if ( m_fDefendTime < engine->Time() )
		complete();
	else
	{
		if ( m_fTime == 0 )
		{
			float fdist = pBot->distanceFrom(m_vMoveTo);
			m_fTime = engine->Time() + randomFloat(5.0,10.0);
			m_vMoveTo = m_vOrigin + Vector(randomFloat(-m_iRadius,m_iRadius),randomFloat(-m_iRadius,m_iRadius),0);

			if ( fdist < 32 )
				pBot->stopMoving(5);
			else if ( fdist > 400 )
				fail();
			else
			{				
				pBot->setMoveTo(m_vMoveTo,3);
			}		
		}
		else if ( m_fTime < engine->Time() )
		{
			m_fTime = 0;
		}
		pBot->setLookAtTask(LOOK_SNIPE,5);
	}
}

void CBotTF2DefendPoint :: debugString ( char *string )
{
	sprintf(string,"CBotTF2DefendPoint (%d,%0.1f,%0.1f,%0.1f,%d)",m_iArea,m_vOrigin.x,m_vOrigin.y,m_vOrigin.z,m_iRadius);
}

///////////
CBotTF2UpgradeBuilding :: CBotTF2UpgradeBuilding ( edict_t *pBuilding )
{
	m_pBuilding = pBuilding;
	m_fTime = 0.0f;
}

void CBotTF2UpgradeBuilding :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	pBot->wantToShoot(false);

	if (!m_fTime )
		m_fTime = engine->Time() + randomFloat(9.0f,11.0f);
	
	if ( m_fTime<engine->Time() )
	{
		complete();
	}// Fix 16/07/09
	else if ( !CBotGlobals::entityIsValid(m_pBuilding) )
	{
		fail();
		return;
	}
	else if ( !pBot->isVisible(m_pBuilding) )
	{
		if ( pBot->distanceFrom(m_pBuilding) > 200 )
			fail();
		else if ( pBot->distanceFrom(m_pBuilding) > 100 )
			pBot->setMoveTo(CBotGlobals::entityOrigin(m_pBuilding),3);
		
		pBot->setLookAtTask(LOOK_EDICT,3);
		pBot->lookAtEdict(m_pBuilding);
	}
	else if ( CBotGlobals::entityIsValid(m_pBuilding) && CBotGlobals::entityIsAlive(m_pBuilding) )
	{
		if ( !((CBotFortress*)pBot)->upgradeBuilding(m_pBuilding) )
			fail();		
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


CBotTF2WaitAmmoTask :: CBotTF2WaitAmmoTask ( Vector vOrigin )
{
	m_vOrigin = vOrigin;
	m_fWaitTime = 0.0f;
}
	
void CBotTF2WaitAmmoTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( !m_fWaitTime )
		m_fWaitTime = engine->Time() + randomFloat(5.0f,10.0f);

	if ( !pBot->hasSomeConditions(CONDITION_NEED_AMMO) )
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

void CBotTF2WaitAmmoTask :: debugString ( char *string )
{
	sprintf(string,"CBotTF2WaitAmmoTask");
}
/////////////////////////////
CBotBackstab :: CBotBackstab (edict_t *_pEnemy)
{
	m_fTime = 0.0f;
	pEnemy = _pEnemy;
}
	
void CBotBackstab ::execute (CBot *pBot,CBotSchedule *pSchedule)
{
	Vector vrear;
	Vector vangles;
	CBotWeapon *pBotWeapon;
	CWeapon *pWeapon;
	CBotFortress *pTF2Bot = (CBotFortress*)pBot;

	pBot->wantToChangeWeapon(false);
	pBot->wantToShoot(false);
	pBot->wantToListen(false);

	pBotWeapon = pBot->getCurrentWeapon();

	if ( !pBotWeapon )
	{
		fail();
		pTF2Bot->waitBackstab();
		return;
	}

	pWeapon = pBotWeapon->getWeaponInfo();

	if (pWeapon == NULL)
	{
		fail();
		pTF2Bot->waitBackstab();
		return;
	}

	if ( !CBotGlobals::isAlivePlayer(pEnemy) )
		fail();

	if ( !m_fTime )
		m_fTime = engine->Time() + randomFloat(5.0f,10.0f);

	if ( m_fTime < engine->Time() )
	{
		fail();
		pTF2Bot->waitBackstab();
		return;
	}
	else if ( !pEnemy || !CBotGlobals::entityIsValid(pEnemy) || !CBotGlobals::entityIsAlive(pEnemy) )
	{
		if ( pBot->getEnemy() && (pEnemy != pBot->getEnemy()) && pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && CBotGlobals::isAlivePlayer(pBot->getEnemy()) )
		{
			pEnemy = pBot->getEnemy();
		}
		else
		{
			fail();
		}

		pTF2Bot->waitBackstab();
		return;
	}
	else if ( !pBot->isVisible(pEnemy) )
	{
		// this guy will do
		if ( pBot->getEnemy() && (pEnemy != pBot->getEnemy()) && pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) && CBotGlobals::isAlivePlayer(pBot->getEnemy()) )
		{
			pEnemy = pBot->getEnemy();
		}
		else
		{
			fail();
		}

		pTF2Bot->waitBackstab();
		return;
	}
	else if (pWeapon->getID() != TF2_WEAPON_KNIFE)
	{
		if ( !pBot->select_CWeapon(CWeapons::getWeapon(TF2_WEAPON_KNIFE)) )
		{
			fail();
			pTF2Bot->waitBackstab();
			return;
		}
	}
	
	AngleVectors(CBotGlobals::entityEyeAngles(pEnemy),&vangles);
	vrear = CBotGlobals::entityOrigin(pEnemy) - (vangles * 45) + Vector(0,0,32);

	if ( pBot->distanceFrom(vrear) > 40 ) 
	{
		pBot->setMoveTo(vrear,8);
	}
	else
	{
		pBot->handleAttack(pBotWeapon,pEnemy);
	}
}


////////////////////////////

void CBotDefendTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( m_fTime == 0 )
		m_fTime = engine->Time() + randomFloat(20.0f,90.0f);
	
	pBot->stopMoving(5);
	pBot->setLookAtTask(LOOK_SNIPE,5);

	if ( m_fTime < engine->Time() )
		complete();
}

//////////////////////
void CBotTF2EngiLookAfter :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	CBotFortress *tfBot = (CBotFortress*)pBot;

	if ( !m_fTime )
	{
		m_fTime = engine->Time() + randomFloat(21.0f,60.0f);
		m_fHitSentry = engine->Time() + randomFloat(1.0f,3.0f);
	}
	else if ( m_fTime < engine->Time() )
		complete();
	else if ( tfBot->lookAfterBuildings(&m_fHitSentry) )
	{
		tfBot->nextLookAfterSentryTime(engine->Time()+randomFloat(20.0f,50.0f));
		complete();
	}
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

	bool bAimingOk = true;

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

	pBot->setLookAtTask(LOOK_BUILD,3);
	bAimingOk = CBotGlobals::yawAngleFromEdict(pBot->getEdict(),pBot->getAiming()) < 15;

	if ( m_iObject == ENGI_DISP )
	{
		edict_t *pSentry = tfBot->getSentry();

		if ( pSentry && CBotGlobals::entityIsValid(pSentry) )
		{
			pBot->setLookVector(pBot->getOrigin() + (pBot->getOrigin()-CBotGlobals::entityOrigin(pSentry)));
			pBot->setLookAtTask(LOOK_VECTOR,4);
			bAimingOk = CBotGlobals::yawAngleFromEdict(pBot->getEdict(),pBot->getLookVector()) < 15;
		}
		else
		{
			pBot->setLookVector(pBot->getOrigin() + (pBot->getOrigin()-pBot->getAiming()));
			pBot->setLookAtTask(LOOK_VECTOR,4);
			bAimingOk = CBotGlobals::yawAngleFromEdict(pBot->getEdict(),pBot->getLookVector()) < 15;
		}
	}

	if ( pBot->distanceFrom(m_vOrigin) > 100 )
	{
		if ( !CBotGlobals::isVisible(pBot->getEdict(),pBot->getEyePosition(),m_vOrigin) )
			fail();
		else
			pBot->setMoveTo(m_vOrigin,2);
	}
	else if ( bAimingOk )
	{
		int state = tfBot->engiBuildObject(&m_iState,m_iObject,&m_fTime,&m_iTries);

		if ( state == 1 )
			complete();
		else if ( state == 0 )
			fail();
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
		pBot->m_fWaypointStuckTime = engine->Time() + randomFloat(7.0f,12.0f);

#ifdef _DEBUG
		CProfileTimer *timer = CProfileTimers::getTimer(BOT_ROUTE_TIMER);

		if ( CClients::clientsDebugging(BOT_DEBUG_PROFILE) )
		{
			timer->Start();
		}
#endif

		if ( pBot->getNavigator()->workRoute(pBot->getOrigin(),m_vVector,&bFail,(m_iInt==0),m_bNoInterruptions) )
			m_iInt = 2;
		else
			m_iInt = 1;

#ifdef _DEBUG
		if ( CClients::clientsDebugging(BOT_DEBUG_PROFILE) )
		{
			timer->Stop();
		}
#endif

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
				pBot->getNavigator()->failMove();
			}

			if ( m_pEdict )
			{
				if ( CBotGlobals::entityIsValid(m_pEdict) )
				{
					if ( pBot->isVisible(m_pEdict) )
					{
						if ( pBot->distanceFrom(m_pEdict) < pBot->distanceFrom(pBot->getNavigator()->getNextPoint()) )
							complete();
					}
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
	fPrevDist = 0;
	//m_vVector = Vector(0,0,0);
	//m_pEdict = NULL;
}

void CMoveToTask :: debugString ( char *string )
{
	sprintf(string,"MoveToTask (%0.4f,%0.4f,%0.4f)",m_vVector.x,m_vVector.y,m_vVector.z);	
}

CMoveToTask :: CMoveToTask ( edict_t *pEdict )
{
	m_pEdict = pEdict;
	m_vVector = CBotGlobals::entityOrigin(m_pEdict);

	//setFailInterrupt(CONDITION_SEE_CUR_ENEMY);
}

void CMoveToTask :: execute ( CBot *pBot, CBotSchedule *pSchedule )
{

	float fDistance;

	fDistance = pBot->distanceFrom(m_vVector);

	// sort out looping move to origins by using previous distance check
	if ( (fDistance < 64) || (fPrevDist&&(fPrevDist < fDistance)) )
	{
		complete();
		return;
	}
	else
	{		
		pBot->setMoveTo(m_vVector,3);

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
	m_fJumpTime = 0.0f;
	m_iState = 0;
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
		CBotTF2 *tf2Bot = ((CBotTF2*)pBot);

		if ( !m_fTime )
		{
			m_fTime = engine->Time()+randomFloat(4.0f,5.0f);
		}

		if ( tf2Bot->rocketJump(&m_iState,&m_fJumpTime) == BOT_FUNC_COMPLETE )
			complete();
		else if ( m_fTime < engine->Time() )
		{
			fail();
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
/////////////////////////////////////////////
CBotRemoveSapper :: CBotRemoveSapper ( edict_t *pBuilding, eEngiBuild id )
{
	m_fTime = 0.0f;
	m_pBuilding = pBuilding;
	m_id = id;
}
	
void CBotRemoveSapper :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	//int i = 0;

	pBot->wantToShoot(false);

	if (!m_fTime )
		m_fTime = engine->Time() + randomFloat(9.0f,11.0f);

	if ( m_id == ENGI_DISP )
	{
		if ( !CTeamFortress2Mod::isDispenserSapped(m_pBuilding) )
		{
			complete();
			return;
		}


	}
	else if ( m_id == ENGI_TELE )
	{
		if ( !CTeamFortress2Mod::isTeleporterSapped(m_pBuilding) )
		{
			complete();
			return;
		}
	}
	else if ( m_id == ENGI_SENTRY )
	{
		if ( !CTeamFortress2Mod::isSentrySapped(m_pBuilding) )
		{
			complete();
			return;
		}
	}
	
	if ( m_fTime<engine->Time() )
	{
		fail();
	}// Fix 16/07/09
	else if ( !CBotGlobals::entityIsValid(m_pBuilding) )
	{
		fail();
		return;
	}
	else if ( !pBot->isVisible(m_pBuilding) )
	{
		if ( pBot->distanceFrom(m_pBuilding) > 200 )
			fail();
		else if ( pBot->distanceFrom(m_pBuilding) > 100 )
			pBot->setMoveTo(CBotGlobals::entityOrigin(m_pBuilding),3);
		
		pBot->setLookAtTask(LOOK_EDICT,3);
		pBot->lookAtEdict(m_pBuilding);
	}
	else
	{

		if ( CBotGlobals::entityIsValid(m_pBuilding) && CBotGlobals::entityIsAlive(m_pBuilding) )
		{
			if ( !((CBotFortress*)pBot)->upgradeBuilding(m_pBuilding) )
				fail();
		}
	
		fail();
	}
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

	pBot->wantToShoot(false);
	pBot->wantToListen(false);

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
		if ( CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->getEdict()) )
			pBot->secondaryAttack();

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
		if ( CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->getEdict()) )
			pBot->secondaryAttack();

		complete();
	}
	else
	{
		pBot->stopMoving(4);

		if ( pBot->hasEnemy() )
		{
			pBot->setLookAtTask(LOOK_ENEMY,6);

			// careful that the round may have not started yet
			if ( CTeamFortress2Mod::hasRoundStarted() )
				pBot->handleAttack(pBotWeapon,pBot->getEnemy());
		}
		else
		{
			pBot->setLookAtTask(LOOK_SNIPE,5);

			if (m_fTime<engine->Time() )
			{
				if ( CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->getEdict()) )
					pBot->secondaryAttack();

				complete();
			}
			else
			{
				if ( !CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->getEdict()) )
					pBot->secondaryAttack();
			}
		}
	}
}

/////////////////////////////////////////////////////

CBotTF2SpySap :: CBotTF2SpySap ( edict_t *pBuilding, eEngiBuild id )
{
	m_pBuilding = pBuilding;
	m_fTime = 0.0f;
	m_id = id;
}

void CBotTF2SpySap :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( !pBot->isTF() )
	{
		fail();
		return;
	}

	if ( !m_fTime )
		m_fTime = engine->Time() + randomFloat(4.0f,6.0f);

	CBotTF2 *tf2Bot = (CBotTF2*)pBot;
	CBotWeapon *weapon;
	pBot->wantToShoot(false);

	if ( tf2Bot->getClass() != TF_CLASS_SPY )
	{
		fail();
		return;
	}

	if ( !m_pBuilding || !CBotGlobals::entityIsValid(m_pBuilding) || !CBotGlobals::entityIsAlive(m_pBuilding) )
	{
		fail();
		return;
	}

	if ( m_id == ENGI_SENTRY )
	{
		if ( CTeamFortress2Mod::isSentrySapped(m_pBuilding) )
		{
			complete();
			return;
		}
	}
	else if ( m_id == ENGI_DISP )
	{
		if ( CTeamFortress2Mod::isDispenserSapped(m_pBuilding) )
		{
			complete();
			return;
		}
	}
	else if ( m_id == ENGI_TELE) 
	{
		if ( CTeamFortress2Mod::isTeleporterSapped(m_pBuilding) )
		{
			complete();
			return;
		}
	}

	pBot->lookAtEdict(m_pBuilding);
	pBot->setLookAtTask(LOOK_EDICT,5);
	weapon = tf2Bot->getCurrentWeapon();

	// time out
	if ( m_fTime < engine->Time() )
		fail();
	else if ( weapon->getID() != TF2_WEAPON_BUILDER )
	{
		helpers->ClientCommand(pBot->getEdict(),"build 3 0");
	}
	else 
	{
		if ( pBot->distanceFrom(m_pBuilding) > 100 )
		{
			pBot->setMoveTo(CBotGlobals::entityOrigin(m_pBuilding));
		}
		else
		{
			pBot->tapButton(IN_ATTACK);
			complete();
		}
	}

}

void CBotTF2SpySap :: debugString ( char *string )
{
	sprintf(string,"sap building");
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
	{
	
		fail();
		return;
	}

	if ( !pBot->isTF() )
	{
		if ( ((CBotFortress*)pBot)->hasFlag() )
		{
			fail();
			return;
		}
	}

	if ( !m_fTime )
		m_fTime = engine->Time() + 12.0f;

	// FIX BUG
	//if ( !((CBotFortress*)pBot)->isTeleporterUseful(m_pTele) )
	//	fail();

	if ( m_fTime < engine->Time() )
		fail();
	else
	{
		Vector vTele = CBotGlobals::entityOrigin(m_pTele);		


		if ( pBot->distanceFrom(vTele) > 48 )
			pBot->setMoveTo(vTele,5);
		else
			pBot->stopMoving(5);

		if ( (m_vLastOrigin - pBot->getOrigin()).Length() > 48 )
		{
			pBot->getNavigator()->freeMapMemory();
			
			complete();
		}
		else
			m_vLastOrigin = pBot->getOrigin();
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
		//pBot->selectWeaponSlot(pWeapon->getWeaponInfo()->getSlot());
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

CFindLastEnemy::CFindLastEnemy (Vector vLast,Vector vVelocity)
{
	m_vLast = vLast+vVelocity;
	m_fTime = 0;
}

void CFindLastEnemy::execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	if ( m_fTime == 0 )
		m_fTime = engine->Time() + randomFloat(2.0,4.0);

	if ( pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
		complete();
	if ( !pBot->moveToIsValid() || pBot->moveFailed() )
		fail();
	if ( pBot->distanceFrom(m_vLast) > 80 )
		pBot->setMoveTo(m_vLast);
	else
		pBot->stopMoving();

	pBot->setLookAtTask(LOOK_AROUND,2);

	if ( m_fTime < engine->Time() )
		complete();
}
////////////////////////////////////////////////////////
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
//////////////////////////////////////////
CBotTF2DemomanPipeTrap :: CBotTF2DemomanPipeTrap ( Vector vLoc, Vector vSpread)
{
	m_vLocation = vLoc;
	m_vSpread = vSpread;
	m_iState = 0;
	m_iStickies = 6;
}
	
void CBotTF2DemomanPipeTrap :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	bool bFail = false;
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;

	pBot->wantToChangeWeapon(false);
	
	pTF2Bot->deployStickies(m_vLocation,m_vSpread,&m_iState,&m_iStickies,&bFail);

	if ( bFail )
		fail();


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