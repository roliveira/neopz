#ifndef CONSERVATIONLAWHPP
#define CONSERVATIONLAWHPP

#include <iostream>
#include "pzmaterial.h"
#include "pzfmatrix.h"
#include "pzvec.h"


class TPZConservationLaw  : public TPZMaterial {

  int fDim;
  REAL fTimeStep;
  
  public :

  REAL fDelta;
  
  TPZConservationLaw(int nummat,REAL delta_t,int dim);
  
  virtual ~TPZConservationLaw();

  /**
   * termodinamic pressure determined by the law of an ideal gas 
   */
  virtual REAL Pressure(TPZVec<REAL> &U);

  virtual REAL Gamma();

  //virtual int IntegrationDegree() = 0;

  //virtual void SetIntegDegree(int degree) = 0;
  
  void SetDelta(REAL delta){fDelta = delta;}
  
  REAL Delta();

  void SetTimeStep(REAL timestep){fTimeStep = timestep;}

  REAL TimeStep(){return fTimeStep;}

  int Dimension() { return fDim;}
  
  int NStateVariables();
  
  virtual void Print(ostream & out);
  
  char *Name() { return "TPZConservationLaw"; }
  
  virtual void Contribute(TPZVec<REAL> &x,TPZFMatrix &,TPZVec<REAL> &sol,TPZFMatrix &dsol,REAL weight,
			  TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &dphi,TPZFMatrix &ek,TPZFMatrix &ef);
  
  virtual void ContributeInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL,TPZVec<REAL> &solR,TPZFMatrix &dsolL,
				   TPZFMatrix &dsolR,REAL weight,TPZVec<REAL> &normal,TPZFMatrix &phiL,
				   TPZFMatrix &phiR,TPZFMatrix &dphiL,TPZFMatrix &dphiR,
				   TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond *bc);
  
  virtual void ContributeBC(TPZVec<REAL> &x,TPZVec<REAL> &sol,REAL weight,
			    TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc);
  
  virtual int VariableIndex(char *name);
  
  virtual int NSolutionVariables(int var);
  
  virtual int NFluxes(){ return 1;}
  
  virtual void Solution(TPZVec<REAL> &Sol,TPZFMatrix &DSol,TPZFMatrix &axes,int var,TPZVec<REAL> &Solout);
  
  /**compute the value of the flux function to be used by ZZ error estimator*/
  virtual void Flux(TPZVec<REAL> &x, TPZVec<REAL> &Sol, TPZFMatrix &DSol, TPZFMatrix &axes, TPZVec<REAL> &flux);
  
  void Errors(TPZVec<REAL> &x,TPZVec<REAL> &u,
	      TPZFMatrix &dudx, TPZFMatrix &axes, TPZVec<REAL> &flux,
	      TPZVec<REAL> &u_exact,TPZFMatrix &du_exact,TPZVec<REAL> &values);
};


inline void TPZConservationLaw::ContributeInterface(TPZVec<REAL> &,TPZVec<REAL> &,TPZVec<REAL> &,TPZFMatrix &,
						    TPZFMatrix &,REAL ,TPZVec<REAL> &,TPZFMatrix &,TPZFMatrix &,
						    TPZFMatrix &,TPZFMatrix &,TPZFMatrix &,TPZFMatrix &,TPZBndCond *){
  PZError << "TPZConservationLaw::ContributeInterface it would never have to be called\n";
}

inline void TPZConservationLaw::Solution(TPZVec<REAL> &Sol,TPZFMatrix &DSol,TPZFMatrix &axes,int var,TPZVec<REAL> &Solout){
  PZError << "TPZConservationLaw::Solution it would never have to be called\n";
}

inline REAL TPZConservationLaw::Pressure(TPZVec<REAL> &U) {
  PZError << "TPZConservationLaw::Pressure it would never have to be called\n";
  return 0.0;
}

inline REAL TPZConservationLaw::Gamma() {
  PZError << "TPZConservationLaw::Gamma it would never have to be called\n";
  return 0.0;
}
#endif
