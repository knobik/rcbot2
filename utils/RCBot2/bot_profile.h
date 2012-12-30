#ifndef __RCBOT_PROFILE_H__
#define __RCBOT_PROFILE_H__

#include <vector>
using namespace std;

class CBotProfile
{
public:
	// setup profile
	CBotProfile (const char *szName, const char *szModel, int iTeam, const char *szWeapon, int m_iVisionTicks, int m_iPathTicks, int iClass = 0 );

	inline char *getName ()	 { return m_szName; }
	inline char *getModel () { return m_szModel; }
	inline int getTeam () { return m_iTeam; }
	inline char *getWeapon () { return m_szWeapon; }
	inline int getVisionTicks () { return m_iVisionTicks; }
	inline int getPathTicks () { return m_iPathTicks; }
	inline int getClass () { return m_iClass; }
	inline void setTeam (int iTeam) { m_iTeam = iTeam; }
	inline void setClass (int iClass) { m_iClass = iClass; }

private:
	// bot's name
	char *m_szName;
	char *m_szModel;
	// bot's team
	int m_iTeam;				// preferred player team
	char *m_szWeapon;			// preferred weapon
	int m_iVisionTicks;			// speed of finding non players (npcs/teleporters etc)
	int m_iPathTicks;			// speed of finding a path
	int m_iClass;				// preferred player class
	int m_iVisionTicksClients;	// speed of finding other players and enemy players
	float m_fSensitivity;		// sensitivity of bot's "mouse" (angle speed)
	float m_fBraveness;			// sensitivity to danger (brave = less sensitive)
};

class CBotProfiles
{
public:
	static void deleteProfiles ();

	// find profiles and setup list
	static void setupProfiles ();

	// return a profile unused by a bot
	static CBotProfile *getRandomFreeProfile ();

	static CBotProfile *getDefaultProfile ();

private:
	static vector <CBotProfile*> m_Profiles;
	static CBotProfile *m_pDefaultProfile;
};

#endif