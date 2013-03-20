#ifndef CNode_H
#define CNode_H

//------------------------------------------------------------------------
//
//  Name:  CNode.h
//
//  Desc:  class to describe a node for a Kohonen Self Organising Map
//
//------------------------------------------------------------------------

#include <vector>
#include <stdlib.h>
#include <iostream>

class CNode
{

private:

  //this node's weights
  std::vector<double>    m_weights;

  //its position within the lattice
  int               m_x,
  									m_y;


public:

  CNode(int x, int y, int numWeights);

  double SqEuclideanDistance(const std::vector<double> &inputVector);

  void   AdjustWeights(const std::vector<double> &vec,
                       const double         learningRate,
                       const double         influence);

  int X()const;
  int Y()const;

  std::vector<double>* getWeights();

};


#endif
