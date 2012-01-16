#ifndef __BOT_CONFIGFILE__
#define __BOT_CONFIGFILE__

#include "bot_utility.h"

#include <vector>
using namespace std;

class CConfigFile
{
public:
	virtual void loadConfig () = 0;
};


typedef enum
{
	BOT_ATT_UTIL = 0,
	BOT_NORM_UTIL,
	UTIL_TYPE_MAX
}eTF2UtilType;

class CRCBotTF2UtilFile : public CConfigFile
{
public:
	void loadConfig ();
	// 2 Teams / 2 Types Attack/Defend / 
	static float m_fUtils[2][UTIL_TYPE_MAX][BOT_UTIL_MAX];
};


class CBotConfigFile
{
public:
	static void load ();

	static void reset ()
	{
		m_iCmd = 0;
		m_fNextCommandTime = 0.0f;
	}

	static void doNextCommand ();
private:
	static vector <char *> m_Commands;
	static unsigned int m_iCmd; // current command (time delayed)
	static float m_fNextCommandTime;
};

#endif