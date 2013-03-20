#ifndef CSOM_H
#define CSOM_H


#include "CNode.h"
#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string.h>

class CSOM
{

private:

  //the neurons representing the Self Organizing Map
  std::vector<CNode>           m_SOM;

  //the neurons representing the BMUs of the SOM
  std::vector<CNode*>           m_BMU;
  
  //the nodes representing the error map
//   std::vector<bool>           m_SOM_err;

  //this is the topological 'radius' of the feature map
  double                  m_mapRadius;

  //the height and width of the SOM
  int                     m_width;
  int                     m_height;

  //the number of inputs of the SOM
  int                     m_numInputNodes;
  
  std::vector<std::vector<double> > m_currentInputData;    //input data
  int                     m_numIterations;       //total number of iterations the training will do
  double                  m_initialLearningRate; //the initial learning rate
  int                     m_timeConstant;        //the time contstant
  int                     m_iteration;           //the current iteration
  double                  m_influence;           //defines how much will the weights of a node change, reduces as it is pvomg away of the bmu 
  double                  m_learningRate;        //changing learning rate, reduced with each iteration
  double                  m_nbhRadius;           //the radius of the neighbourhood that will be affected

  CNode*                  getBMU(std::vector<double> &vec);
public:

  CSOM(int width, int height, int numInputs);
  bool trainNextIteration();
  void setTrainingValues(std::vector<std::vector<double> > &input, int numIterations, double initialLearningRate);
  CNode* getNode(int x, int y);
  int getWidth();
  int getHeight();
  int getNumInputNodes();
	void importState(const char * filename);
	void exportState(const char * filename);
	double query(double *sample, int size);
  std::vector<CNode*> * getBMUlist();

};


#endif
