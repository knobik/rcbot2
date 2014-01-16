#ifndef __RCBOT_EHANDLE_H__
#define __RCBOT_EHANDLE_H__

////// entity handling in network
class MyEHandle 
{
public:
	MyEHandle ()
	{
		m_pEnt = NULL;
		m_iSerialNumber = 0;
	}

    MyEHandle ( edict_t *pent )
	{
		m_pEnt = pent;

		if ( pent )
			m_iSerialNumber = pent->m_NetworkSerialNumber;
	}

	inline edict_t *get ()
	{
		if ( m_pEnt )
		{
			if ( !m_pEnt->IsFree() && (m_iSerialNumber == m_pEnt->m_NetworkSerialNumber) )
				return m_pEnt;
		}

		return NULL;
	}

	inline edict_t *get_old ()
	{
		return m_pEnt;
	}

	inline operator edict_t * const ()
	{ // same as get function (inlined for speed)
		if ( m_pEnt )
		{
			if ( !m_pEnt->IsFree() && (m_iSerialNumber == m_pEnt->m_NetworkSerialNumber) )
				return m_pEnt;
		}

		return NULL;
	}

	inline bool operator == ( int a )
	{
		return ((int)get() == a);
	}

	inline bool operator == ( edict_t *pent )
	{
		return (get() == pent);
	}

	inline bool operator == ( MyEHandle &other )
	{
		return (get() == other.get());
	}

	inline edict_t * operator = ( edict_t *pent )
	{
		m_pEnt = pent;

		if ( pent )
			m_iSerialNumber = pent->m_NetworkSerialNumber;

		return m_pEnt;
	}
private:
	int m_iSerialNumber;
	edict_t *m_pEnt;
};

#endif