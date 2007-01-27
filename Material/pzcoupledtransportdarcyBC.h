// -*- c++ -*-

//$Id: pzcoupledtransportdarcyBC.h,v 1.4 2007-01-27 14:49:27 phil Exp $

#ifndef MATCOUPLEDTRANSPDARCYBC
#define MATCOUPLEDTRANSPDARCYBC

#include <iostream>

#include "pzreal.h"
#include "pzbndcond.h"
#include "pzvec.h"
#include "pzmanvector.h"
#include "pzfmatrix.h"
#include "pzmaterialid.h"
#include "pzcoupledtransportdarcy.h"


class TPZCoupledTransportDarcyBC : public TPZBndCond{

protected:

  TPZBndCond * fMaterials[2];
  
  TPZBndCond * GetNonNullMaterial(){
    if (this->fMaterials[0]) return this->fMaterials[0];
    if (this->fMaterials[1]) return this->fMaterials[1];
    PZError << "Error! - "  << __PRETTY_FUNCTION__ << std::endl;
    exit (-1);
  }
  
  void UpdateConvectionDir(TPZFMatrix &dsol);
  void UpdateConvectionDirInterface(TPZFMatrix &dsolL, TPZFMatrix &dsolR, TPZFMatrix &phiL, TPZFMatrix &phiR);
  
public :

  TPZCoupledTransportDarcyBC(TPZCoupledTransportDarcy * material, int id);
  
  ~TPZCoupledTransportDarcyBC();

  TPZBndCond * GetCurrentMaterial(){
    const int eq = TPZCoupledTransportDarcy::CurrentEquation();
    if (eq == 0 || eq == 1) return this->fMaterials[eq];
    else {
      PZError << "Error! - " << __PRETTY_FUNCTION__ << std::endl;
      exit (-1);
    }
  }  
  
  virtual int HasForcingFunction() {
    TPZBndCond * bc = this->GetCurrentMaterial();
    if (bc) return bc->HasForcingFunction();
    return 0;
  }
  
  void SetMaterial(int eq, TPZBndCond * mat){
    if (eq == 0 || eq == 1) this->fMaterials[eq] = mat;
    else {
        PZError << "Error! - " << __PRETTY_FUNCTION__ << std::endl;
        exit (-1);
    }
  }
  
  /**returns the integrable dimension of the material*/
  int Dimension() { 
    return this->GetNonNullMaterial()->Dimension();
  }

  virtual int NFluxes(){ return this->GetNonNullMaterial()->NFluxes(); }

  int NStateVariables() { return this->GetNonNullMaterial()->NStateVariables(); }

  /**
   * Returns the number of norm errors. Default is 3: energy, L2 and H1.
   */
  virtual int NEvalErrors() {return this->GetNonNullMaterial()->NEvalErrors();}

  /**compute the value of the flux function to be used by ZZ error estimator*/
  void Flux(TPZVec<REAL> &x, TPZVec<REAL> &Sol, TPZFMatrix &DSol, TPZFMatrix &axes, TPZVec<REAL> &flux){
    flux.Fill(0.);
  }

  void Print(std::ostream & out = std::cout) {
    out << " Boundary condition number = " << Id() << "\n";
  }

  void Contribute(TPZVec<REAL> &x,TPZFMatrix &jacinv, TPZVec<REAL> &sol,TPZFMatrix &dsol, REAL
	weight,TPZFMatrix &axes,TPZFMatrix &phi,TPZFMatrix &dphi,TPZFMatrix &ek,TPZFMatrix &ef);

  void ContributeBC(TPZVec<REAL> &x,TPZVec<REAL> &sol,REAL weight,TPZFMatrix &axes,
		    TPZFMatrix &phi,TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc) {  }

  void Errors(TPZVec<REAL> &x,TPZVec<REAL> &sol,TPZFMatrix &dsol, TPZFMatrix &axes, TPZVec<REAL> &flux,
	      TPZVec<REAL> &uexact,TPZFMatrix &duexact,TPZVec<REAL> &val){
    val.Fill(0.);
  }

  virtual void ContributeInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL,TPZVec<REAL> &solR,TPZFMatrix &dsolL,
				   TPZFMatrix &dsolR,REAL weight,TPZVec<REAL> &normal,TPZFMatrix &phiL,
				   TPZFMatrix &phiR,TPZFMatrix &dphiL,TPZFMatrix &dphiR,
       TPZFMatrix &axesleft, TPZFMatrix &axesright,
       TPZFMatrix &ek,TPZFMatrix &ef);

  virtual void ContributeInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL,TPZVec<REAL> &solR,TPZFMatrix &dsolL,
				   TPZFMatrix &dsolR,REAL weight,TPZVec<REAL> &normal,TPZFMatrix &phiL,
				   TPZFMatrix &phiR,TPZFMatrix &dphiL,TPZFMatrix &dphiR,
       TPZFMatrix &axesleft, TPZFMatrix &axesright,
       TPZFMatrix &ef);
  
  virtual void ContributeInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL,TPZVec<REAL> &solR,TPZFMatrix &dsolL,
				   TPZFMatrix &dsolR,REAL weight,TPZVec<REAL> &normal,TPZFMatrix &phiL,
				   TPZFMatrix &phiR,TPZFMatrix &dphiL,TPZFMatrix &dphiR,
       TPZFMatrix &axesleft, TPZFMatrix &axesright,
       TPZFMatrix &ek,TPZFMatrix &ef, int LeftPOrder, int RightPOrder, REAL faceSize);

  virtual void ContributeBCInterface(TPZVec<REAL> &x,TPZVec<REAL> &solL, TPZFMatrix &dsolL, REAL weight, TPZVec<REAL> &normal,
    TPZFMatrix &phiL,TPZFMatrix &dphiL, 
    TPZFMatrix &axesleft,
    TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc) {
    //NOTHING TO BE DONE HERE
  }

};


#endif
