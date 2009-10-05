#ifndef __BOT_CONFIGFILE__
#define __BOT_CONFIGFILE__

#include <vector>
using namespace std;

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