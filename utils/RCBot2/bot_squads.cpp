
#include "mathlib.h"
#include "vector.h"

#include "eiface.h"

#include "bot_const.h"
#include "bot.h"
#include "bot_globals.h"
#include "bot_squads.h"
#include "bot_getprop.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

dataStack<CBotSquad*> CBotSquads::m_theSquads;

class CRemoveBotFromSquad : public IBotFunction
{
public:

	CRemoveBotFromSquad ( CBotSquad *pSquad )
	{
		m_pSquad = pSquad;
	}

	virtual void execute ( CBot *pBot )
	{
		if ( pBot->inSquad(m_pSquad) )
			pBot->clearSquad();
	}

private:
	CBotSquad *m_pSquad;
};

//-------------

void CBotSquads::FreeMemory ( void )
{
	CBotSquad *pSquad;
	dataStack<CBotSquad*> tempStack = m_theSquads;

	while ( !tempStack.IsEmpty() )
	{
		pSquad = tempStack.ChooseFromStack();

		if ( pSquad )
			delete pSquad;
		pSquad = NULL;
	}

	m_theSquads.Destroy();
}

void CBotSquads::removeSquadMember ( CBotSquad *pSquad, edict_t *pMember )
{
	pSquad->removeMember(pMember);

	if ( pSquad->numMembers() <= 1 )
	{
		RemoveSquad(pSquad);
	}
}

// AddSquadMember can have many effects
// 1. scenario: squad leader exists as squad leader
//              assign bot to squad
// 2. scenario: 'squad leader' exists as squad member in another squad
//              assign bot to 'squad leaders' squad
// 3. scenario: no squad has 'squad leader' 
//              make a new squad
CBotSquad *CBotSquads::AddSquadMember ( edict_t *pLeader, edict_t *pMember )
{
	dataStack<CBotSquad*> tempStack = m_theSquads;
	CBotSquad *theSquad;
	CBot *pBot;

	//char msg[120];

	if ( !pLeader )
		return NULL;

	if ( CClassInterface::getTeam(pLeader) != CClassInterface::getTeam(pMember) )
		return NULL;

	//CClient *pClient = gBotGlobals.m_Clients.GetClientByEdict(pLeader);

	//if ( pClient )
	//{
	//	pClient->AddNewToolTip(BOT_TOOL_TIP_SQUAD_HELP);
	//}

	//sprintf(msg,"%s %s has joined your squad",BOT_DBG_MSG_TAG,STRING(pMember->v.netname));
	//ClientPrint(pLeader,HUD_PRINTTALK,msg);
	
	while ( !tempStack.IsEmpty() )
	{
		theSquad = tempStack.ChooseFromStack();
		
		if ( theSquad->IsLeader(pLeader) )
		{
			theSquad->AddMember(pMember);
			tempStack.Init();
			return theSquad;
		}
		else if ( theSquad->IsMember(pLeader) )
		{
			theSquad->AddMember(pMember);
			tempStack.Init();
			return theSquad;
		}
	}
	
	// no squad with leader, make one
	
	theSquad = new CBotSquad(pLeader,pMember);
	
	if ( theSquad != NULL )
	{
		m_theSquads.Push(theSquad);
		
		if ( (pBot = CBots::getBotPointer(pLeader)) != NULL )
			pBot->setSquad(theSquad);
	}
	
	return theSquad;
}

CBotSquad *CBotSquads::FindSquadByLeader ( edict_t *pLeader )
{
	CBotSquad *pSquad;
	dataStack<CBotSquad*> tempStack = m_theSquads;

	while ( !tempStack.IsEmpty() )
	{
		pSquad = tempStack.ChooseFromStack();

		if ( pSquad->IsLeader(pLeader) )
		{
			tempStack.Init();
			return pSquad;
		}
	}

	return NULL;
}

void CBotSquads::RemoveSquad ( CBotSquad *pSquad )
{
	CRemoveBotFromSquad *func = new CRemoveBotFromSquad(pSquad);
	CBots::botFunction(func);
	delete func;
	
	m_theSquads.Remove(pSquad);
	
	if ( pSquad != NULL )
		delete pSquad;
}

void CBotSquads::UpdateAngles ( void )
{
	CBotSquad *pSquad;
	dataStack<CBotSquad*> tempStack = m_theSquads;

	pSquad = NULL;

	while ( !tempStack.IsEmpty() )
	{
		try
		{
			pSquad = tempStack.ChooseFromStack();
			pSquad->UpdateAngles();
		}

		catch ( ... )
		{
			// Arghhhhh

			m_theSquads.Remove(pSquad);
			tempStack = m_theSquads;
		}
	}
}

//-------------

void CBotSquad::UpdateAngles ( void )
{
	edict_t *pLeader = GetLeader();

	Vector velocity;

	CClassInterface::getVelocity(pLeader,&velocity);

	if ( velocity.Length2D() > 1.0f )
	{
		VectorAngles(velocity,m_vLeaderAngle);
	}
}

void CBotSquad::Init ()
{
	edict_t *pLeader;

	m_theDesiredFormation = SQUAD_FORM_WEDGE; // default wedge formation
	m_fDesiredSpread = SQUAD_DEFAULT_SPREAD; 

	m_CombatType = COMBAT_COMBAT;

	bCanFire = true;

	if ( (pLeader = GetLeader()) != NULL )
	{
		IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pLeader);
		m_vLeaderAngle = p->GetLastUserCommand().viewangles;
	}
}

void CBotSquad::ChangeLeader ( void )
{
	if ( m_theSquad.IsEmpty() )
	{
		SetLeader(NULL);
	}
	else
	{
		m_pLeader = m_theSquad.Pop();

		if ( m_theSquad.IsEmpty() )
			SetLeader(NULL);
		else
		{
			Init(); // new squad init
		}
	}
}

Vector CBotSquad :: GetFormationVector ( edict_t *pEdict )
{
	Vector vLeaderOrigin;
	Vector vBase; 
	Vector v_forward;
	Vector v_right;
	QAngle angle_right;
	// vBase = first : offset from leader origin without taking into consideration spread and position
	int iPosition;
	trace_t *tr = CBotGlobals::getTraceResult();

	edict_t *pLeader = GetLeader();
	
	iPosition = GetFormationPosition(pEdict);
	vLeaderOrigin = CBotGlobals::entityOrigin(pLeader);

	int iMod = iPosition % 2;

	AngleVectors(m_vLeaderAngle,&v_forward); // leader body angles as base

	angle_right = m_vLeaderAngle;
	angle_right.y += 90.0f;

	CBotGlobals::fixFloatAngle(&(angle_right.y));

	AngleVectors(angle_right,&v_right); // leader body angles as base

	// going to have members on either side.
	switch ( m_theDesiredFormation ) 
	{
	case SQUAD_FORM_VEE:
		{
			if ( iMod )			
				vBase = (v_forward-v_right);			
			else
				vBase = (v_forward+v_right);
		}
		break;
	case SQUAD_FORM_WEDGE:
		{
			if ( iMod )			
				vBase = -(v_forward-v_right);			
			else
				vBase = -(v_forward+v_right);
		}
		break;
	case SQUAD_FORM_LINE:
		{

			// have members on either side of leader

			if ( iMod )			
				vBase = v_right;			
			else
				vBase = -v_right;
		}
		break;
	case SQUAD_FORM_COLUMN:
		{
			vBase = -v_forward;
		}
		break;
	case SQUAD_FORM_ECH_LEFT:
		{
			vBase = -v_forward - v_right;
		}
		break;
	case SQUAD_FORM_ECH_RIGHT:
		{
			vBase = -v_forward + v_right;
		}
		break;
	}
	
	vBase = (vBase * m_fDesiredSpread) * iPosition;

	CBotGlobals::quickTraceline(pLeader,vLeaderOrigin,vLeaderOrigin+vBase);

	if ( tr->fraction < 1.0 )
	{
		return vLeaderOrigin + (vBase*tr->fraction*0.5f);
	}

	return vLeaderOrigin+vBase;
}

int CBotSquad::GetFormationPosition ( edict_t *pEdict )
{
	int iPosition = 0;

	dataStack<MyEHandle> tempStack = m_theSquad;

	while ( !tempStack.IsEmpty() )
	{
		iPosition++;			

		if ( pEdict == tempStack.ChooseFromStack().get() )
		{				
			/// !! musssst init
			tempStack.Init();
			return iPosition;
		}			
	}

	return 0;
}

void CBotSquad::removeMember ( edict_t *pMember )
{
	dataStack<MyEHandle> tempStack;
	MyEHandle *temp;

	tempStack = m_theSquad;

	while ( !tempStack.IsEmpty() )
	{
		temp = tempStack.ChoosePointerFromStack();

		if ( temp->get() == pMember )
		{
			m_theSquad.RemoveByPointer(temp);
			tempStack.Init();
			return;
		}
	}
}

void CBotSquad::AddMember ( edict_t *pEdict )
{
	if ( !IsMember(pEdict) )
	{
		MyEHandle newh;

		newh = pEdict;

		m_theSquad.Push(newh);
	}
}

int CBotSquad::numMembers ()
{
	dataStack<MyEHandle> tempStack;
	int num = 0;

	tempStack = m_theSquad;

	while ( !tempStack.IsEmpty() )
	{
		tempStack.ChooseFromStack();

		num++;
	}

	return num;
}

void CBotSquad :: ReturnAllToFormation ( void )
{
	dataStack<MyEHandle> tempStack = this->m_theSquad;
	edict_t *pMember;
	CBot *pBot;

	while ( !tempStack.IsEmpty() )
	{
		pMember = tempStack.ChooseFromStack().get();

		pBot = CBots::getBotPointer(pMember);
			
		if ( pBot )
		{
			pBot->removeCondition(CONDITION_PUSH);
			pBot->removeCondition(CONDITION_COVERT);
			pBot->updateCondition(CONDITION_CHANGED);
			pBot->setSquadIdleTime(0.0f);
		}
    }
}

bool CBotSquad::IsMember ( edict_t *pEdict )
{
	dataStack<MyEHandle> tempStack;
	MyEHandle temp;

	tempStack = m_theSquad;

	while ( !tempStack.IsEmpty() )
	{
		temp = tempStack.ChooseFromStack();

		if ( temp.get() == pEdict )
		{
			tempStack.Init();
			return true;
		}
	}

	return false;
}

