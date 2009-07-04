/*
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
#include <string.h>

#include "bot.h"
#include "bot_script.h"
#include "bot_fortress.h"
#include "bot_strings.h"
#include "bot_globals.h"

vector <CResetPoint*> CPoints::m_points;

CPointStyle::CPointStyle (int iPointArea,ePointStyle iStyle)
{
		m_iPointArea = iPointArea;
		m_iStyle = iStyle;
}

void CResetPoint :: getCurrentPoints ( int iTeamCaptured, int iTeam, vector<CPointStyle> **iNextPoints )
{
	*iNextPoints = NULL;

// when blue captures (or is beginning)
	if ( isResetPoint() || (iTeamCaptured == TF2_TEAM_BLUE) )
	{
		if ( iTeam == TF2_TEAM_BLUE )
			*iNextPoints = &m_iNextBlue[0];
		else if ( iTeam == TF2_TEAM_RED )
			*iNextPoints = &m_iNextRed[0];
	}
	// When red captured
	else
	{
		if ( iTeam == TF2_TEAM_BLUE )
			*iNextPoints = &m_iNextBlue[1];
		else if ( iTeam == TF2_TEAM_RED )
			*iNextPoints = &m_iNextRed[1];
	}
}

void CResetPoint :: addPointStyle ( int iTeamCaptured, int iTeam, CPointStyle pointstyle )
{
	if ( isResetPoint() || (iTeamCaptured == TF2_TEAM_BLUE) )
	{
		if ( iTeam == TF2_TEAM_BLUE )
			m_iNextBlue[0].push_back(pointstyle);
		else
			m_iNextRed[0].push_back(pointstyle);
	}
	else if ( iTeamCaptured == TF2_TEAM_RED )
	{
		if ( iTeam == TF2_TEAM_BLUE )
			m_iNextBlue[1].push_back(pointstyle);
		else
			m_iNextRed[1].push_back(pointstyle);
	}
}

void CResetPoint :: addValidArea ( int iArea )
{
	m_iValidAreas.push_back(iArea);
}

CPoint :: CPoint ( const char *szName )
{
	m_szName = CStrings::getString(m_szName);
}

bool CPoint :: isPoint ( const char *szName ) 
{ 
	return !strcmp(szName,m_szName); 
}


CResetPoint *CPoints::getPoint ( const char *szName )
{
	unsigned int i;
	CResetPoint *newpoint = NULL;

    for ( i = 0; i < m_points.size(); i ++ )
	{
		if ( szName == NULL )
		{
			if ( m_points[i]->isResetPoint() )
				return m_points[i];
		}
		else if ( m_points[i]->isPoint(szName) )
			return m_points[i];
	}

	if ( szName )
		newpoint = new CPoint(szName);
	else
		newpoint = new CResetPoint();

	m_points.push_back(newpoint);

	return newpoint;
}

void CPoints :: freeMemory ()
{
	unsigned int i = 0;

	for ( i = 0; i < m_points.size(); i ++ )
	{
		delete m_points[i];
		m_points[i] = NULL;
	}

	m_points.clear();
}

void CPoints :: loadMapScript ( )
{
	char filename[1024];

	vector < CResetPoint* > points;
	CResetPoint *currentpoint = NULL;

	freeMemory();
	
	CBotGlobals::buildFileName(filename,CBotGlobals::getMapName(),BOT_SCRIPT_FOLDER,BOT_SCRIPT_EXTENSION);

	FILE *fp = CBotGlobals::openFile(filename,"r");

	int state = STATE_NONE;
	if ( fp )
	{
		char line[256];
		int len;

		int linenum = 0;

		while ( fgets(line,255,fp) != NULL )
		{
			line[255] = 0;

			linenum++;

			if ( line[0] == '#' )
				continue;

			len = strlen(line);

			if ( line[len-1] == '\n' )
				line[--len] = 0;
			if ( len < 1 )
				continue;

			// control character
			if ( line[0] == ':' )
			{
				if ( strcmpi(&line[1],"on_blue_cap:") == 0 )
				{
					currentpoint = getPoint(&line[14]);
					state = STATE_BLUE_CAP;
				}
				else if ( strcmpi(&line[1],"on_red_cap:") == 0 )
				{
					currentpoint = getPoint(&line[13]);
					state = STATE_BLUE_CAP;
				}
				else if ( strcmpi(&line[1],"on_reset:") == 0 )
				{
					currentpoint = getPoint(&line[11]);
					state = STATE_RESET;
				}
			}
			else if ( state != STATE_NONE )
			{
				int iTeam = 0;
				ePointStyle style;
			
				int i,n;
				char num[4];

				num[0] = 0;
				
				if ( strncmp("red_",&line[1],4) == 0 )
				{
					iTeam = TF2_TEAM_RED;
					i = 6;
				}
				else if ( strncmp("blue_",&line[1],5) == 0 )
				{
					iTeam = TF2_TEAM_BLUE;
					i = 7;
				}
				else
				{
					CBotGlobals::botMessage(NULL,0,"SCRIPT Syntax Error line : %d, missing team name",linenum);
					// Syntax Error : missing team name
					continue;
				}

				if ( strcmp("attack:",&line[i]) == 0) 
					style = POINT_ATTACK;
				else if ( strcmp("defend:",&line[i]) == 0 )
					style = POINT_DEFEND;
				else
				{
					CBotGlobals::botMessage(NULL,0,"SCRIPT Syntax Error line : %d, missing attack/defend",linenum);
					// Syntax Error : missing attack/defend
					continue;
				}

				// get points

				i = i + 7;
				n = 0;

				while ( i < len )
				{
					if ( n < 4 )
						num[n++] = line[i];

					if ( ((i+1) >= len) || (line[i+1]==',') )
					{
						if ( state == STATE_RESET )
							currentpoint->addPointStyle(0,iTeam,CPointStyle(atoi(num),style));
						else if ( state == STATE_BLUE_CAP )
							currentpoint->addPointStyle(TF2_TEAM_BLUE,iTeam,CPointStyle(atoi(num),style));
						else if ( state == STATE_RED_CAP )
							currentpoint->addPointStyle(TF2_TEAM_RED,iTeam,CPointStyle(atoi(num),style));
						
						n = 0;
					}

					i++;
				}
				
			}
		}
	}
	else
		return;
}