//$Id: pzpostprocanalysis.h,v 1.5 2010-10-20 18:41:37 diogo Exp $
#ifndef PZPOSTPROCANALYSIS_H
#define PZPOSTPROCANALYSIS_H

#include "pzanalysis.h"
#include "pzcompel.h"
#include "TPZGeoElement.h"
#include "pzfmatrix.h"
#include "pzvec.h"
#include "pzstrmatrix.h"


#include <iostream>
#include <string>


/**
 * The Post Processing Analysis makes the interface among the computational
 * analysis and itself, being also in charge of getting the current solution
 * from the main analysis.
 */

class TPZPostProcAnalysis : public TPZAnalysis {

public:

TPZPostProcAnalysis(TPZCompMesh * pRef);

TPZPostProcAnalysis();
    
    TPZPostProcAnalysis(const TPZPostProcAnalysis &copy);
    
    TPZPostProcAnalysis &operator=(const TPZPostProcAnalysis &copy);

    virtual ~TPZPostProcAnalysis();
	
    /// Set the computational mesh we are going to post process
    void SetCompMesh(TPZCompMesh *pRef, bool mustOptimizeBandwidth = false) override;
    
    TPZCompMesh *ReferenceCompMesh()
    {
        return fpMainMesh;
    }
/**
 *	Assemble() blank implementation in order to avoid its usage. In such an Analysis
 * class the Assemble() method is useless.
 */
virtual  void Assemble() override;

/**
 *	Solve() blank implementation in order to avoid its usage. In such an Analysis
 * class the Solve() method is useless.
 */

virtual void Solve() override;

/** 
 * TransferSolution is in charge of transferring the solution from the base analysis/mesh
 * to the current post processing mesh, solving for the dof to provide a LSM extrapolation.
 */
void TransferSolution();

/**
 * Informs the material IDs and the variable names that are to be postprocessed, if
 * matching. Should be called right after the class instantiation
 */
void SetPostProcessVariables(TPZVec<int> & matIds, TPZVec<std::string> &varNames);
		
static void SetAllCreateFunctionsPostProc(TPZCompMesh *cmesh);
static void SetAllCreateFunctionsContinuous();
		void AutoBuildDisc();
    
    /** @brief Returns the unique identifier for reading/writing objects to streams */
	public:
int ClassId() const override;

	/** @brief Save the element data to a stream */
	void Write(TPZStream &buf, int withclassid) const override;
	
	/** @brief Read the element data from a stream */
	void Read(TPZStream &buf, void *context) override;
    

protected:
	
	TPZCompMesh * fpMainMesh;

/**
 * TPZCompElPostProc<TCOMPEL> creation function setup
 */

public:

	
static TPZCompEl * CreatePostProcDisc(  TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);
	
static TPZCompEl * CreatePointEl( TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);
static TPZCompEl * CreateLinearEl( TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);
static TPZCompEl * CreateQuadEl( TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);
static TPZCompEl * CreateTriangleEl( TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);
static TPZCompEl * CreateCubeEl( TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);
static TPZCompEl * CreatePyramEl( TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);
static TPZCompEl * CreateTetraEl( TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);
static TPZCompEl * CreatePrismEl( TPZGeoEl *gel, TPZCompMesh &mesh, int64_t &index);

};

#endif

