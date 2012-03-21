#ifndef __BOT_CONFIGFILE__
#define __BOT_CONFIGFILE__

#include "bot_utility.h"

#include <vector>
using namespace std;

typedef enum
{
	BOT_ATT_UTIL = 0,
	BOT_NORM_UTIL,
	UTIL_TYPE_MAX
}eTF2UtilType;

typedef struct
{
	float min;
	float max;
}bot_util_t;

class CRCBotTF2UtilFile
{
public:
	static void addUtilPerturbation (eBotAction iAction, eTF2UtilType iUtil, float fUtility[9][2]);

	static void init ();

	static void loadConfig ();
	// 2 Teams / 2 Types Attack/Defend / 
	static bot_util_t m_fUtils[UTIL_TYPE_MAX][BOT_UTIL_MAX][9];
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