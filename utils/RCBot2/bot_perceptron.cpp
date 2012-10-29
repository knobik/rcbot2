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

void CLogisticalNeuron :: train ()// ITransfer *transferFunction, bool usebias )
{
	ga_nn_value delta;

	for ( register unsigned int i = 0; i < m_weights.size(); i ++ )
	{
		delta = (m_LearnRate * m_inputs[i] * m_error);
		delta += m_momentum * 0.9f;
		m_weights[i] += delta;
		m_momentum = delta;
	}

	//if ( usebias )
	m_Bias += m_LearnRate * m_error;
}

ga_nn_value CLogisticalNeuron :: execute ( ITransfer *transferFunction )//, bool usebias )
{
	m_netinput = 0;

	// bias weight
	//if ( usebias )
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

CBotNeuralNet :: CBotNeuralNet ( unsigned int numinputs, unsigned int numhiddenlayers, 
							  unsigned int neuronsperhiddenlayer, unsigned int numoutputs, 
								ga_nn_value learnrate, ga_nn_value min, ga_nn_value max )
{
	register unsigned int i;
	register unsigned int j;

	m_pOutputs = new CLogisticalNeuron[numoutputs];
	m_pHidden = new CLogisticalNeuron*[numhiddenlayers];

	for ( j = 0; j < numhiddenlayers; j ++ )
	{
		m_pHidden[j] = new CLogisticalNeuron[neuronsperhiddenlayer];

		for ( i = 0; i < neuronsperhiddenlayer; i ++ )
		{
			if ( j == 0 )
				m_pHidden[j][i].init(numinputs,learnrate);
			else
				m_pHidden[j][i].init(neuronsperhiddenlayer,learnrate);
		}
	}

	for ( i = 0; i < numoutputs; i ++ )
		m_pOutputs[i].init(neuronsperhiddenlayer,learnrate);

	m_transferFunction = new CSigmoidTransfer ();

	m_numInputs = numinputs;
	m_numOutputs = numoutputs;
	m_numHidden = neuronsperhiddenlayer;
	m_numHiddenLayers = numhiddenlayers;

	m_fMax = max;
	m_fMin = min;
}

void CBotNeuralNet :: batch_train ( training_batch_t *batches, unsigned numbatches, unsigned int epochs )
{
	vector <ga_nn_value> outs;
	//ga_nn_value err = 0;
	ga_nn_value exp_out; // expected
	ga_nn_value act_out; // actual
	ga_nn_value out_error;
	unsigned int e; // epoch
	register unsigned int bi; // batch iterator
	register unsigned int i; // ith node
	register unsigned int j; //jth output
	register signed int l; // layer
	CLogisticalNeuron *pNode;


	for ( e = 0; e < epochs; e ++ )
	{
		/*if ( !(e%100) )
		{
			system("CLS");
			printf("-----epoch %d-----\n",e);
			printf("training... %0.1f percent",(((float)e/epochs))*100);
			printf("in1\tin2\texp\tact\terr\n");
		}*/

		for ( bi = 0; bi < numbatches; bi ++ )
		{
			outs.clear();

			execute(&(batches[bi].in),&outs);

			// work out error for output layer
			for ( j = 0; j < m_numOutputs; j ++ )
			{
				pNode = &(m_pOutputs[j]);
				act_out = pNode->getOutput();
				exp_out = scale(batches[bi].out[j],m_fMin,m_fMax);
				out_error = act_out * (1.0f-act_out) * (exp_out - act_out);
				pNode->setError(out_error);
			}

			/*if ( !(e%100) )
			{
				printf("%0.2f\t%0.2f\t%0.2f\t%0.6f\t%0.6f\n",batches[bi].in[0],batches[bi].in[1],batches[bi].out[0],outs[0],out_error);
			}*/

			//Send Error back to Hidden Layer before output
			for ( i = 0; i < m_numHidden; i ++ )
			{	
				ga_nn_value err = 0;

				for ( j = 0; j < m_numOutputs; j ++ )
				{
					err += m_pOutputs[j].getError(i);
				}

				pNode = &m_pHidden[m_numHiddenLayers-1][i];

				pNode->setError(pNode->getOutput() * (1.0f-pNode->getOutput()) * err);
			}

			for ( l = (m_numHiddenLayers-2); l >= 0; l -- )
			{
				//Send Error back to Input Layer
				for ( i = 0; i < m_numHidden; i ++ )
				{	
					ga_nn_value err = 0;

					pNode = m_pHidden[l+1];

					for ( j = 0; j < m_numHidden; j ++ )
					{
						// check the error from the next layer
						err += pNode[j].getError(i);
					}

					pNode = &(m_pHidden[l][i]);

					pNode->setError((pNode->getOutput() * (1.0f-pNode->getOutput())) * err);
				}
			}

			for ( j = 0; j < m_numHiddenLayers; j ++ )
			{
				pNode = m_pHidden[j];
				// update weights for hidden layer (each neuron)
				for ( i = 0; i < m_numHidden; i ++ )
				{	
					(&(pNode[i]))->train(); // update weights for this node
				}
			}

			// update weights for output layer
			for ( i = 0; i < m_numOutputs; i ++ )
			{	
				m_pOutputs[i].train(); // update weights for this node
			}
		}
	}

}

void CBotNeuralNet :: execute ( vector <ga_nn_value> *inputs, vector<ga_nn_value> *outputs )
{
	vector <ga_nn_value> layeroutput;
	vector <ga_nn_value> layerinput;
	CLogisticalNeuron *pNode;
	CLogisticalNeuron *pLayer;
	register unsigned int i; // i-th node
	register unsigned short l; // layer

	outputs->clear();

	//scale inputs
	for ( i = 0; i < inputs->size(); i ++ )
		layeroutput.push_back(scale((*inputs)[i],m_fMin,m_fMax));

	for ( l = 0; l < m_numHiddenLayers; l ++ )
	{
		layerinput = layeroutput;
		layeroutput.clear();

		pLayer = m_pHidden[l];

		// execute hidden
		for ( i = 0; i < m_numHidden; i ++ )
		{
			pNode = &(pLayer[i]);
			pNode->input(layerinput);
			pNode->execute(m_transferFunction);

			layeroutput.push_back(pNode->getOutput());
		}
	}

	// execute output
	for ( i = 0; i < m_numOutputs; i ++ )
	{
		pNode = &m_pOutputs[i];
		pNode->input(layeroutput);
		pNode->execute(m_transferFunction);
		outputs->push_back(descale(pNode->getOutput(),m_fMin,m_fMax));
	}
}