/* Generated by Together */

#ifndef TPZCOPYSOLVE_H
#define TPZCOPYSOLVE_H
#include "pzsolve.h"
class TPZFMatrix;
class TPZCopySolve : public TPZMatrixSolver {
public:  

    /**
     *      Solves the system of linear equations stored in current matrix
     *      As this class implements only a copy operation, it just copies u to F;
     *      @param F contains Force vector
     *      @param result contains the solution
     */
  virtual void Solve(const TPZFMatrix &F, TPZFMatrix &result, TPZFMatrix * ){result=F;}  

    /**
     *      Clones the current object returning a pointer of type TPZSolver
      */
  virtual TPZSolver *Clone() const {return new TPZCopySolve;}

;
};
#endif //TPZCOPYSOLVE_H
