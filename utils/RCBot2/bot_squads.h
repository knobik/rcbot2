#ifndef __RCBOT_SQUADS_H__
#define __RCBOT_SQUADS_H__

#include "vector.h"
#include "bot_genclass.h"
#include "bot_ehandle.h"

#define SQUAD_DEFAULT_SPREAD 80.0// say 50 units between each member...?

typedef enum eSquadForm
{
	SQUAD_FORM_NONE = 0,
	SQUAD_FORM_WEDGE,
	SQUAD_FORM_LINE,
	SQUAD_FORM_ECH_LEFT,
	SQUAD_FORM_ECH_RIGHT,
	SQUAD_FORM_COLUMN,
	SQUAD_FORM_VEE
};

typedef enum eCombatType
{
	COMBAT_NONE = 0,
	COMBAT_STEALTH,
	COMBAT_COMBAT,
	COMBAT_CROUCH,
	COMBAT_PRONE
};

class CBotSquad
{
public:

	CBotSquad ( edict_t *pLeader, edict_t *pMember )
	{
		SetLeader(pLeader);
		AddMember(pMember);

		Init();
	}

	~CBotSquad()
	{
		m_theSquad.Destroy();
	}

	void Init ();

	void ReturnAllToFormation ( void );

	inline void SetLeader ( edict_t *pLeader )
	{
		m_pLeader = pLeader;
	}

	edict_t *getMember ( int iMember );

	void ToggleFireMode ( void )
	{
		bCanFire = !bCanFire;

		/*if ( bCanFire )
			BotPrintTalkMessageOne(m_pLeader.Get(),"Squad can now OPEN FIRE");
		else
			BotPrintTalkMessageOne(m_pLeader.Get(),"Squad is now HOLDING FIRE");*/
	}

	inline bool SquadCanShoot ( void )
	{
		return bCanFire;
	}

	inline bool IsStealthMode ( void )
	{
		return (m_CombatType == COMBAT_STEALTH);
	}

	inline bool IsProneMode ( void )
	{
		return (m_CombatType == COMBAT_PRONE);
	}

	inline bool IsCrouchMode ( void )
	{
		return (m_CombatType == COMBAT_CROUCH);
	}

	inline edict_t *GetLeader ( void )
	{
		return (edict_t*)m_pLeader.get();
	}

	void SetCombatType ( eCombatType iCombatType )
	{
		edict_t *pLeader = GetLeader();

		m_CombatType = iCombatType;

		if ( !pLeader /*|| (pLeader->v.flags & FL_FAKECLIENT)*/ )
			return;

		char szCombatType[16];

		szCombatType[0] = 0;

		switch ( m_CombatType )
		{
		case COMBAT_STEALTH:
			strcpy(szCombatType,"STEALTH");
			break;
		case COMBAT_PRONE:
			strcpy(szCombatType,"PRONE");
            break;
        case COMBAT_CROUCH:
            strcpy(szCombatType,"CROUCH");
            break;
		case COMBAT_NONE:
		case COMBAT_COMBAT:
			strcpy(szCombatType,"NORMAL");
			break;
        }

		//BotPrintTalkMessageOne ( pLeader, "Combat mode is now %s\n", szCombatType );
	}

	void ChangeLeader ( void );

	void removeMember ( edict_t *pMember );

	inline bool IsLeader ( edict_t *pLeader )
	{
		return (GetLeader() == pLeader);
	}

	void AddMember ( edict_t *pEdict );

	int numMembers ();

	bool IsMember ( edict_t *pEdict );

	inline bool isFormation (eSquadForm theFormation)
	{
		return m_theDesiredFormation == theFormation;
	}

	inline void ChangeFormation ( eSquadForm theNewFormation )
	{
		m_theDesiredFormation = theNewFormation;
	}

	inline float GetSpread ( void ) const
	{
		return m_fDesiredSpread;
	}

	inline void ChangeSpread ( float fNewSpread )
	{
		m_fDesiredSpread = fNewSpread;
	}

	int GetFormationPosition ( edict_t *pEdict );

	Vector GetFormationVector ( edict_t *pEdict );

	void UpdateAngles ( void );

private:
	// use 'EHandles' for squads
	// as players might leave and stuff...
	MyEHandle m_pLeader;
	dataStack<MyEHandle> m_theSquad;

	eSquadForm m_theDesiredFormation;
	float m_fDesiredSpread;
	bool bCanFire;

	eCombatType m_CombatType;

	QAngle m_vLeaderAngle;
};

//-------------------

class CBotSquads
{
public:

	static void FreeMemory ( void );

	static void removeSquadMember ( CBotSquad *pSquad, edict_t *pMember );

	// AddSquadMember can have many effects
	// 1. scenario: squad leader exists as squad leader
	//              assign bot to squad
	// 2. scenario: 'squad leader' exists as squad member in another squad
	//              assign bot to 'squad leaders' squad
	// 3. scenario: no squad has 'squad leader' 
	//              make a new squad
	static CBotSquad *AddSquadMember ( edict_t *pLeader, edict_t *pMember );

	static CBotSquad *SquadJoin ( edict_t *pLeader, edict_t *pMember );

	static CBotSquad *FindSquadByLeader ( edict_t *pLeader );

	static void RemoveSquad ( CBotSquad *pSquad );

	static void UpdateAngles ( void );

	static void ChangeLeader ( CBotSquad *theSquad );

private:
	static dataStack<CBotSquad*> m_theSquads;
};
/*
class CBotSquadE
{
public:
	CBotSquadE ()
	{
		m_fIdleTime = 0.0f;
		m_bIsIdle = false;
	}

	addMember ( edict_t *pEdict )
	{
	}

	void think ()
	{

	}

private:
	edict_t 
	float m_fIdleTime;
	bool m_bIsIdle;
	int m_uMembers;
}

class CBotSquadsE
{
public:
	void think ();
private:
	static dataStack<CBotSquad*> m_theSquads;
}
*/
#endif