#ifndef __PERCEPTRON_H__
#define __PERCEPTRON_H__

#include "bot_ga_nn_const.h"
#include <vector>
using namespace std;

inline ga_nn_value scale (ga_nn_value x, ga_nn_value min, ga_nn_value max)
{
	return ((x-min)/(max-min));
}

inline ga_nn_value descale (ga_nn_value x, ga_nn_value min, ga_nn_value max)
{
	return (min+(x*(max-min)));
}

class ITransfer
{
public:
	virtual ga_nn_value transfer ( ga_nn_value netInput ) = 0;
	virtual ga_nn_value deriv ( ga_nn_value x ) = 0;
};


class CSigmoidTransfer : public ITransfer
{
public:
	ga_nn_value transfer ( ga_nn_value netInput );
	ga_nn_value deriv ( ga_nn_value x );
};

class CNeuron
{
public:
	CNeuron ();

	CNeuron (unsigned int iInputs);

	void setWeights ( vector <ga_nn_value> weights );

	virtual void input ( vector <ga_nn_value> inputs );

	inline ga_nn_value getWeight ( unsigned int i ) { return m_weights[i]; }

	ga_nn_value execute ();

	bool fired ();

	inline ga_nn_value getOutput () { return m_output; }

protected:
	
	unsigned int m_iInputs;
	ga_nn_value m_LearnRate;
	vector <ga_nn_value> m_inputs;
	vector <ga_nn_value> m_weights;
	ga_nn_value m_output;
	ga_nn_value m_Bias;
	
};

class CPerceptron : public CNeuron
{
public:

	static ga_nn_value m_fDefaultLearnRate;// = 0.5f;
	static ga_nn_value m_fDefaultBias;// = 1.0f;

	CPerceptron (unsigned int iInputs,ITransfer *transferFunction=NULL);

	void setWeights ( vector <ga_nn_value> weights );

	ga_nn_value execute ();

	bool fired ();

	ga_nn_value getOutput ();

	void train ( ga_nn_value expectedOutput );

private:
	
	ITransfer *m_transferFunction;
};

class CLogisticalNeuron : public CNeuron
{
public:
	CLogisticalNeuron()
	{
		m_error = 0;
		m_netinput = 0;
		m_LearnRate = 0.2f;
		m_output = 0;
		m_iInputs = 0;
		m_momentum = 0;
		m_Bias = -1.0f;
	}

	void init(unsigned int iInputs, ga_nn_value learnrate);

	void train ();/// ITransfer *transferFunction, bool usebias = true );

	ga_nn_value execute ( ITransfer *transferFunction );//, bool usebias = true );

	inline void setError ( ga_nn_value err ) { m_error = err; }
	inline void addError ( ga_nn_value err ) { m_error += err; }
	inline void divError ( unsigned int samples ) { m_error /= samples; }

	inline ga_nn_value getError ( unsigned int w ) { return m_error * m_weights[w]; }
	inline ga_nn_value getMSE () { return m_error; }
private:
	ga_nn_value m_error;
	ga_nn_value m_netinput;
	ga_nn_value m_momentum;
};

typedef struct
{
	vector<ga_nn_value> in;
	vector<ga_nn_value> out;
}training_batch_t;

class CBotNeuralNet
{
public:

	CBotNeuralNet ( unsigned int numinputs, unsigned int hiddenlayers, unsigned int hiddenlayer, unsigned int outputlayer, ga_nn_value learnrate, ga_nn_value min, ga_nn_value max );

	CBotNeuralNet ()
	{
		m_pOutputs = NULL;
		m_transferFunction = NULL;
		m_fMin = 0;
		m_numInputs = 0; // number of inputs
		m_numOutputs = 0; // number of outputs
		m_numHidden = 0; // neurons per hidden layer
		m_numHiddenLayers = 0;
		m_fMax = 1;
	}

	void execute ( vector <ga_nn_value> *inputs, vector<ga_nn_value> *outputs );

	void batch_train ( training_batch_t *batches, unsigned numbatches, unsigned int epochs );

	~CBotNeuralNet ()
	{
		if ( m_pOutputs )
			delete [] m_pOutputs;
		if ( m_transferFunction )
			delete m_transferFunction;
		if ( m_pHidden )
		{
			for ( unsigned int i = 0; i < m_numHiddenLayers; i ++ )
				delete [] m_pHidden[i];
		}
	}
private:
	ITransfer *m_transferFunction;
	unsigned int m_numInputs; // number of inputs
	unsigned int m_numOutputs; // number of outputs
	unsigned int m_numHidden; // neurons per hidden layer
	unsigned int m_numHiddenLayers;

	CLogisticalNeuron *m_pOutputs;
	CLogisticalNeuron **m_pHidden;

	ga_nn_value m_fMax;
	ga_nn_value m_fMin;

};


class CBotTrainDatum
{
public:

	// 
private:
	training_batch_t batch;
};

// keep training data as not to train on the fly
// can train during map change
class CBotTrainData
{
public:
	CBotTrainData ( unsigned int maxbatches, CBotNeuralNet *nn )
	{
		//assert(nn!=NULL);
		m_nn = nn;
		m_usedbatches = 0;
		m_maxbatches = maxbatches;
		m_batches = new training_batch_t[maxbatches];
	}

	~CBotTrainData()
	{
		delete m_batches;
	}

	bool newbatch ()
	{
		if ( m_usedbatches < m_maxbatches )
		{
			m_usedbatches++;
			return true;
		}
		
		return false;
	}

	inline void input ( ga_nn_value val, ga_nn_value scale )
	{
		m_batches[m_usedbatches-1].in.push_back(val/scale);
	}

	inline void output ( ga_nn_value output )
	{
		m_batches[m_usedbatches-1].out.push_back(output);
	}

	void train (unsigned int epochs)
	{
		m_nn->batch_train(m_batches,m_usedbatches,epochs);
	}

	//void check

private:
	training_batch_t *m_batches;
	unsigned int m_maxbatches;
	unsigned int m_usedbatches;
	CBotNeuralNet *m_nn; // nn to train
	//float m_fCheckTime;
};


#endif