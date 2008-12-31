#ifndef __BOT_COOP_H__
#define __BOT_COOP_H__

class CBotCoop : public CBot
{
public:
	virtual void modThink ();

	virtual bool isEnemy ( edict_t *pEdict, bool bCheckWeapons = true  );

	virtual bool startGame ();
};

#endif