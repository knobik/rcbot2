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
#include "bot_mtrand.h"
#include "bot.h"
#include "bot_schedule.h"
#include "bot_task.h"
#include "bot_navigator.h"
#include "bot_waypoint_locations.h"
#include "bot_globals.h"
#include "in_buttons.h"
#include "bot_weapons.h"
#include "bot_hldm_bot.h"
#include "bot_fortress.h"
#include "bot_profiling.h"
#include "bot_getprop.h"

////////////////////////////////////////////////////////////////////////////////////////////
// Tasks

CBotTF2MedicHeal :: CBotTF2MedicHeal ()
{
	m_pHeal = NULL;
}

void CBotTF2MedicHeal::execute(CBot *pBot,CBotSchedule *pSchedule)
{	
	edict_t *pHeal;
	CBotTF2 *pBotTF2;

	pBot->wantToShoot(false);

	if ( !pBot->isTF2() )
	{
		fail();
		return;
	}

	pBotTF2 = (CBotTF2*)pBot;

	pHeal = pBotTF2->getHealingEntity();

	if ( !pHeal )
	{
		// because the medic would have followed this guy, he would have lost his own waypoint
		pBot->getNavigator()->rollBackPosition();
		fail();
	}
	else if ( pBot->getCurrentWeapon() == NULL )
	{
		pBotTF2->clearHealingEntity();
		pBot->getNavigator()->rollBackPosition();
		fail();
	}
	else if ( !pBotTF2->wantToHeal(pHeal) )
	{
		pBotTF2->clearHealingEntity();
		pBot->getNavigator()->rollBackPosition();
		fail();
	}/*
	else if ( !pBot->isVisible(pHeal) )
	{
		pBot->getNavigator()->rollBackPosition();
		((CBotFortress*)pBot)->clearHealingEntity();
		fail();
	}*/
	else if ( pBot->distanceFrom(pHeal) > 416 )
	{
		pBotTF2->clearHealingEntity();
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

		if ( !pBotTF2->healPlayer(pHeal,m_pHeal) )
		{
			pBot->getNavigator()->rollBackPosition();
			pBotTF2->clearHealingEntity();
			fail();
		}
	}

	m_pHeal = pHeal;
}

///////////


CBotTF2ShootLastEnemyPosition :: CBotTF2ShootLastEnemyPosition  ( Vector vPosition, edict_t *pEnemy, Vector m_vVelocity )
{
	float len = m_vVelocity.Length();

	m_vPosition = vPosition;

	if ( len > 0 )
		m_vPosition = m_vPosition - ((m_vVelocity/m_vVelocity.Length())*16);
	
	m_pEnemy = pEnemy;
	m_fTime = 0;
}

void CBotTF2ShootLastEnemyPosition ::  execute (CBot *pBot,CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon = pBot->getCurrentWeapon();
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;
	CWeapon *pChange = NULL;
	CBotWeapon *pChangeTo = NULL;

	if ( m_fTime == 0 )
		m_fTime = engine->Time() + randomFloat(2.0f,4.5f);

	if ( m_fTime < engine->Time() )
	{
		complete();
		return;
	}

	if ( !CBotGlobals::entityIsValid(m_pEnemy) || !CBotGlobals::entityIsAlive(m_pEnemy) )
	{
		complete();
		return;
	}

	if ( pBot->getEnemy() && pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{
		fail();
		return;
	}

	pBot->wantToShoot(false);
	pBot->wantToChangeWeapon(false);
	pBot->wantToListen(false);

	if ( pTF2Bot->getClass()  == TF_CLASS_SOLDIER )
	{
		pChange = CWeapons::getWeapon(TF2_WEAPON_ROCKETLAUNCHER);
	}
	else if ( pTF2Bot->getClass() == TF_CLASS_DEMOMAN )
	{
		pChange = CWeapons::getWeapon(TF2_WEAPON_GRENADELAUNCHER);
	}

	if ( !pChange )
	{
		fail();
		return;
	}

	pChangeTo = pBot->getWeapons()->getWeapon(pChange);
	
	if ( pChangeTo->getAmmo(pBot) < 1 )
	{
		complete();
		return;
	}

	if ( pChangeTo != pWeapon )
	{
		pBot->selectBotWeapon(pChangeTo);
	}
	else
	{
		if ( randomInt(0,1) )
			pBot->primaryAttack(false);
	}

	pBot->setLookVector(m_vPosition);
	pBot->setLookAtTask((LOOK_VECTOR));

}

void CBotTF2ShootLastEnemyPosition :: debugString ( char *string )
{
	sprintf(string,"Shoot Last Enemy Position (%0.4f,%0.4f,%0.4f)",m_vPosition.x,m_vPosition.y,m_vPosition.z);
}


/////////////

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
			pBot->setMoveTo((m_vOrigin));
		else
			pBot->stopMoving();

		if ( pBot->isTF() )
		{
			((CBotTF2*)pBot)->taunt();

			if ( ((CBotTF2*)pBot)->isBeingHealed() )
				complete();
		}
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
		if ( !((CBotFortress*)pBot)->waitForFlag(&m_vOrigin,&m_fWaitTime,m_bFind) )
		{
			fail();
		}
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
			float fdist;

			m_fTime = engine->Time() + randomFloat(5.0,10.0);
			m_vMoveTo = m_vOrigin + Vector(randomFloat(-m_iRadius,m_iRadius),randomFloat(-m_iRadius,m_iRadius),0);
			fdist = pBot->distanceFrom(m_vMoveTo);

			if ( (((CBotTF2*)pBot)->getClass() == TF_CLASS_SPY) && (((CBotTF2*)pBot)->isDisguised()))
			{
				pBot->primaryAttack(); // remove disguise to capture
			}

			((CBotFortress*)pBot)->wantToDisguise(false);

			if ( fdist < 32 )
			{
				pBot->stopMoving();
			}
			else if ( fdist > 400 )
				fail();
			else
			{				
				pBot->setMoveTo((m_vMoveTo));
			}

			pBot->setLookAtTask((LOOK_AROUND));

			if ( ((CBotTF2*)pBot)->checkAttackPoint() )
				complete();
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

////////////////////////
void CPrimaryAttack:: execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	pBot->primaryAttack();
	complete();
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

		if ( pBot->distanceFrom(m_vMoveTo) < 100 )
		{	
			if ( (((CBotTF2*)pBot)->getClass() == TF_CLASS_SPY) && (((CBotTF2*)pBot)->isDisguised()))
				pBot->primaryAttack(); // remove disguise to capture

			((CBotFortress*)pBot)->wantToDisguise(false);

		}
		else
			pBot->setMoveTo((m_vMoveTo));

		pBot->setLookAtTask((LOOK_AROUND));
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
			pBot->setMoveTo((m_vMoveTo));
		else
			pBot->stopMoving();

		pBot->setLookAtTask((LOOK_EDICT));
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
			float fdist;

			m_fTime = engine->Time() + randomFloat(5.0,10.0);
			m_vMoveTo = m_vOrigin + Vector(randomFloat(-m_iRadius,m_iRadius),randomFloat(-m_iRadius,m_iRadius),0);
			fdist = pBot->distanceFrom(m_vMoveTo);

			if ( fdist < 32 )
				pBot->stopMoving();
			else if ( fdist > 400 )
				fail();
			else
			{				
				pBot->setMoveTo((m_vMoveTo));
			}		
		}
		else if ( m_fTime < engine->Time() )
		{
			m_fTime = 0;
		}
		pBot->setLookAtTask((LOOK_SNIPE));
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
			pBot->setMoveTo((CBotGlobals::entityOrigin(m_pBuilding)));
		
		pBot->setLookAtTask((LOOK_EDICT));
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

void CBotHL2DMUseCharger :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	static Vector vOrigin;
	
	if ( m_pCharger.get() == NULL )
	{
		fail();
		return;
	}

	vOrigin = CBotGlobals::entityOrigin(m_pCharger);

	if ( m_fTime == 0.0f )
	{
		m_fTime = engine->Time() + randomFloat(4.0f,6.0f);
	}

	if ( m_fTime < engine->Time() )
		complete();

	if ( CClassInterface::getAnimCycle(m_pCharger) == 1.0f )
		complete();

	if ( ( m_iType == CHARGER_HEALTH ) && ( pBot->getHealthPercent() >= 0.99f ) )
		complete();
	else if ( ( m_iType == CHARGER_ARMOR ) && ( ((CHLDMBot*)pBot)->getArmorPercent() >= 0.99f ) )
		complete();

	pBot->setLookVector(vOrigin);
	pBot->setLookAtTask(LOOK_VECTOR);

	if ( pBot->distanceFrom(m_pCharger) > 96 )
	{
		pBot->setMoveTo(vOrigin);
	}
	else if ( pBot->DotProductFromOrigin(vOrigin) > 0.965925f )
	{
		pBot->use();
	}
}

void CBotGravGunPickup :: execute(CBot *pBot,CBotSchedule *pSchedule)
{
	static Vector vOrigin;
	static Vector vBotOrigin;

	if ( m_fTime == 0.0f )
	{
		m_fSecAttTime = 0;
		m_fTime = engine->Time() + randomFloat(2.0f,4.0f);
	}

	if ( m_fTime < engine->Time() )
	{
		CHLDMBot *HL2DMBot = ((CHLDMBot*)pBot);

		if (HL2DMBot->getFailedObject() && (HL2DMBot->distanceFrom(HL2DMBot->getFailedObject())<=(pBot->distanceFrom(m_Prop)+48)) )
			pBot->primaryAttack();

		HL2DMBot->setFailedObject(m_Prop);

		fail();
		return;
	}

	if ( !CBotGlobals::entityIsValid(m_Prop) || !pBot->isVisible(m_Prop) )
	{
		((CHLDMBot*)pBot)->setFailedObject(m_Prop);
		fail();
		return;
	}

	pBot->wantToChangeWeapon(false);

	vBotOrigin = pBot->getOrigin();
	vOrigin = CBotGlobals::entityOrigin(m_Prop);

	CBotWeapon *pWeapon = pBot->getCurrentWeapon();

	if ( !pWeapon || ( pWeapon->getID() != HL2DM_WEAPON_PHYSCANNON)  )
	{
		if ( !pBot->select_CWeapon(CWeapons::getWeapon(HL2DM_WEAPON_PHYSCANNON)) )
		{
			fail();
		}
	}
	else if ( pBot->distanceFrom(vOrigin) > 100 )
		pBot->setMoveTo(vOrigin);
	else if ( ((vOrigin-vBotOrigin).Length2D() < 16) && (vOrigin.z < vBotOrigin.z) )
		pBot->setMoveTo(vBotOrigin + (vBotOrigin-vOrigin)*100);
	else
		pBot->stopMoving();

	m_Weapon = INDEXENT(pWeapon->getWeaponIndex());

	pBot->setMoveLookPriority(MOVELOOK_OVERRIDE);
	pBot->setLookVector(vOrigin);
	pBot->setLookAtTask(LOOK_VECTOR);
	pBot->setMoveLookPriority(MOVELOOK_TASK);

	if ( pBot->DotProductFromOrigin(vOrigin) > 0.965925f )
	{
		edict_t *pPhys = CClassInterface::gravityGunObject(m_Weapon);

		if ( pPhys == m_Prop.get() )
			complete();
		else if ( pPhys || CClassInterface::gravityGunOpen(m_Weapon) )
		{
			if ( m_fSecAttTime < engine->Time() )
			{
				pBot->secondaryAttack();
				m_fSecAttTime = engine->Time() + randomFloat(0.25,0.75);
			}
		}
	}
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
			pBot->setMoveTo((vOrigin));
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
	else if ( pBot->distanceFrom(m_vOrigin) > 100 )
	{
		pBot->setMoveTo(m_vOrigin);
	}
	else
	{
		pBot->stopMoving();
	}
}

void CBotTF2WaitAmmoTask :: debugString ( char *string )
{
	sprintf(string,"CBotTF2WaitAmmoTask");
}
///////////////////////////
CBotTaskEngiPickupBuilding :: CBotTaskEngiPickupBuilding ( edict_t *pBuilding )
{
	m_pBuilding = pBuilding;
	m_fTime = 0.0f;
}
// move building / move sentry / move disp / move tele
void CBotTaskEngiPickupBuilding :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	CBotWeapon *pWeapon = pBot->getCurrentWeapon();

	if ( m_fTime == 0.0f )
		m_fTime = engine->Time() + 6.0f;

	pBot->wantToShoot(false);
	pBot->lookAtEdict(m_pBuilding.get());
	pBot->setLookAtTask((LOOK_EDICT));

	((CBotTF2*)pBot)->updateCarrying();

	if ( ((CBotTF2*)pBot)->isCarrying() ) //if ( CBotGlobals::entityOrigin(m_pBuilding) == CBotGlobals::entityOrigin(pBot->getEdict()) )
		complete();
	else if ( m_fTime < engine->Time() )
		fail();
	else if ( !pWeapon )
		fail();
	else if ( pWeapon->getID() != TF2_WEAPON_WRENCH )
	{
		if ( !pBot->select_CWeapon(CWeapons::getWeapon(TF2_WEAPON_WRENCH)) )
			fail();
	}
	else if ( pBot->distanceFrom(m_pBuilding) < 100 )
	{
		if ( CBotGlobals::yawAngleFromEdict(pBot->getEdict(),CBotGlobals::entityOrigin(m_pBuilding)) < 25 )
		{	
			pBot->secondaryAttack();
		}
	}
	else
		pBot->setMoveTo((CBotGlobals::entityOrigin(m_pBuilding)));
}
void CBotTaskEngiPickupBuilding :: debugString ( char *string )
{
	sprintf(string,"CBotTaskEngiPickupBuilding");
}

/////////////////
CBotTaskEngiPlaceBuilding :: CBotTaskEngiPlaceBuilding ( eEngiBuild iObject, Vector vOrigin )
{
	m_vOrigin = vOrigin;
	m_fTime = 0.0f;
	m_iState = 1; // BEGIN HERE , otherwise bot will try to destroy the building
	m_iObject = iObject;
	m_iTries = 0;
}

void CBotTaskEngiPlaceBuilding :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( m_fTime == 0.0f )
	{
		// forward
		/*Vector v_start = pBot->getOrigin();
		Vector v_comp = (pBot->getOrigin()-m_vOrigin);
		v_comp = v_comp/v_comp.Length();
		CBotGlobals::traceLine(v_start,v_start + (v_comp*4096.0),MASK_SOLID_BRUSHONLY,&filter);
		trace_t *t = CBotGlobals::getTraceResult();

		v_comp = t->endpos - m_vOrigin;
		
		if ( v_comp.Length() >  )
*/
		m_fTime = engine->Time() + 6.0f;
	}

	pBot->setLookVector(m_vOrigin);
	pBot->setLookAtTask((LOOK_VECTOR));

	((CBotTF2*)pBot)->updateCarrying();

	if ( !(((CBotTF2*)pBot)->isCarrying()) ) 
		complete();
	else if ( m_fTime < engine->Time() )
		fail();
	else if ( pBot->distanceFrom(m_vOrigin) < 100 )
	{		
		if ( CBotGlobals::yawAngleFromEdict(pBot->getEdict(),m_vOrigin) < 25 )
		{	
			int state = ((CBotTF2*)pBot)->engiBuildObject(&m_iState,m_iObject,&m_fTime,&m_iTries);

			if ( state == 1 )
				complete();
			else if ( state == 0 )
				fail();
			
			//pBot->primaryAttack();
		}
	}
	else
		pBot->setMoveTo((m_vOrigin));
	
	if ( pBot->hasEnemy() )
	{
		pBot->primaryAttack();
	}
}
void CBotTaskEngiPlaceBuilding :: debugString ( char *string )
{
	sprintf(string,"CBotTaskEngiPlaceBuilding");
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
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;

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

	pBot->setLookAtTask(LOOK_EDICT);
	pBot->lookAtEdict(pEnemy);

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
		pBot->setMoveTo((vrear));
	}
	else
	{
		// uncloak
		if ( CTeamFortress2Mod::TF2_IsPlayerCloaked(pBot->getEdict()) )
			pTF2Bot->spyUnCloak();
		else
			pTF2Bot->handleAttack(pBotWeapon,pEnemy);
	}
}


////////////////////////////

void CBotDefendTask :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( m_fTime == 0 )
	{
		if ( m_fMaxTime > 0 )
			m_fTime = engine->Time() + m_fMaxTime;
		else
			m_fTime = engine->Time() + randomFloat(20.0f,90.0f);
	}

	pBot->defending();
	
	pBot->stopMoving();

	if ( m_bDefendOrigin )
	{
		pBot->setLookVector(m_vDefendOrigin);
		pBot->setLookAtTask(LOOK_VECTOR);
	}
	else
		pBot->setLookAtTask(LOOK_SNIPE);

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

	pBot->wantToShoot(false); // don't shoot enemies , want to build the damn thing
	pBot->wantToChangeWeapon(false); // if enemy just strike them with wrench

	tfBot = (CBotFortress*)pBot;

	if ( tfBot->getClass() != TF_CLASS_ENGINEER )
	{
		fail();
		return;
	}
	/*else if ( pBot->hasEnemy() && pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{
		fail();
		return;
	}*/

	pBot->setLookAtTask((LOOK_BUILD));
	bAimingOk = pBot->DotProductFromOrigin(pBot->getAiming()) > 0.965925f; // 15 degrees

	if ( m_iObject == ENGI_DISP )
	{
		edict_t *pSentry = tfBot->getSentry();

		if ( pSentry && CBotGlobals::entityIsValid(pSentry) )
		{
			Vector vSentry = CBotGlobals::entityOrigin(pSentry);
			Vector vOrigin = pBot->getOrigin();
			Vector vLookAt = vOrigin - (vSentry - vOrigin);

			pBot->setLookVector(vLookAt);
			pBot->setLookAtTask((LOOK_VECTOR));
			
			//LOOK_VECTOR,11);
			bAimingOk = pBot->DotProductFromOrigin(vLookAt) > 0.965925f; // 15 degrees // < CBotGlobals::yawAngleFromEdict(pBot->getEdict(),pBot->getLookVector()) < 15;
		}
		else
		{
			Vector vSentry = pBot->getAiming();
			Vector vOrigin = pBot->getOrigin();
			Vector vLookAt = vOrigin - (vSentry - vOrigin);

			pBot->setLookVector(vLookAt);
			pBot->setLookAtTask((LOOK_VECTOR));

			/*pBot->setLookVector(pBot->getAiming());
			pBot->setLookAtTask((LOOK_VECTOR));*/
			bAimingOk = pBot->DotProductFromOrigin(vLookAt) > 0.965925f;
		}
	}

	if ( pBot->distanceFrom(m_vOrigin) > 100 )
	{
		if ( !CBotGlobals::isVisible(pBot->getEdict(),pBot->getEyePosition(),m_vOrigin) )
			fail();
		else
			pBot->setMoveTo((m_vOrigin));
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

			//// running path
			//if ( !pBot->hasEnemy() && !pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
			
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
		pBot->setMoveTo((m_vVector));

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
	m_pBuilding = MyEHandle(pBuilding);
	m_id = id;
	m_fHealTime = 0.0f;
}
	
void CBotRemoveSapper :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	//int i = 0;
	edict_t *pBuilding;
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;

	pBot->wantToShoot(false);
	pBot->wantToChangeWeapon(false);

	if ( m_fTime == 0.0f )
		m_fTime = engine->Time() + randomFloat(8.0f,12.0f);

	pBuilding = m_pBuilding.get();

	if ( !pBuilding )
	{
		fail();
		return;
	}

	if ( m_id == ENGI_DISP )
	{
		if ( !CTeamFortress2Mod::isDispenserSapped(pBuilding) )
		{
			if ( m_fHealTime == 0.0f )
				m_fHealTime = engine->Time() + randomFloat(2.0f,3.0f);
			else if ( m_fHealTime < engine->Time() )
			{
				complete();
				return;
			}
		}
		else if ( m_fHealTime > 0.0f )
		{
			fail();
			pTF2Bot->waitRemoveSap();
			return;
		}
	}
	else if ( m_id == ENGI_TELE )
	{
		if ( !CTeamFortress2Mod::isTeleporterSapped(pBuilding) )
		{
			if ( m_fHealTime == 0.0f )
				m_fHealTime = engine->Time() + randomFloat(2.0f,3.0f);
			else if ( m_fHealTime < engine->Time() )
			{
				complete();
				return;
			}
		}
		else if ( m_fHealTime > 0.0f )
		{
			fail();
			pTF2Bot->waitRemoveSap();
			return;
		}
	}
	else if ( m_id == ENGI_SENTRY )
	{
		if ( !CTeamFortress2Mod::isSentrySapped(pBuilding) )
		{
			if ( m_fHealTime == 0.0f )
				m_fHealTime = engine->Time() + randomFloat(2.0f,3.0f);
			else if ( m_fHealTime < engine->Time() )
			{
				complete();
				return;
			}
		}
		else if ( m_fHealTime > 0.0f )
		{
			fail();
			pTF2Bot->waitRemoveSap();
			return;
		}
	}
	
	if ( m_fTime<engine->Time() )
	{
		fail();
	}// Fix 16/07/09
	else if ( !pBot->isVisible(pBuilding) )
	{
		if ( pBot->distanceFrom(pBuilding) > 200 )
			fail();
		else if ( pBot->distanceFrom(pBuilding) > 100 )
			pBot->setMoveTo((CBotGlobals::entityOrigin(pBuilding)));
		
		pBot->setLookAtTask((LOOK_EDICT));
		pBot->lookAtEdict(pBuilding);
	}
	else
	{
		if ( !((CBotFortress*)pBot)->upgradeBuilding(pBuilding,true) )
			fail();
	}
}

////////////////////////////////////////////////////

CBotTF2Snipe :: CBotTF2Snipe ( Vector vOrigin, float fYaw )
{
	QAngle angle;
	m_fTime = 0.0f;
	angle = QAngle(0,fYaw,0);
	AngleVectors(angle,&m_vAim);
	m_vAim = vOrigin + (m_vAim*1024);
	m_vOrigin = vOrigin;
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
	else if ( pBot->distanceFrom(m_vOrigin) > 200 )
	{
		if ( CTeamFortress2Mod::TF2_IsPlayerZoomed(pBot->getEdict()) )
			pBot->secondaryAttack();
		// too far away
		fail();
	}
	else if ( pBot->distanceFrom(m_vOrigin) > 100 )
	{
		pBot->setMoveTo(m_vOrigin);
	}
	else
	{
		pBot->stopMoving();

		if ( pBot->hasEnemy() )
		{
			pBot->setLookAtTask((LOOK_ENEMY));

			// careful that the round may have not started yet
			if ( CTeamFortress2Mod::hasRoundStarted() )
				pBot->handleAttack(pBotWeapon,pBot->getEnemy());
		}
		else
		{
			pBot->setLookAtTask((LOOK_SNIPE));
			pBot->setAiming(m_vAim);
//			pBot->setAiming(m_vAiming);

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
	m_pBuilding = MyEHandle(pBuilding);
	m_fTime = 0.0f;
	m_id = id;
}

void CBotTF2SpySap :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	edict_t *pBuilding;

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

	pBuilding = m_pBuilding.get();

	if ( !pBuilding )
	{
		complete();
		return;
	}

	if ( m_id == ENGI_SENTRY )
	{
		if ( CTeamFortress2Mod::isSentrySapped(pBuilding) )
		{
			complete();
			return;
		}
	}
	else if ( m_id == ENGI_DISP )
	{
		if ( CTeamFortress2Mod::isDispenserSapped(pBuilding) )
		{
			complete();
			return;
		}
	}
	else if ( m_id == ENGI_TELE) 
	{
		if ( CTeamFortress2Mod::isTeleporterSapped(pBuilding) )
		{
			complete();
			return;
		}
	}

	pBot->lookAtEdict(pBuilding);
	pBot->setLookAtTask((LOOK_EDICT));
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
		if ( pBot->distanceFrom(pBuilding) > 100 )
		{
			pBot->setMoveTo((CBotGlobals::entityOrigin(pBuilding)));
		}
		else
		{
			if ( CTeamFortress2Mod::TF2_IsPlayerCloaked(pBot->getEdict()) )
			{
				pBot->secondaryAttack();
				tf2Bot->waitCloak();
			}
			else if ( randomInt(0,1) )
				pBot->tapButton(IN_ATTACK);
			//complete();
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
	{
		// initialize
		m_fTime = engine->Time() + 13.0f;
		m_vLastOrigin = pBot->getOrigin();
	}

	// FIX BUG
	//if ( !((CBotFortress*)pBot)->isTeleporterUseful(m_pTele) )
	//	fail();

	if ( m_fTime < engine->Time() )
	{
		if ( CClients::clientsDebugging(BOT_DEBUG_TASK) )
			CClients::clientDebugMsg(BOT_DEBUG_TASK,"TELEPORT: TIMEOUT",pBot);

		fail();

	}
	else
	{
		if ( CTeamFortress2Mod::getTeleporterExit(m_pTele) ) // exit is still alive?
		{
			Vector vTele = CBotGlobals::entityOrigin(m_pTele);		

			if ( pBot->distanceFrom(vTele) > 48 )
			{
				pBot->setMoveTo((vTele));		
				
				if ( (m_vLastOrigin - pBot->getOrigin()).Length() > 50 )
				{
					pBot->getNavigator()->freeMapMemory(); // restart navigator
				
					complete(); // finished
				}
			}
			else
			{
				pBot->stopMoving();
			}
		
			m_vLastOrigin = pBot->getOrigin();

		}
		else
			fail();
	}
}

void CBotTFUseTeleporter :: debugString ( char *string )
{
	sprintf(string,"CBotTFUseTeleporter %x",(int)m_pTele.get());
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

	pBot->setLookAtTask((LOOK_ENEMY));

	if ( !pBot->handleAttack ( pWeapon, m_pEdict ) )
		fail();
}

///
CThrowGrenadeTask :: CThrowGrenadeTask (int ammo, Vector vLoc )
{
	m_fTime = 0;
	m_vLoc = vLoc + Vector(0,0,48);
	m_fHoldAttackTime = 0;
	m_iAmmo = ammo;
}

void CThrowGrenadeTask ::init()
{
	m_fTime = 0;
}

void CThrowGrenadeTask::debugString(char *string)
{
	sprintf(string,"CThrowGrenadeTask (%0.4f,%0.4f,%0.4f) fTime = %0.4f",m_vLoc.x,m_vLoc.y,m_vLoc.z,m_fTime);	
}
void CThrowGrenadeTask ::execute (CBot *pBot,CBotSchedule *pSchedule)
{
	if ( m_fTime == 0 )
		m_fTime = engine->Time() + 2.0f;

	if ( m_fTime < engine->Time() )
		fail();

	CBotWeapon *pWeapon;

	pWeapon = pBot->getCurrentWeapon();
	pBot->wantToChangeWeapon(false);

	if ( pWeapon && (pWeapon->getID() == HL2DM_WEAPON_PHYSCANNON) && CClassInterface::gravityGunObject(INDEXENT(pWeapon->getWeaponIndex())) )
	{
		// drop it
		if ( randomInt(0,1) )
			pBot->primaryAttack();	
		else
			pBot->secondaryAttack();
	}
	else if ( !pWeapon || (pWeapon->getID() != HL2DM_WEAPON_FRAG) )
	{
		pBot->select_CWeapon(CWeapons::getWeapon(HL2DM_WEAPON_FRAG));
	}
	else
	{
		pBot->setLookVector(m_vLoc);
		pBot->setLookAtTask(LOOK_VECTOR);

		if ( pBot->DotProductFromOrigin(m_vLoc) > 0.98 )
		{
			if ( randomInt(0,1) )
				pBot->primaryAttack();

			if ( pWeapon->getAmmo(pBot) < m_iAmmo )
				complete();
		}
		else if ( pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
		{
			pBot->primaryAttack();
			fail();
		}
	}
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
	setCompleteInterrupt(CONDITION_SEE_CUR_ENEMY);
	m_vLast = vLast+(vVelocity*10);
	m_fTime = 0;
}

void CFindLastEnemy::execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	if ( m_fTime == 0 )
		m_fTime = engine->Time() + randomFloat(2.0,4.0);

	if ( !pBot->moveToIsValid() || pBot->moveFailed() )
		fail();
	if ( pBot->distanceFrom(m_vLast) > 80 )
		pBot->setMoveTo(m_vLast);
	else
		pBot->stopMoving();

	pBot->setLookAtTask((LOOK_AROUND));

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
	pBot->setLookVector(m_vHideFrom);
	pBot->setLookAtTask(LOOK_VECTOR);
	pBot->duck(true);

	if ( m_fHideTime == 0 )
		m_fHideTime = engine->Time() + randomFloat(5.0,10.0);

	if ( m_fHideTime < engine->Time() )
		complete();
}
//////////////////////////////////////////
CBotTF2DemomanPipeTrap :: CBotTF2DemomanPipeTrap ( eDemoTrapType type, Vector vStand, Vector vLoc, Vector vSpread, bool bAutoDetonate)
{
	m_vPoint = vLoc;
	m_vLocation = vLoc;
	m_vSpread = vSpread;
	m_iState = 0;
	m_iStickies = 6;
	m_iTrapType = type;
	m_vStand = vStand;
	m_fTime = 0.0f;
	m_bAutoDetonate = bAutoDetonate;
}
	
void CBotTF2DemomanPipeTrap :: execute (CBot *pBot,CBotSchedule *pSchedule)
{
	bool bFail = false;
	CBotTF2 *pTF2Bot = (CBotTF2*)pBot;

	pBot->wantToChangeWeapon(false);

	if ( pBot->getEnemy() && pBot->hasSomeConditions(CONDITION_SEE_CUR_ENEMY) )
	{
		if ( CTeamFortress2Mod::hasRoundStarted() )
		{
			pBot->secondaryAttack();
			fail();
		}
	}
	
	if ( pTF2Bot->deployStickies(m_iTrapType,m_vStand,m_vLocation,m_vSpread,&m_vPoint,&m_iState,&m_iStickies,&bFail,&m_fTime) )
	{
		complete();

		if ( m_bAutoDetonate )
			pBot->secondaryAttack();
	}

	if ( bFail )
		fail();
}
/////////

CMessAround::CMessAround ( edict_t *pFriendly )
{
	m_fTime = 0.0f;
	m_pFriendly = pFriendly;
	m_iType = randomInt(0,3);
}

void CMessAround::execute ( CBot *pBot, CBotSchedule *pSchedule )
{
	CBotTF2 *pBotTF2 = (CBotTF2*)pBot;

	if ( !m_pFriendly || !CBotGlobals::entityIsValid(m_pFriendly) )
		fail();

	// smack the friendly player with my melee attack
	switch ( m_iType )
	{
	case 0:
		{
		Vector origin = CBotGlobals::entityOrigin(m_pFriendly);
		bool ok = true;

		if ( !pBotTF2->FInViewCone(m_pFriendly) )
		{
			pBotTF2->setLookVector(origin);
			pBotTF2->setLookAtTask((LOOK_VECTOR));
			ok = false;
		}

		if ( pBotTF2->distanceFrom(m_pFriendly) > 100 )
		{
			pBotTF2->setMoveTo((origin));
			ok = false;
		}

		if ( ok )
		{
			CBotWeapon *pWeapon = pBotTF2->getBestWeapon(NULL,true,true);

			if ( pWeapon )
			{
				pBotTF2->selectBotWeapon(pWeapon);

				if ( randomInt(0,1) )
					pBotTF2->primaryAttack();
			}
		}

		if ( !m_fTime )
			m_fTime = engine->Time() + randomFloat(3.5f,8.0f);
	}
	break;// taunt at my friendly player
	case 1:
	{
		Vector origin = CBotGlobals::entityOrigin(m_pFriendly);
		bool ok = true;

		if ( !pBotTF2->FInViewCone(m_pFriendly) )
		{
			pBotTF2->setLookVector(origin);
			pBotTF2->setLookAtTask((LOOK_VECTOR));
			ok = false;
		}

		if ( pBotTF2->distanceFrom(m_pFriendly) > 100 )
		{
			pBotTF2->setMoveTo((origin));
			ok = false;
		}

		if ( ok )
			pBotTF2->taunt();

		if ( !m_fTime )
			m_fTime = engine->Time() + randomFloat(3.5f,6.5f);

	}
	// say some random voice commands
	break;
	case 2:
	{
		if ( !m_fTime )
			pBotTF2->voiceCommand((eVoiceCMD)randomInt(0,31));

		if ( !m_fTime )
			m_fTime = engine->Time() + randomFloat(1.5f,3.0f);
	}
	// press some random buttons, such as attack2, jump
	break;
	case 3:
	{
		if ( randomInt(0,1) )
			pBotTF2->jump();
		else
		{
			if ( pBotTF2->getClass() == TF_CLASS_HWGUY )
				pBotTF2->secondaryAttack(true);
		}

		if ( !m_fTime )
			m_fTime = engine->Time() + randomFloat(1.5f,3.0f);
	}
	default:
		break;
	}

	if ( m_fTime < engine->Time() )
		complete();

	if ( CTeamFortress2Mod::hasRoundStarted() )
		complete();

}

/////////////

void CBotNest :: execute (CBot *pBot, CBotSchedule *pSchedule)
{
	CBotTF2 *pBotTF2 = (CBotTF2*)pBot;

	if ( pBotTF2->someoneCalledMedic() )
		fail();

	if ( !pBotTF2->wantToNest() )
	{
			complete();
			pBotTF2->voiceCommand(TF_VC_GOGOGO);
			return;
	}
	else if ( pBot->hasSomeConditions(CONDITION_PUSH) )
	{
		complete();
		pBot->removeCondition(CONDITION_PUSH);
		pBotTF2->voiceCommand(TF_VC_GOGOGO);
		return;
	}

	if ( m_fTime == 0 )
	{
		m_fTime = engine->Time() + randomFloat(5.0,10.0);

		if ( randomInt(0,1) )
			pBotTF2->voiceCommand(TF_VC_HELP);
	}
	else if ( m_fTime < engine->Time() )
	{
		complete();
		pBotTF2->voiceCommand(TF_VC_GOGOGO);
	}

	// wait around
	// wait for more friendlies
	// heal up
	// 

	pBot->setLookAtTask((LOOK_AROUND));

	pBot->stopMoving();

}

CBotNest::CBotNest()
{
	m_fTime = 0.0f;
	m_pEnemy = NULL;
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
	if ( m_iCompleteInterruptConditionsHave )
	{
		if ( pBot->hasSomeConditions(m_iCompleteInterruptConditionsHave) )
			return STATE_COMPLETE;
	}

	if ( m_iCompleteInterruptConditionsDontHave )
	{
		if ( !pBot->hasAllConditions(m_iCompleteInterruptConditionsDontHave) )
			return STATE_COMPLETE;
	}

	if ( m_iFailInterruptConditionsHave )
	{
		if ( pBot->hasSomeConditions(m_iFailInterruptConditionsHave) )
			return STATE_FAIL;
	}

	if ( m_iFailInterruptConditionsDontHave )
	{
		if ( !pBot->hasAllConditions(m_iFailInterruptConditionsDontHave) )
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
	m_iFailInterruptConditionsHave = 0;
	m_iFailInterruptConditionsDontHave = 0;
	m_iCompleteInterruptConditionsHave = 0;
	m_iCompleteInterruptConditionsDontHave = 0;
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
// if this condition is true it will complete, if bUnset is true, the condition must be false to be complete
void CBotTask :: setCompleteInterrupt ( int iInterruptHave, int iInterruptDontHave )
{
	m_iCompleteInterruptConditionsHave = iInterruptHave;
	m_iCompleteInterruptConditionsDontHave = iInterruptDontHave;
}

void CBotTask :: setFailInterrupt ( int iInterruptHave, int iInterruptDontHave )
{
	m_iFailInterruptConditionsHave = iInterruptHave;
	m_iFailInterruptConditionsDontHave = iInterruptDontHave;
}

void CBotTask :: fail ()
{
	m_iState = STATE_FAIL;
}

void CBotTask :: complete ()
{
	m_iState = STATE_COMPLETE;
}