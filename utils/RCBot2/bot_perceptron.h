#ifndef __PERCEPTRON_H__
#define __PERCEPTRON_H__

#include "bot_ga_nn_const.h"
#include <vector>
using namespace std;

typedef float ga_nn_value;

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

typedef struct
{
	vector<ga_nn_value> in;
	vector<ga_nn_value> out;
}training_batch_t;

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

	void train_back ( ga_nn_value error );

	void train_forward ( ITransfer *transferFunction );

	void train ( ITransfer *transferFunction, bool usebias = true );

	ga_nn_value execute ( ITransfer *transferFunction, bool usebias = true );

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

class CNN_1_Hidden
{
public:

	CNN_1_Hidden ( unsigned int numinputs, unsigned int inputlayer, unsigned int hiddenlayer, unsigned int outputlayer, ga_nn_value learnrate, ga_nn_value min, ga_nn_value max );

	CNN_1_Hidden ()
	{
		m_pInputs = NULL;
		m_pOutputs = NULL;
		m_transferFunction = NULL;
		m_fMin = 0;
		m_fMax = 1;
	}

	void execute ( vector <ga_nn_value> inputs, vector<ga_nn_value> *outputs );

	void batch_train ( training_batch_t *batches, unsigned numbatches, unsigned int epochs );

	~CNN_1_Hidden ()
	{
		if ( m_pInputs ) 
			delete m_pInputs;
		if ( m_pOutputs )
			delete m_pOutputs;
		if ( m_transferFunction )
			delete m_transferFunction;
		if ( m_pHidden )
			delete m_pHidden;
	}
private:
	ITransfer *m_transferFunction;
	unsigned int m_numInputs;
	unsigned int m_numOutputs;
	unsigned int m_numHidden;

	CLogisticalNeuron *m_pInputs;
	CLogisticalNeuron *m_pOutputs;
	CLogisticalNeuron *m_pHidden;

	ga_nn_value m_fMax;
	ga_nn_value m_fMin;

};
#endif