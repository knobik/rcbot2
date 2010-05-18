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
#ifndef __RCBOT_TASK_H__
#define __RCBOT_TASK_H__

class CBot;
class CBotSchedule;
struct edict_t;

#include "bot.h"
#include "bot_const.h"
#include "bot_fortress.h"

class CBotTask
{
public:	
	CBotTask();	
	void _init();
	virtual void init ();
	virtual void execute ( CBot *pBot, CBotSchedule *pSchedule );

	//void setTimeout ();

	bool hasFailed ();
	bool isComplete ();
	//void setVector ( Vector vOrigin );
	//void setFloat ( float fFloat );
	bool timedOut ();
	//void setEdict ( edict_t *pEdict );
	void setFailInterrupt ( int iInterrupt );
	void setCompleteInterrupt ( int iInterrupt );
	virtual eTaskState isInterrupted (CBot *pBot);
	void fail ();
	void complete ();
	inline bool hasFlag ( int iFlag ) { return (m_iFlags & iFlag) == iFlag; }
	inline void setFlag ( int iFlag ) { m_iFlags |= iFlag; }
	void clearFailInterrupts () { m_iFailInterruptConditions = 0; }	
	virtual void debugString ( char *string ) { string[0] = 0; return; }

	//bool isID ( eTaskID eTaskId ) { };

protected:

	//void setID();
	// flags
	int m_iFlags;
	// conditions that may happen to fail task
	int m_iFailInterruptConditions;
	int m_iCompleteInterruptConditions;
	// current state
	eTaskState m_iState;
	// time out
	float m_fTimeOut;
	// vars
	//edict_t *m_pEdict;
	//float m_fFloat;
	//int m_iInt;
	//Vector m_vVector;
};
///////////////////////////////////

class CFindPathTask : public CBotTask
{
public:
	CFindPathTask ()
	{
		m_bGetPassedVector = false;
		m_pEdict = NULL;
	}

	CFindPathTask ( Vector vOrigin )
	{
		m_vVector = vOrigin;
		m_pEdict = NULL; // no edict
	}

	CFindPathTask ( edict_t *pEdict );

	void getPassedVector () { m_bGetPassedVector = true; }

	void setNoInterruptions () { clearFailInterrupts(); m_bNoInterruptions = true; }

	void execute ( CBot *pBot, CBotSchedule *pSchedule );

	void init ();

	virtual void debugString ( char *string );
private:

	bool m_bNoInterruptions;
	bool m_bGetPassedVector;
	bool m_bDontLookAtWaypoints;
	Vector m_vVector;
	edict_t *m_pEdict;
	int m_iInt;
};

class CBotTF2DemomanPipeTrap : public CBotTask
{
public:
	CBotTF2DemomanPipeTrap ( Vector vLoc, Vector vSpread );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotTF2DemomanPipeTrap");
	}
private:
	Vector m_vLocation;
	Vector m_vSpread;
	float m_fTime;
	int m_iState;
	int m_iStickies;

};

class CBotTF2MedicHeal : public CBotTask
{
public:
	CBotTF2MedicHeal (  );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotTF2MedicHeal");
	}
private:
	edict_t *m_pHeal;

};

class CBotRemoveSapper : public CBotTask
{
public:
	CBotRemoveSapper ( edict_t *pBuilding, eEngiBuild id );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotRemoveSapper");
	}
private:
	float m_fTime;
	float m_fHealTime;
	MyEHandle m_pBuilding;
	eEngiBuild m_id;
};

class CBotBackstab : public CBotTask
{
public:
	CBotBackstab ( edict_t *_pEnemy );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotBackstab");
	}
private:
	float m_fTime;
	edict_t *pEnemy;
};

class CBotDefendTask : public CBotTask
{
public:
	CBotDefendTask ( Vector vOrigin ) { m_vOrigin = vOrigin; m_fTime = 0; setCompleteInterrupt(CONDITION_SEE_CUR_ENEMY); }
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotDefendTask");
	}
private:
	float m_fTime;
	Vector m_vOrigin;
};

class CBotTF2EngiLookAfter : public CBotTask
{
public:
	CBotTF2EngiLookAfter (  ) { m_fTime = 0; m_fHitSentry = 0; }
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotTF2EngiLookAfter");
	}
private:
	float m_fTime;
	float m_fHitSentry;
};

class CBotTF2Snipe : public CBotTask
{
public:
	CBotTF2Snipe (  );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotTF2Snipe");
	}
private:
	float m_fTime;
	Vector m_vAiming;
};

class CBotTF2SpyDisguise : public CBotTask
{
public:
	CBotTF2SpyDisguise (  );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotTF2SpyDisguise");
	}
};

class CBotTF2SapBuilding : public CBotTask
{
public:
	CBotTF2SapBuilding ( edict_t *pBuilding );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	void debugString ( char *string )
	{
		sprintf(string,"CBotTF2SapBuilding");
	}
};

class CBotTFEngiBuildTask : public CBotTask
{
public:
	CBotTFEngiBuildTask ( eEngiBuild iObject, Vector vOrigin );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	virtual void debugString ( char *string );

private:
	Vector m_vOrigin;
	eEngiBuild m_iObject;
	int m_iState;
	float m_fTime;
	int m_iTries;
};

class CBotTF2AttackPoint : public CBotTask
{
public:
	CBotTF2AttackPoint ( int iArea, Vector vOrigin, int iRadius );
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	virtual void debugString ( char *string );
private:
	Vector m_vOrigin;
	Vector m_vMoveTo;
	float m_fAttackTime;
	float m_fTime;
	int m_iArea;
	int m_iRadius;
};

class CBotTF2DefendPoint : public CBotTask
{
public:
	CBotTF2DefendPoint ( int iArea, Vector vOrigin, int iRadius );
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	virtual void debugString ( char *string );
private:
	Vector m_vOrigin;
	Vector m_vMoveTo;
	float m_fDefendTime;
	float m_fTime;
	int m_iArea;
	int m_iRadius;
};

class CBotTF2PushPayloadBombTask : public CBotTask
{
public:
	CBotTF2PushPayloadBombTask (edict_t * pPayloadBomb);
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	edict_t * m_pPayloadBomb;
	Vector m_vMoveTo;
	Vector m_vRandomOffset;
	float m_fPushTime;
	float m_fTime;
	Vector m_vOrigin;
};

class CBotTF2DefendPayloadBombTask : public CBotTask
{
public:
	CBotTF2DefendPayloadBombTask (edict_t * pPayloadBomb);
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	edict_t * m_pPayloadBomb;
	Vector m_vMoveTo;
	Vector m_vRandomOffset;
	float m_fDefendTime;
	float m_fTime;
	Vector m_vOrigin;
};

class CBotTF2UpgradeBuilding : public CBotTask
{
public:
	CBotTF2UpgradeBuilding ( edict_t *pBuilding );
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	virtual void debugString ( char *string );
private:
	edict_t *m_pBuilding;
	float m_fTime;
};

class CBotTF2WaitAmmoTask : public CBotTask
{
public:
	CBotTF2WaitAmmoTask ( Vector vOrigin );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	Vector m_vOrigin;
	float m_fWaitTime;
};

class CBotTF2WaitHealthTask : public CBotTask
{
public:
	CBotTF2WaitHealthTask ( Vector vOrigin );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	Vector m_vOrigin;
	float m_fWaitTime;
};

class CBotTFDoubleJump : public CBotTask
{
public:
	CBotTFDoubleJump (); // going here
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	float m_fTime;
};


class CBotTFRocketJump : public CBotTask
{
public:
	CBotTFRocketJump (); // going here
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	float m_fTime;
	float m_fJumpTime;
	int m_iState;
};

class CBotTF2SpySap : public CBotTask
{
public:
	CBotTF2SpySap ( edict_t *pBuilding, eEngiBuild id ); // going to use this 
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);

	virtual void debugString ( char *string );
private:
	MyEHandle m_pBuilding;
	float m_fTime;
	eEngiBuild m_id;
};

class CBotTFUseTeleporter : public CBotTask
{
public:
	CBotTFUseTeleporter ( edict_t *pTele ); // going to use this 
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	edict_t *m_pTele;
	float m_fTime;
	Vector m_vLastOrigin;
};



class CBotTF2WaitFlagTask : public CBotTask
{
public:
	CBotTF2WaitFlagTask ( Vector vOrigin, bool bFind = false );
	
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	Vector m_vOrigin;
	float m_fWaitTime;
	bool m_bFind;
};

class CAttackEntityTask : public CBotTask
{
public:
	CAttackEntityTask ( edict_t *pEdict );
	void init ();
	void execute (CBot *pBot,CBotSchedule *pSchedule);
	virtual void debugString ( char *string );
private:
	edict_t *m_pEdict;
};

////////////////////
class CAutoBuy : public CBotTask
{
public:
	void init ();

	void execute (CBot *pBot,CBotSchedule *pSchedule);

private:
	float m_fTime;
	bool m_bTimeset;
};
//////////////////////
class CMoveToTask : public CBotTask
{
public:
	CMoveToTask ( Vector vOrigin )
	{
		m_vVector = vOrigin;
		m_pEdict = NULL;

		setFailInterrupt(CONDITION_SEE_CUR_ENEMY);
	}

	CMoveToTask ( edict_t *pEdict );

	void init ();

	void execute ( CBot *pBot, CBotSchedule *pSchedule );

	virtual void debugString ( char *string );
private:
	float fPrevDist;
	Vector m_vVector;
	edict_t *m_pEdict;
};

class CFindLastEnemy : public CBotTask
{
public:
	CFindLastEnemy (Vector vLast,Vector vVelocity);

	void execute ( CBot *pBot, CBotSchedule *pSchedule );

	void debugString ( char *string )
	{
		sprintf(string,"CFindLastEnemy");
	}
private:
	Vector m_vLast;
	float m_fTime;
};

class CFindGoodHideSpot : public CBotTask
{
public:
	CFindGoodHideSpot ( edict_t *pEntity );

	CFindGoodHideSpot ( Vector vec );

	void init ();

	void execute ( CBot *pBot, CBotSchedule *pSchedule );


	void debugString ( char *string )
	{
		sprintf(string,"CFindGoodHideSpot");
	}
private:
	Vector m_vHideFrom;
};

class CHideTask : public CBotTask
{
public:
	CHideTask ( Vector vHideFrom );

	void init ();

	void execute ( CBot *pBot, CBotSchedule *pSchedule );

	virtual void debugString ( char *string );
private:
	Vector m_vHideFrom;
	float m_fHideTime;
};
/*
class CAttackTask : public CBotTask
{
public:
	CAttackTask ( Vector vToAttack )
	{
		setVector(vToAttack);
	}

	CAttackTask ( edict_t *pToAttack )
	{
		setEdict(pToAttack);
	}

	void init ()
	{
		setInterrupt(CONDITION_OUT_OF_AMMO|CONDITION_NO_WEAPON);
	}

	virtual void execute ( CBot *pBot )
	{

	}
};

class CFindRunPath : public CBotTask
{
public:
	CFindRunPath ( Vector pGoto )
	{
		setVector(pGoto);
	}

	CFindRunPath ( edict_t *pGoto )
	{
		setEdict(pGoto);
		//setVector(pGoto->v.origin); ??
	}

	void init ()
	{
		setInterrupt(0);
	}

	virtual void execute ( CBot *pBot );
};*/
#endif