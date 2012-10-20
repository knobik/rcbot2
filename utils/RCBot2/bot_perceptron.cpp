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
//#include "vstdlib/random.h" // for random functions
#include "bot_mtrand.h"
#include "bot_perceptron.h"

ga_nn_value CPerceptron::m_fDefaultLearnRate = 0.5f;
ga_nn_value CPerceptron::m_fDefaultBias = 1.0f;

ga_nn_value CSigmoidTransfer :: transfer ( ga_nn_value netInput )
{
	return 1/(1+exp(-netInput));
}

ga_nn_value CSigmoidTransfer :: deriv ( ga_nn_value x )
{
	ga_nn_value ex = exp(x);
	ga_nn_value ex1 = (1 + ex);
	return (ex / (ex1*ex1));
}

static ga_nn_value m_fDefaultLearnRate;// = 0.5f;
static ga_nn_value m_fDefaultBias;// = 1.0f;

CNeuron :: CNeuron ()
{
	
}

CPerceptron :: CPerceptron (unsigned int iInputs,ITransfer *transferFunction)
{
	m_inputs.clear();
	m_iInputs = iInputs;
	
	// bias weight
	m_Bias = -0.5;
	
	for ( unsigned int i = 0; i < m_iInputs; i++ )
		m_weights.push_back(-0.3f+randomFloat(0.0f,0.6f));
	
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

void CNeuron :: input ( vector <ga_nn_value> inputs )
{
	m_inputs.clear();
	
	for ( unsigned int i = 0; i < inputs.size(); i ++ )
		m_inputs.push_back(inputs[i]);		
}

ga_nn_value CPerceptron :: execute ()
{
	// bias weight
	ga_nn_value fNetInput = m_Bias;
	
	for ( unsigned int i = 0; i < m_inputs.size(); i ++ )	
	{
		fNetInput += m_weights[i]*m_inputs[i];
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
	m_Bias += m_LearnRate*(expectedOutput-m_output);
	
	for ( unsigned int i = 0; i < m_weights.size(); i ++ )
	{
		m_weights[i] = m_weights[i] + m_LearnRate*(expectedOutput-m_output)*m_inputs[i];
	}
}

void CLogisticalNeuron :: train ( ITransfer *transferFunction, bool usebias )
{
	ga_nn_value delta;

	for ( unsigned int i = 0; i < m_weights.size(); i ++ )
	{
		delta = (m_LearnRate * m_inputs[i] * m_error);
		delta += m_momentum * m_LearnRate;
		m_weights[i] = m_weights[i] + delta;
		m_momentum = delta;
	}

	if ( usebias )
		m_Bias += m_LearnRate * m_error;
}

ga_nn_value CLogisticalNeuron :: execute ( ITransfer *transferFunction, bool usebias )
{
	m_netinput = 0;

	// bias weight
	if ( usebias )
		m_netinput = m_Bias;
	
	for ( unsigned int i = 0; i < m_inputs.size(); i ++ )	
	{
		m_netinput += m_weights[i]*m_inputs[i];
	}

	m_output = transferFunction->transfer(m_netinput);
	
	return m_output;
}

void CLogisticalNeuron::init(unsigned int iInputs, ga_nn_value learnrate)
{
		m_error = 0;
		m_netinput = 0;
		m_LearnRate = 0.2f;
		m_output = 0;
		m_momentum = 0;

		for ( unsigned int i = 0; i < iInputs; i ++ )
			m_weights.push_back(randomFloat(-0.99f,0.99f));

		m_iInputs = iInputs;
		m_LearnRate = learnrate;
}

CNN_1_Hidden :: CNN_1_Hidden ( unsigned int numinputs, unsigned int inputlayer, 
							  unsigned int hiddenlayer, unsigned int outputlayer, 
								ga_nn_value learnrate, ga_nn_value min, ga_nn_value max )
{
	m_pInputs = new CLogisticalNeuron[inputlayer];
	m_pOutputs = new CLogisticalNeuron[outputlayer];
	m_pHidden = new CLogisticalNeuron[hiddenlayer];

	for ( unsigned int i = 0; i < inputlayer; i ++ )
		m_pInputs[i].init(numinputs,learnrate);

	for ( unsigned int i = 0; i < hiddenlayer; i ++ )
		m_pHidden[i].init(inputlayer,learnrate);

	for ( unsigned int i = 0; i < outputlayer; i ++ )
		m_pOutputs[i].init(hiddenlayer,learnrate);

	m_transferFunction = new CSigmoidTransfer ();

	m_numInputs = inputlayer;
	m_numOutputs = outputlayer;
	m_numHidden = hiddenlayer;

	m_fMax = max;
	m_fMin = min;
}

void CNN_1_Hidden :: batch_train ( training_batch_t *batches, unsigned numbatches, unsigned int epochs )
{
	vector <ga_nn_value> outs;
	ga_nn_value err = 0;
	ga_nn_value exp_out; // expected
	ga_nn_value act_out; // actual

	for ( unsigned int e = 0; e < epochs; e ++ )
	{
		printf("-----epoch %d-----\n",e);

		for ( unsigned int bi = 0; bi < numbatches; bi ++ )
		{
			outs.clear();

			execute(batches[bi].in,&outs);

			printf("%0.6f,%0.6f,%0.6f\n",batches[bi].in[0],batches[bi].in[1],outs[0]);

			// work out error for output layer
			for ( unsigned int j = 0; j < m_numOutputs; j ++ )
			{
				act_out = m_pOutputs[j].getOutput();
				exp_out = scale(batches[bi].out[j],m_fMin,m_fMax);

				m_pOutputs[j].setError(act_out * (1.0f-act_out) * (exp_out - act_out));
			}

			//Send Error back to Hidden Layer
			for ( unsigned int i = 0; i < m_numHidden; i ++ )
			{	
				ga_nn_value err = 0;

				for ( unsigned int j = 0; j < m_numOutputs; j ++ )
				{
					err += m_pOutputs[j].getError(i);
				}

				m_pHidden[i].setError((m_pHidden[i].getOutput() * (1.0f-m_pHidden[i].getOutput())) * err);
			}

			//Send Error back to Input Layer
			for ( unsigned int i = 0; i < m_numInputs; i ++ )
			{	
				ga_nn_value err = 0;

				for ( unsigned int j = 0; j < m_numHidden; j ++ )
				{
					err += m_pHidden[j].getError(i);
				}

				m_pInputs[i].setError((m_pInputs[i].getOutput() * (1.0f-m_pInputs[i].getOutput())) * err);
			}

			// update weights for input layer
			for ( unsigned int i = 0; i < m_numInputs; i ++ )
			{	
				m_pInputs[i].train(m_transferFunction);
			}

			// update weights for hidden layer
			for ( unsigned int i = 0; i < m_numHidden; i ++ )
			{	
				m_pHidden[i].train(m_transferFunction);
			}

			// update weights for output layer
			for ( unsigned int i = 0; i < m_numOutputs; i ++ )
			{	
				m_pOutputs[i].train(m_transferFunction);
			}
		}
	}

}

void CNN_1_Hidden :: execute ( vector <ga_nn_value> inputs, vector<ga_nn_value> *outputs )
{
	vector <ga_nn_value> layer1output;
	vector <ga_nn_value> layer2output;

	outputs->clear();

	//scale inputs
	for ( unsigned int i = 0; i < inputs.size(); i ++ )
	{
		inputs[i] = scale(inputs[i],m_fMin,m_fMax);
	}

	// execute input
	for ( unsigned short i = 0; i < m_numInputs; i ++ )
	{
		m_pInputs[i].input(inputs);
		m_pInputs[i].execute(m_transferFunction);
		
		layer1output.push_back(m_pInputs[i].getOutput());
	}

	// execute hidden
	for ( unsigned short i = 0; i < m_numHidden; i ++ )
	{
		m_pHidden[i].input(layer1output);
		m_pHidden[i].execute(m_transferFunction);
		
		layer2output.push_back(m_pInputs[i].getOutput());
	}

	// execute output
	for ( unsigned short i = 0; i < m_numOutputs; i ++ )
	{
		m_pOutputs[i].input(layer2output);
		m_pOutputs[i].execute(m_transferFunction);
		outputs->push_back(descale(m_pOutputs[i].getOutput(),m_fMin,m_fMax));
	}
}
