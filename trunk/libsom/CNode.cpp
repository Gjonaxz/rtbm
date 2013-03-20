
//------------------------------------------------------------------------
//
//  Name:  CNode.cpp
//
//  Desc:  Implementation of class to describe a node for a Kohonen Self Organising Map
//
//------------------------------------------------------------------------

#include "CNode.h"


CNode::CNode(int x, int y, int numWeights):m_x(x),
                                           m_y(y)
{
  //initialize the weights to small random variables
  for (int w=0; w<numWeights; ++w)
  {
    m_weights.push_back(rand()/(RAND_MAX+1.0));
  }
}


//-------------------------- CalculateDistance ---------------------------
//
//  returns the euclidean distance (squared) between the node's weights 
//  and the input vector
//------------------------------------------------------------------------
double CNode::SqEuclideanDistance(const std::vector<double> &inputVector)
{
  double distanceSq = 0;
  
  for (int i=0; i<m_weights.size(); ++i)
  {
    distanceSq += (inputVector[i] - m_weights[i]) * 
                  (inputVector[i] - m_weights[i]);
  }

  return distanceSq;
}

//-------------------------- AdjustWeights -------------------------------
//
//  Given a learning rate and a target vector this function adjusts
//  the node's weights accordingly
//------------------------------------------------------------------------
void CNode::AdjustWeights(const std::vector<double> &target,
                          const double learningRate,
                          const double influence)
{
	if (m_weights.size() != target.size()){
		std::cout << "bad input, node has:" << m_weights.size() << ", target has: " << target.size() << std::endl;
		return;
	}
  for (int w=0; w<target.size(); ++w)
  {
    m_weights[w] += learningRate * influence * (target[w] - m_weights[w]);
  }
}

//-------------------------- Getters -------------------------------------
//
//  Return the X and Y coordinates of the node
//  
//------------------------------------------------------------------------
int CNode::X()const{return m_x;}
int CNode::Y()const{return m_y;}


std::vector<double>* CNode::getWeights(){
	return &m_weights;
}
