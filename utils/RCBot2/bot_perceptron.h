#ifndef __PERCEPTRON_H__
#define __PERCEPTRON_H__

#include "bot_ga_nn_const.h"
#include <vector>
using namespace std;

typedef float ga_nn_value;

class ITransfer
{
public:
	virtual ga_nn_value transfer ( ga_nn_value netInput ) = 0;
};


class CSigmoidTransfer : public ITransfer
{
public:
	ga_nn_value transfer ( ga_nn_value netInput );
};

class CPerceptron
{
public:

	static ga_nn_value m_fDefaultLearnRate;// = 0.5f;
	static ga_nn_value m_fDefaultBias;// = 1.0f;

	CPerceptron (unsigned int iInputs,ITransfer *transferFunction=NULL);

	void setWeights ( vector <ga_nn_value> weights );

	void input ( vector <ga_nn_value> inputs );

	ga_nn_value execute ();

	bool fired ();

	ga_nn_value getOutput ();

	void train ( ga_nn_value expectedOutput );

private:
	
	unsigned int m_iInputs;
	ga_nn_value m_Bias;
	ga_nn_value m_LearnRate;
	vector <ga_nn_value> m_inputs;
	vector <ga_nn_value> m_weights;
	ga_nn_value m_output;
	ITransfer *m_transferFunction;
};

#endif