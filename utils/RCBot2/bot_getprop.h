#ifndef __RCBOT_GETPROP_H__
#define __RCBOT_GETPROP_H__

typedef enum
{
	GETPROP_UNDEF = -1,
	GETPROP_TF2SCORE = 0,
	GETPROP_ENTITY_FLAGS,
	GETPROP_TEAM,
	GETPROP_PLAYERHEALTH,
	GETPROP_EFFECTS,
	GETPROP_AMMO,
	GETPROP_TF2_NUMHEALERS,
	GETPROP_TF2_CONDITIONS,
	GETPROP_VELOCITY,
	GETPROP_TF2CLASS,
	GETPROP_TF2SPYMETER,// CTFPlayer::
	GETPROP_TF2SPYDISGUISED_TEAM,//CTFPlayer::m_nDisguiseTeam
	GETPROP_TF2SPYDISGUISED_CLASS,//CTFPlayer::m_nDisguiseClass
	GETPROP_TF2SPYDISGUISED_TARGET_INDEX,//CTFPlayer::m_iDisguiseTargetIndex
	GETPROP_TF2SPYDISGUISED_DIS_HEALTH,//CTFPlayer::m_iDisguiseHealth
 	GETPROP_TF2MEDIGUN_HEALING,
	GETPROP_TF2MEDIGUN_TARGETTING,
	//SETPROP_SET_TICK_BASE,
	GETPROP_TF2TELEPORTERMODE,
	GETPROP_CURRENTWEAPON,
	GETPROP_TF2UBERCHARGE_LEVEL,
	GETPROP_TF2SENTRYHEALTH,
	GETPROP_TF2DISPENSERHEALTH,
	GETPROP_TF2TELEPORTERHEALTH,
	GETPROP_TF2OBJECTCARRIED,
	GETPROP_TF2OBJECTUPGRADELEVEL,
	GETPROP_TF2OBJECTMAXHEALTH,
	GETPROP_TF2DISPMETAL,
	GETPROP_MAXSPEED,
	GETPROP_CONSTRAINT_SPEED,
	GETPROP_TF2OBJECTBUILDING,
	GETPROP_HL2DM_PHYSCANNON_ATTACHED,
	GETPROP_HL2DM_PHYSCANNON_OPEN,
	GETPROP_HL2DM_PLAYER_AUXPOWER,
	GETPROP_HL2DM_LADDER_ENT,
	GETPROP_WEAPONLIST,
	GETPROP_WEAPONSTATE,
	GETPROP_WEAPONCLIP1,
	GETPROP_WEAPONCLIP2,
	GETPROP_WEAPON_AMMOTYPE1,
	GETPROP_WEAPON_AMMOTYPE2,
	GETPROP_DOD_PLAYERCLASS,
	GETPROP_DOD_DES_PLAYERCLASS,
	GETPROP_DOD_STAMINA,
	GETPROP_DOD_PRONE,
	GET_PROPDATA_MAX
}getpropdata_id;

bool UTIL_FindSendPropInfo(ServerClass *pInfo, const char *szType, unsigned int *offset);
ServerClass *UTIL_FindServerClass(const char *name);
void UTIL_FindServerClassPrint(const char*name_cmd);

class CClassInterfaceValue
{
public:
	CClassInterfaceValue ()
	{
		m_data = NULL; 
		m_class = NULL;
		m_value = NULL;
		m_offset = 0;
	}

	CClassInterfaceValue ( char *key, char *value, unsigned int preoffset )
	{
		init(key,value,preoffset);
	}

	void init ( char *key, char *value, unsigned int preoffset = 0 );

	void findOffset ( );

	void getData ( edict_t *edict );

	edict_t *getEntity ( edict_t *edict );

	CBaseHandle *getEntityHandle ( edict_t *edict );

	inline bool getBool ( edict_t *edict, bool defaultvalue ) 
	{ 
		getData(edict);  
		
		if ( !m_data ) 
			return defaultvalue; 
		
		return *((bool*)m_data); 
	}

	inline float getFloat ( edict_t *edict, float defaultvalue ) 
	{ 
		getData(edict); 
		
		if ( !m_data ) 
			return defaultvalue; 
		
		return *((float*)m_data); 
	}

	inline float *getFloatPointer ( edict_t *edict, float defaultvalue ) 
	{ 
		getData(edict); 
		
		if ( !m_data ) 
			return NULL; 
		
		return ((float*)m_data); 
	}

	inline char *getString (edict_t *edict ) 
	{ 
		getData(edict); 

		return (char*)m_data; 
	}

	inline bool getVector ( edict_t *edict, Vector *v )
	{
		static float *x;

		getData(edict);

		if ( m_data )
		{
			x = (float*)m_data;
			*v = Vector(*x,*(x+1),*(x+2));

			return true;
		}

		return false;
	}

	inline int getInt ( edict_t *edict, int defaultvalue ) 
	{ 
		getData(edict); 
		
		if ( !m_data ) 
			return defaultvalue; 
		
		return *((int*)m_data);
	}

	inline int *getIntPointer ( edict_t *edict ) 
	{ 
		getData(edict); 

		return (int*)m_data; 
	}

	inline float getFloatFromInt ( edict_t *edict, float defaultvalue )
	{
		getData(edict); 

		if ( !m_data ) 
			return defaultvalue; 

		return (float)(*(int *)m_data);
	}

	static void resetError () { m_berror = false; }
	static bool isError () { return m_berror; }

private:
	unsigned int m_offset;
	unsigned int m_preoffset;
	void *m_data;
	char *m_class;
	char *m_value;

	static bool m_berror;
};


extern CClassInterfaceValue g_GetProps[GET_PROPDATA_MAX];

#define DEFINE_GETPROP(id,classname,value,preoffs)\
 g_GetProps[id] = CClassInterfaceValue( CClassInterfaceValue ( classname, value, preoffs ) );

class CClassInterface
{
public:
	static void init ();

	// TF2
	static int getTF2Score ( edict_t *edict );
	inline static int getFlags ( edict_t *edict ) { return g_GetProps[GETPROP_ENTITY_FLAGS].getInt(edict,0); }
	inline static int getTeam ( edict_t *edict ) { return g_GetProps[GETPROP_TEAM].getInt(edict,0); }
	inline static float getPlayerHealth ( edict_t *edict ) { return g_GetProps[GETPROP_PLAYERHEALTH].getFloatFromInt(edict,0); }
	inline static int getEffects ( edict_t *edict ) { return g_GetProps[GETPROP_EFFECTS].getInt(edict,0); }
	inline static int *getAmmoList ( edict_t *edict ) { return g_GetProps[GETPROP_AMMO].getIntPointer(edict); }
	//static unsigned int findOffset(const char *szType,const char *szClass);
	inline static int getTF2NumHealers ( edict_t *edict ) { return g_GetProps[GETPROP_TF2_NUMHEALERS].getInt(edict,0); }
	inline static int getTF2Conditions ( edict_t *edict ) { return g_GetProps[GETPROP_TF2_CONDITIONS].getInt(edict,0); }
	inline static bool getVelocity ( edict_t *edict, Vector *v ) {return g_GetProps[GETPROP_VELOCITY].getVector(edict,v); }
	inline static int getTF2Class ( edict_t *edict ) { return g_GetProps[GETPROP_TF2CLASS].getInt(edict,0); }
	inline static float getTF2SpyCloakMeter ( edict_t *edict ) { return g_GetProps[GETPROP_TF2SPYMETER].getFloat(edict,0); }
	static bool getTF2SpyDisguised( edict_t *edict, int *_class, int *_team, int *_index, int *_health ) 
	{ 
		CClassInterfaceValue::resetError();
		*_team = g_GetProps[GETPROP_TF2SPYDISGUISED_TEAM].getInt(edict,0); 
		*_class = g_GetProps[GETPROP_TF2SPYDISGUISED_CLASS].getInt(edict,0); 
		*_index = g_GetProps[GETPROP_TF2SPYDISGUISED_TARGET_INDEX].getInt(edict,0); 
		*_health = g_GetProps[GETPROP_TF2SPYDISGUISED_DIS_HEALTH].getInt(edict,0);
		return !CClassInterfaceValue::isError();
	}
	inline static bool getMedigunHealing ( edict_t *edict ) { return g_GetProps[GETPROP_TF2MEDIGUN_HEALING].getBool(edict,false); }
	inline static edict_t *getMedigunTarget ( edict_t *edict ) { return g_GetProps[GETPROP_TF2MEDIGUN_TARGETTING].getEntity(edict); }
	inline static bool isMedigunTargetting ( edict_t *pgun, edict_t *ptarget) { return (g_GetProps[GETPROP_TF2MEDIGUN_TARGETTING].getEntity(pgun) == ptarget); }
	//static void setTickBase ( edict_t *edict, int tickbase ) { return ;
	inline static int isTeleporterMode (edict_t *edict, eTeleMode mode ) { return (g_GetProps[GETPROP_TF2TELEPORTERMODE].getInt(edict,-1) == (int)mode); }
	inline static edict_t *getCurrentWeapon (edict_t *player) { return g_GetProps[GETPROP_CURRENTWEAPON].getEntity(player); }
	inline static int getUberChargeLevel (edict_t *pWeapon) { return (int)(g_GetProps[GETPROP_TF2UBERCHARGE_LEVEL].getFloat(pWeapon,0)*100.0); }
	//static void test ();
	inline static float getSentryHealth ( edict_t *edict ) { return g_GetProps[GETPROP_TF2SENTRYHEALTH].getFloatFromInt(edict,100); }
	inline static float getDispenserHealth ( edict_t *edict ) { return g_GetProps[GETPROP_TF2DISPENSERHEALTH].getFloatFromInt(edict,100); }
	inline static float getTeleporterHealth ( edict_t *edict ) { return g_GetProps[GETPROP_TF2TELEPORTERHEALTH].getFloatFromInt(edict,100); }
	inline static bool isObjectCarried ( edict_t *edict ) { return g_GetProps[GETPROP_TF2OBJECTCARRIED].getBool(edict,false); }
	inline static int getTF2UpgradeLevel ( edict_t *edict ) { return g_GetProps[GETPROP_TF2OBJECTUPGRADELEVEL].getInt(edict,0); }
	inline static int getTF2GetBuildingMaxHealth ( edict_t *edict ) { return g_GetProps[GETPROP_TF2OBJECTMAXHEALTH].getInt(edict,0); }
	inline static int getTF2DispMetal ( edict_t *edict ) { return g_GetProps[GETPROP_TF2DISPMETAL].getInt(edict,0); }
	inline static float getMaxSpeed(edict_t *edict) { return g_GetProps[GETPROP_MAXSPEED].getFloat(edict,0); }
	inline static float getSpeedFactor(edict_t *edict) { return g_GetProps[GETPROP_CONSTRAINT_SPEED].getFloat(edict,0); } 
	inline static bool isObjectBeingBuilt(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECTBUILDING].getBool(edict,false); }
	inline static edict_t *gravityGunObject(edict_t *pgun) { return g_GetProps[GETPROP_HL2DM_PHYSCANNON_ATTACHED].getEntity(pgun); }
	inline static bool gravityGunOpen(edict_t *pgun) { return g_GetProps[GETPROP_HL2DM_PHYSCANNON_OPEN].getBool(pgun,false); }
	inline static float auxPower (edict_t *player) { return g_GetProps[GETPROP_HL2DM_PLAYER_AUXPOWER].getFloat(player,0);} 
	inline static edict_t *onLadder ( edict_t *player ) { return g_GetProps[GETPROP_HL2DM_LADDER_ENT].getEntity(player);}
	inline static CBaseHandle *getWeaponList ( edict_t *player ) { return g_GetProps[GETPROP_WEAPONLIST].getEntityHandle(player);}
	inline static int getWeaponState ( edict_t *pgun ) { return g_GetProps[GETPROP_WEAPONSTATE].getInt(pgun,0); }

	inline static int *getWeaponClip1Pointer ( edict_t *pgun ) { return g_GetProps[GETPROP_WEAPONCLIP1].getIntPointer(pgun); }
	inline static int *getWeaponClip2Pointer ( edict_t *pgun ) { return g_GetProps[GETPROP_WEAPONCLIP2].getIntPointer(pgun); }

	inline static void getWeaponClip ( edict_t *pgun, int *iClip1, int *iClip2 ) { *iClip1 = g_GetProps[GETPROP_WEAPONCLIP1].getInt(pgun,0); *iClip2 = g_GetProps[GETPROP_WEAPONCLIP2].getInt(pgun,0); }
	inline static void getAmmoTypes ( edict_t *pgun, int *iAmmoType1, int *iAmmoType2 ) { *iAmmoType1 = g_GetProps[GETPROP_WEAPON_AMMOTYPE1].getInt(pgun,-1); *iAmmoType2 = g_GetProps[GETPROP_WEAPON_AMMOTYPE2].getInt(pgun,-1);} 

	inline static int getPlayerClassDOD(edict_t *player) { return g_GetProps[GETPROP_DOD_PLAYERCLASS].getInt(player,0); }
	inline static void getPlayerInfoDOD(edict_t *player, bool *m_bProne, float *m_flStamina)
	{
		*m_bProne = g_GetProps[GETPROP_DOD_STAMINA].getBool(player,false);
		*m_flStamina = g_GetProps[GETPROP_DOD_PRONE].getFloat(player,0);
	}

	inline static int getDesPlayerClassDOD(edict_t *player) { return g_GetProps[GETPROP_DOD_DES_PLAYERCLASS].getInt(player,0); }
	// HL2DM
	//static void 

private:
	static CClassInterfaceValue g_GetProps[GET_PROPDATA_MAX];

};

#endif