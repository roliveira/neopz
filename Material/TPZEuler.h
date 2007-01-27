/* Generated by Together */
class TPZFMatrix;
class TPZBndCond;
class TPZEuler;

#ifndef TPZEULER_H
#define TPZEULER_H
#include "pzmaterial.h"
#include "eulerdif.h"

/**
 * This class implements a linear scalar convection equation with modified 
 * SUPG difusion
 */
class TPZEuler : public TPZMaterial {
public:  
    TPZEuler(TPZEuler & copy);

    TPZEuler(int id, REAL deltat) ;

  /** 
   * Set the state of the Euler material
   * state = 0 -> L2 projection
   * state = 1 -> Euler time stepping
   **/
  void SetState(int state) {
    fState = state;
  }


    /**returns the integrable dimension of the material*/
    virtual int Dimension() ;

    /** returns the number of state variables associated with the material*/
    virtual int NStateVariables()  ;

    /** return the number of components which form the flux function*/
    virtual int NFluxes() {return 2;}


    virtual void ContributeBC(TPZVec<REAL> &x,TPZVec<REAL> &sol,REAL weight,
    			    TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc);

    /** print out the data associated with the material*/
    virtual void Print(std::ostream &out = std::cout);

    /**returns the variable index associated with the name*/
    virtual int VariableIndex(char *name);

    /** returns the number of variables associated with the variable indexed by var.
   *       var is obtained by calling VariableIndex*/
    virtual int NSolutionVariables(int var);

    /**returns the solution associated with the var index based on the finite element approximation*/
    virtual void Solution(TPZVec<REAL> &Sol,TPZFMatrix &DSol,TPZFMatrix &axes,int var,TPZVec<REAL> &Solout);

    /**compute the value of the flux function to be used by ZZ error estimator*/
    virtual void Flux(TPZVec<REAL> &x, TPZVec<REAL> &Sol, TPZFMatrix &DSol, TPZFMatrix &axes, TPZVec<REAL> &flux) {}

    /**To create another material of the same type*/
    virtual TPZAutoPointer<TPZMaterial> NewMaterial();

    /**Read data of the material from a istream (file data)*/
    virtual void SetData(std::istream &data);

    /**
       * Contribute adds the contribution to the stiffness matrix
       **/
//    virtual void Contribute(TPZVec<REAL> &x,TPZFMatrix &jacinv, TPZVec<REAL> &sol,TPZFMatrix &dsol, REAL weight,
//  			  TPZFMatrix &axes,TPZFMatrix &daxesdksi, TPZFMatrix &phi,TPZFMatrix &dphi,TPZFMatrix &ek,TPZFMatrix &ef);

    /**Compute contribution to the stiffness matrix and right hand side at an integration point*/
    virtual void Contribute(TPZVec<REAL> &x,TPZFMatrix &jacinv, TPZVec<REAL> &sol,TPZFMatrix &dsol, REAL weight,
  			  TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &dphi,TPZFMatrix &ek,TPZFMatrix &ef) ;

private:    

  static TEulerDiffusivity gEul;
  REAL fDeltaT;
  int fState;
};
#endif //TPZEULER_H
