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
#include <math.h>
#include "vstdlib/random.h" // for random functions
#include "bot_perceptron.h"

ga_nn_value CPerceptron::m_fDefaultLearnRate = 0.5f;
ga_nn_value CPerceptron::m_fDefaultBias = 1.0f;

ga_nn_value CSigmoidTransfer :: transfer ( ga_nn_value netInput )
{
	return 1/(1+exp(-netInput));
}

static ga_nn_value m_fDefaultLearnRate;// = 0.5f;
static ga_nn_value m_fDefaultBias;// = 1.0f;

CPerceptron :: CPerceptron (unsigned int iInputs,ITransfer *transferFunction)
{
	m_inputs.clear();
	m_iInputs = iInputs;
	
	// bias weight
	m_weights.push_back(RandomFloat(0,1));
	
	for ( unsigned int i = 0; i < m_iInputs; i++ )
		m_weights.push_back(-1+RandomFloat(0,2));
	
	m_transferFunction = transferFunction;
	
	if ( m_transferFunction == NULL )
		m_transferFunction = new CSigmoidTransfer();
}

void CPerceptron :: setWeights ( vector <ga_nn_value> weights )
{
	m_weights.clear();
	
	for ( unsigned int i = 0; i < weights.size(); i ++ )
		m_weights.push_back(weights[i]);
}

void CPerceptron :: input ( vector <ga_nn_value> inputs )
{
	m_inputs.clear();
	
	for ( unsigned int i = 0; i < inputs.size(); i ++ )
		m_inputs.push_back(inputs[i]);		
}

ga_nn_value CPerceptron :: execute ()
{
	// bias weight
	ga_nn_value fNetInput = m_weights[0];
	
	for ( unsigned int i = 0; i < m_inputs.size(); i ++ )	
	{
		fNetInput = m_weights[i+1]*m_inputs[i];
	}
	
	m_output = m_transferFunction->transfer(fNetInput);
	
	return m_output;
}

bool CPerceptron :: fired ()
{
	return m_output >= 0.5f;
}

ga_nn_value CPerceptron :: getOutput ()
{
	return m_output;
}

void CPerceptron :: train ( ga_nn_value expectedOutput )
{
	// bias
	m_weights[0] = m_weights[0] + m_LearnRate*(expectedOutput-m_output)*m_Bias;
	
	for ( unsigned int i = 1; i < m_weights.size(); i ++ )
	{
		m_weights[i] = m_weights[i] + m_LearnRate*(expectedOutput-m_output)*m_inputs[i-1];
	}
}
