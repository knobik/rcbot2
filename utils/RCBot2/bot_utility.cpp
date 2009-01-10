/*
 *    part of https://rcbot2.svn.sourceforge.net/svnroot/rcbot2
 *
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

#include "bot_utility.h"

void CBotUtilities :: execute ()
{
	unsigned int i = 0;
	CBotUtility *pUtil;
	float fUtil;

	util_node_t *temp;
	util_node_t *pnew;
	util_node_t *prev;

	m_pBest.head = NULL;

	for ( i = 0; i < m_Utilities.size(); i ++ )
	{
		pUtil = &(m_Utilities[i]);
		fUtil = pUtil->getUtility();

		if ( pUtil->canDo() )
		{			
			temp = m_pBest.head;

			pnew = (util_node_t*)malloc(sizeof(util_node_t));
			pnew->util = pUtil;
			pnew->next = NULL;
			prev = NULL;

			if ( temp )
			{
				while ( temp )
				{
					if ( fUtil > temp->util->getUtility() )
					{
						if ( temp == m_pBest.head )
						{
							pnew->next = temp;
							m_pBest.head = pnew;
							break;
						}
						else
						{
							prev->next = pnew;
							pnew->next = temp;
							break;
						}
					}

					prev = temp;
					temp = temp->next;
				}

				if ( pnew->next == NULL )
					prev->next = pnew;
			}
			else
				m_pBest.head = pnew;
		}
	}

	//return pBest;
}

void CBotUtilities :: freeMemory ()
{
	util_node_t *temp;
	m_Utilities.clear();

	while ( (temp = m_pBest.head) != NULL )
	{
		temp = m_pBest.head;
		m_pBest.head = m_pBest.head->next;
		free(temp);		
	}
}

CBotUtility *CBotUtilities :: nextBest ()
{
	CBotUtility *pBest;
	util_node_t *temp;

	if ( m_pBest.head == NULL )
		return NULL;

	pBest = m_pBest.head->util;

	temp = m_pBest.head;

	m_pBest.head = m_pBest.head->next;

	free(temp);

	return pBest;
	
}