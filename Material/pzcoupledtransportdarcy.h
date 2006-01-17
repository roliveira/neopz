// -*- c++ -*-

//$Id: pzcoupledtransportdarcy.h,v 1.2 2006-01-17 11:27:44 tiago Exp $

#ifndef MATCOUPLEDTRANSPDARCY
#define MATCOUPLEDTRANSPDARCY

#include <iostream>
#include "pzdiscgal.h"
#include "pzfmatrix.h"
#include "pzpoisson3d.h"

class TPZCoupledTransportDarcyBC;

/**
 * Implements two equations where the second one requires the solution of the first.
 * First equation is  -div[ K Grad[p] ] = 0
 * Second equation is -div[ A Grad[u] ] + beta . Grad[u] = 0,
 * where K and A are constants and beta = alpha * (-K Grad[p] )
 * Both equations are implemented by TPZMatPoisson3d material.
 * This material has two instances of TPZMatPoisson3d. In a first moment
 * it behaves like first equation. Once the problem is solved, it behaves
 * like the second equation. In the latter case beta value is set when Contribute
 * is called by processing the solution of the previous equation.
 */
class TPZCoupledTransportDarcy : public TPZDiscontinuousGalerkin {
 
  protected :

  /** 
   * In second equation beta = alpha * (-K Grad[p] ). Here alpha is stored.
   */
  REAL fAlpha;

  /** 
   * Two instances of TPZMatPoisson3d 
   */  
  TPZMatPoisson3d * fMaterials[2];
  
  static int gCurrentEq;
  
  void UpdateConvectionDir(TPZFMatrix &dsol);
  void UpdateConvectionDirInterface(TPZFMatrix &dsolL, TPZFMatrix &dsolR);
  
public:

  virtual TPZBndCond *CreateBC(int id, int typ, TPZFMatrix &val1,TPZFMatrix &val2){
    PZError << "Error! - This method should not be called - " << __PRETTY_FUNCTION__ << std::endl;
  }

  TPZCoupledTransportDarcyBC *CreateBC(int id);
  
  static void SetCurrentMaterial(const int i);
  
  static int CurrentEquation();
  
  virtual int HasForcingFunction() {return this->GetCurrentMaterial()->HasForcingFunction();}
  
  TPZMatPoisson3d * GetCurrentMaterial(){
#ifdef DEBUG
  if (!this->fMaterials[0] || !this->fMaterials[1]){
      PZError << "Error! - " << __PRETTY_FUNCTION__ << std::endl;
      exit (-1);
  }  
#endif  
    if (TPZCoupledTransportDarcy::gCurrentEq == 0 ||
        TPZCoupledTransportDarcy::gCurrentEq == 1) return this->fMaterials[TPZCoupledTransportDarcy::gCurrentEq];
    else {
      PZError << "Error! - " << __PRETTY_FUNCTION__ << std::endl;
      exit (-1);
    }
  }
  
  TPZMatPoisson3d * GetMaterial(int eq){
#ifdef DEBUG
  if (!this->fMaterials[0] || !this->fMaterials[1]){
      PZError << "Error! - " << __PRETTY_FUNCTION__ << std::endl;
      exit (-1);
  }  
#endif    
    if (eq == 0 || eq ==1) return this->fMaterials[eq];
    else {
      PZError << " Error - " << __PRETTY_FUNCTION__ << std::endl;
      exit (-1);
    }
  }

  TPZCoupledTransportDarcy(int nummat, int nummat0, int nummat1, int dim);

  virtual ~TPZCoupledTransportDarcy();

  TPZCoupledTransportDarcy(TPZCoupledTransportDarcy &copy) : TPZDiscontinuousGalerkin(copy), 
    fAlpha(copy.fAlpha){
    this->fMaterials[0] = dynamic_cast<TPZMatPoisson3d*> (copy.fMaterials[0]->NewMaterial());
    this->fMaterials[1] = dynamic_cast<TPZMatPoisson3d*> (copy.fMaterials[1]->NewMaterial());
  }
  
  virtual TPZMaterial *NewMaterial(){
    return new TPZCoupledTransportDarcy(*this);
  }
    
  int Dimension() { 
    this->GetCurrentMaterial()->Dimension();
  }

  int NStateVariables();

  void SetAlpha(REAL alpha);

  virtual void Print(std::ostream & out);
  
  char *Name() { return "TPZCoupledTransportDarcy"; }
  
  virtual void Contribute(TPZVec<REAL> &x,TPZFMatrix &jacinv,TPZVec<REAL> &sol,TPZFMatrix &dsol,REAL weight,
			  TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &dphi,TPZFMatrix &ek,TPZFMatrix &ef);

  virtual void ContributeBC(TPZVec<REAL> &x,TPZVec<REAL> &sol,REAL weight,
			    TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc);

  virtual int VariableIndex(char *name);
  
  virtual int NSolutionVariables(int var);
  
  virtual int NFluxes(){ return 3;}
  
  virtual void Solution(TPZVec<REAL> &Sol,TPZFMatrix &DSol,TPZFMatrix &axes,int var,TPZVec<REAL> &Solout);
  
  /**compute the value of the flux function to be used by ZZ error estimator*/
  virtual void Flux(TPZVec<REAL> &x, TPZVec<REAL> &Sol, TPZFMatrix &DSol, TPZFMatrix &axes, TPZVec<REAL> &flux);
  
  void Errors(TPZVec<REAL> &x,TPZVec<REAL> &u,
	      TPZFMatrix &dudx, TPZFMatrix &axes, TPZVec<REAL> &flux,
	      TPZVec<REAL> &u_exact,TPZFMatrix &du_exact,TPZVec<REAL> &values);//Cedric

  virtual void ContributeInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL,TPZVec<REAL> &solR,TPZFMatrix &dsolL,
				   TPZFMatrix &dsolR,REAL weight,TPZVec<REAL> &normal,TPZFMatrix &phiL,
				   TPZFMatrix &phiR,TPZFMatrix &dphiL,TPZFMatrix &dphiR,
				   TPZFMatrix &ek,TPZFMatrix &ef);
  
  virtual void ContributeBCInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL, TPZFMatrix &dsolL, REAL weight, TPZVec<REAL> &normal,
			    TPZFMatrix &phiL,TPZFMatrix &dphiL, TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc);
			    
  virtual void ContributeInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL,TPZVec<REAL> &solR,TPZFMatrix &dsolL,
				   TPZFMatrix &dsolR,REAL weight,TPZVec<REAL> &normal,TPZFMatrix &phiL,
				   TPZFMatrix &phiR,TPZFMatrix &dphiL,TPZFMatrix &dphiR,
				   TPZFMatrix &ek,TPZFMatrix &ef, int LeftPOrder, int RightPOrder, REAL faceSize);
				   
  virtual void ContributeBCInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL, TPZFMatrix &dsolL, REAL weight, TPZVec<REAL> &normal,
				     TPZFMatrix &phiL,TPZFMatrix &dphiL, TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc, int POrder, REAL faceSize);

  void InterfaceErrors(TPZVec<REAL> &/*x*/,
		       TPZVec<REAL> &leftu, TPZFMatrix &leftdudx, /* TPZFMatrix &leftaxes,*/ 
		       TPZVec<REAL> &rightu, TPZFMatrix &rightdudx, /* TPZFMatrix &rightaxes,*/ 
		       TPZVec<REAL> &/*flux*/,
		       TPZVec<REAL> &u_exact,TPZFMatrix &du_exact,TPZVec<REAL> &values, 
		       TPZVec<REAL> normal, REAL elsize);

  virtual int IsInterfaceConservative(){ return 1;}

};

#endif
