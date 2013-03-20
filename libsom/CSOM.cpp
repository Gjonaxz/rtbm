#include "CSOM.h"

#include <typeinfo>

template<typename T>
std::istream & binary_read(std::istream* stream, T& value){
	return stream->read(reinterpret_cast<char*>(&value), sizeof(T));
}

template<typename T>
std::ostream& binary_write(std::ostream* outStream, const T& value){
	return outStream->write(reinterpret_cast<const char*>(&value), sizeof(T));
}


CSOM::CSOM(int width,
					int height,
					int numInputs):m_height(height),
													m_width(width),
													m_numInputNodes(numInputs)
{  
	//create all the nodes
	for (int y=0; y<m_height; ++y)
	{
		for (int x=0; x<m_width; ++x)
		{
			m_SOM.push_back(CNode(x, y, numInputs));   //num weights
		}
	}

	//this is the topological 'radius' of the feature map
	m_mapRadius = std::max(m_width, m_height)/2;

}  

//-------------------------- setTrainingValuesStruct ------------------------------
//
//  Prepares a structure that will be used during the training process.
//  It receives the vector that will be used during the training, the
//  number of iterations and the initial learning rate
//------------------------------------------------------------------------
void CSOM::setTrainingValues(std::vector<std::vector<double> > &input, int numIterations, double initialLearningRate){
	//set λ, used in the calculation of the neighbourhood width of influence
	const int timeConstant = numIterations / log(m_mapRadius);

	m_currentInputData = input;
	m_numIterations = numIterations;
	m_initialLearningRate = initialLearningRate;
	m_timeConstant = timeConstant;
	
//   TrainingValuesStruct tv = {input, numIterations, initialLearningRate, timeConstant};

	//make sure the size of the input vector matches the size of each node's 
	//weight vector
//   if (input[0].size() != m_numInputNodes) return false; //TODO find a smart way to handle this check

	//how much the learning rate is adjusted for nodes within
	//the area of influence
	m_learningRate = m_initialLearningRate;

	//set the iteration to start at 1. One division would give 0 if we start with 0
	m_iteration = 1;
	
	//empty the BMU list
	m_BMU.clear();
	m_BMU.reserve(m_currentInputData.size());

	//enter the training loop
//   m_tv = &tv;
//   std::cout << "iteration " << tv.iteration << std::endl;
}

//--------------------------- trainNextIteration -------------------------
//
//  Given a structure with the training values. It will train the SOM 
//  one epoch with every input from the vector and return the control 
//  to the caller.
//------------------------------------------------------------------------
bool CSOM::trainNextIteration()
{	
	//If we reached the number of iterations + 1 (started at 1), end the loop
	if (m_iteration > m_numIterations) return false;
	
	//calculate the width of the neighbourhood for this iteration
	m_nbhRadius = m_mapRadius * exp(-(double)m_iteration/m_timeConstant);

	//For each of the input vectors, look for the best matching
	//unit, then adjust the weights for the BMUs neighborhood
	for (int thisVector=0; thisVector<m_currentInputData.size(); thisVector++){		
		//present the vector to each node and determine the BMU
		CNode *bmu = getBMU(m_currentInputData[thisVector]);
		
		//add the BMU to the list of BMUs
	if (m_BMU.size() == m_currentInputData.size()){
		m_BMU[thisVector] = bmu;
	} else {
		m_BMU.push_back(bmu);
	}

		//Now to adjust the weight vector of the BMU and its
		//neighbours

		//For each node calculate the influence (Theta from equation 6 in
		//the tutorial. If it is greater than zero adjust the node's weights
		//accordingly
		for (int n=0; n<m_SOM.size(); ++n)
		{
			//calculate the Euclidean distance (squared) to this node from the
			//BMU
			double DistToNodeSq = (bmu->X()-m_SOM[n].X()) *
														(bmu->X()-m_SOM[n].X()) +
														(bmu->Y()-m_SOM[n].Y()) *
														(bmu->Y()-m_SOM[n].Y()) ;

			double WidthSq = m_nbhRadius * m_nbhRadius;

			//if within the neighbourhood adjust its weights
			if (DistToNodeSq < WidthSq)
			{

				//calculate by how much its weights are adjusted
				m_influence = exp(-(DistToNodeSq) / (2*WidthSq*m_iteration));

				m_SOM[n].AdjustWeights(m_currentInputData[thisVector],
															m_learningRate,
															m_influence);
			}

		}//next node

	}//next input data

	//reduce the learning rate
	m_learningRate = m_initialLearningRate * exp(-(double)m_iteration/(m_numIterations));
		
	m_iteration++;
	
//  for (int i=0; i<m_BMU.size(); i++){
//		std::cout << m_BMU[i]->X() << ", " << m_BMU[i]->Y() << std::endl;
//	}	

	return true;
}

std::vector<CNode*> * CSOM::getBMUlist()
{
	return &m_BMU;
}


//--------------------- CalculateBestMatchingNode ------------------------
//
//  this function presents an input vector to each node in the network
//  and calculates the Euclidean distance between the vectors for each
//  node. It returns a pointer to the best performer
//------------------------------------------------------------------------
CNode* CSOM::getBMU(std::vector<double> &vec)
{
	CNode* bmu = NULL;

	double LowestDistance = 999999;

	for (int n=0; n<m_SOM.size(); ++n)
	{
		double dist = m_SOM[n].SqEuclideanDistance(vec);

		if (dist < LowestDistance)
		{
			LowestDistance = dist;

			bmu = &m_SOM[n];
		}
	}

	return bmu;
}


//--------------------- ExportTheCurrentSOM (BMUs and Weights) -----------
//
//  this function receives a filename to export to it, the BMUs and
//  weights of the SOM
//------------------------------------------------------------------------
void CSOM::exportState(const char * filename)
{
	std::ofstream file;
	file.open (filename, std::ios::binary);
	std::vector<double> * weights;
	std::cout << std::setprecision(15);
	
	//Check that the file has been succesfully opened
	if (file.fail()){
		std::cout << "Couldn't open " << filename << std::endl;
		return;
	}
	
	binary_write(&file, (int)m_SOM.size()); //Save the size of the SOM
	binary_write(&file, m_numInputNodes); //Save the number of inputs for each node of the SOM
	
// 	std::cout << m_SOM.size() << ", " << m_numInputNodes << std::endl;
	
	for (int i=0 ; i<m_SOM.size() ; i++){
		weights = m_SOM[i].getWeights(); //For each of the nodes
		for (int j=0 ; j<m_numInputNodes ; j++){
			binary_write(&file, (*weights)[j]); //Save each of the weights
// 			std::cout << (*weights)[j] << std::endl;
		}
// 		std::cout << std::endl;
	}
	
	//Save the list of BMUs
	binary_write(&file, (int)m_BMU.size()); //Save the number of BMUs on the list
	for (int i=0; i<m_BMU.size(); i++){
		binary_write(&file, (int)(*m_BMU[i]).X()); //Save the X position
		binary_write(&file, (int)(*m_BMU[i]).Y()); //Save the Y position
		std::cout << (int)(*m_BMU[i]).X() << ", " << (int)(*m_BMU[i]).Y() << std::endl;

	}
	file.close();
}

//--------------------- ImportTheCurrentSOM (BMUs and Weights) -----------
//
//  this function receives a filename to import from it, the BMUs and
//  weights of the SOM
//------------------------------------------------------------------------
void CSOM::importState(const char * filename)
{
	std::cout << "Importing state from: " << filename << std::endl;
	std::ifstream file;
	file.open (filename, std::ifstream::in | std::ios::binary);
	int t_som_size, t_numInputNodes, t_bmu_size, t_bmu_x, t_bmu_y;
	std::vector<double> * weights;
	double w;

	//Check that the file has been succesfully opened
	if (file.fail()){
		std::cout << "Couldn't open " << filename << std::endl;
		return;
	}
	
	std::cout << std::setprecision(15);
	std::cout << std::noshowbase;

	//check SOM size
	binary_read(&file, t_som_size);
	if (t_som_size != m_SOM.size()){
		std::cout << "can't import, differnt SOM sizes" << std::endl;
		return;
	}
	
	//check weights size
	binary_read(&file, t_numInputNodes);
	if (t_numInputNodes != m_numInputNodes){
		std::cout << "can't import, differnt SOM sizes" << std::endl;
		return;
	}

// 	std::cout << t_som_size << std::endl;
// 	std::cout << t_numInputNodes << std::endl;
	
	for (int i=0 ; i<t_som_size ; i++){
		weights = m_SOM[i].getWeights();
		for (int j=0 ; j<t_numInputNodes ; j++){
			binary_read(&file, w);
			(*weights)[j] = w; // ...it would be nice to have the getWeights() method returning a const, but that would not allow this line.
// 			std::cout << (*weights)[j] << "," << w << std::endl;
		}
	}
	
	binary_read(&file, t_bmu_size); //Read the number of BMUs on the list
	m_BMU.clear();
	for (int i=0 ; i<t_bmu_size ; i++){
		binary_read(&file, t_bmu_x); //Read the X position
		binary_read(&file, t_bmu_y); //Read the Y position
		m_BMU.push_back(getNode(t_bmu_x,t_bmu_y));
// 		std::cout << t_bmu_x << "," << t_bmu_x << std::endl;
	}
	
	file.close();
}


CNode* CSOM::getNode(int x, int y){//TODO: Not sure if worth it, no consistency problem, implement locks to not modify the SOM while it is being read. Maybe its better to get the whole SOM and pick each node outside...more consistent
	return &m_SOM[y*m_width+x];
}
int CSOM::getWidth(){
	return m_width;
}
int CSOM::getHeight(){
	return m_height;
}
int CSOM::getNumInputNodes(){
	return m_numInputNodes;
}


double CSOM::query(double *sample, int sampleLenght){
	std::vector<double> csample;
	int t_pos;
	
	if (sampleLenght != m_numInputNodes) {
		std::cout << "error, invalid number of inputs in the sample to evaluate." << std::endl;
		return -1;
	}
	
	for (int i=0 ; i<sampleLenght ; i++){
		csample.push_back(sample[i]);
	}

	CNode* bmu = NULL;
	double LowestDistance = 999999;
	for (int n=0; n<m_BMU.size(); ++n){
		double dist = m_BMU[n]->SqEuclideanDistance(csample);
		if (dist < LowestDistance){
			LowestDistance = dist;
			bmu = m_BMU[n];
			t_pos = n;
		}
	}
	
/*	std::cout << std::setprecision(15);
	std::cout << std::noshowbase;*/
/*	std::cout << t_pos << ";" << bmu->X() << ", " << bmu->Y();
	std::cout << ";";*/
	printf ("%i;%i,%i;%.20f", t_pos, bmu->X(), bmu->Y(), LowestDistance);
// 	std::cout << std::endl;
/*	std::cout << "tamano " << sampleLenght << std::endl;*/
// 	for (int i=0 ; i<sampleLenght ; i++){
// 		std::cout << sample[i] << ",";
// 	}
// 	std::cout << std::endl;
	return LowestDistance;
}

extern "C" {
	CSOM* CSOM_new(int width, int height, int numInputs){ return new CSOM(width, height, numInputs); }
	void CSOM_importState(CSOM* csom, const char * filename){ csom->importState(filename); }
	double CSOM_query(CSOM* csom, double *sample, int size){ return csom->query(sample, size); }
	int CSOM_getWidth(CSOM* csom){ return csom->getWidth(); }
	int CSOM_getHeight(CSOM* csom){ return csom->getHeight(); }
	int CSOM_getNumInputNodes(CSOM* csom){ return csom->getNumInputNodes(); }
	double * CSOM_getNodeWeights(CSOM* csom, int x, int y){ 
		double *destination = new double[3];
		std::vector<double>* weightsVector = (*csom->getNode(x, y)).getWeights();
		memcpy( destination, &(*weightsVector)[0], sizeof( double ) * (weightsVector->size()) );
		//TODO: put a '\0' on the last position
		return destination;
	}
	void CSOM_setTrainingValues(CSOM* csom, double *input, int inputLenght, int numIterations, double initialLearningRate){
		std::vector<std::vector<double> > trainingSet;
		std::vector<double> tmpInput;

		int s=0;
		int i=0;
		for (s=0 ; s<inputLenght ; s++){
			if (!(i<csom->getNumInputNodes())){
				trainingSet.push_back(tmpInput);
				tmpInput.clear();
				i=0;
			}
			tmpInput.push_back(input[s]);
			i++;
		}
		csom->setTrainingValues(trainingSet, numIterations, initialLearningRate);
	}
	bool CSOM_trainNextIteration(CSOM* csom){ return csom->trainNextIteration(); }
}



