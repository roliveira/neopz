/* Generated by Together */

#ifndef TPZNLMGANALYSIS_H
#define TPZNLMGANALYSIS_H

#include "pzanalysis.h"

class TPZTransfe;
class TPZInterpolatedElement;
class TPZTransform;
class TPZStepSolver;

template<class T, class V>

class TPZAvlMap;
class TPZOneDRef;
class TPZGeoEl;


/**
 * Class TPZNonLinMultGridAnalysis derived from TPZAnalysis
 * @ingroup Analysis
 */

class TPZNonLinMultGridAnalysis : public TPZAnalysis {
  
  /**
   * Contains the computational meshes of one cycle: V, W, F, etc
   */
  TPZStack < TPZCompMesh * > fMeshes;
  
  /**
   * Contains the meshes solutions
   */	
  TPZStack <TPZFMatrix *> fSolutions;
  
  /**
   * Contains the solution method applied to the mesh
   */
  TPZStack <TPZMatrixSolver *> fSolvers;
  
  /**
   * Contains the preconditioner of the solution method
   * if the solution method is a krylov method, the preconditioner
   * can be used as a coarse grid iteration
   */
  TPZStack <TPZMatrixSolver *> fPrecondition;

 public:
  
  /**
   * Destructor
   */
  virtual ~TPZNonLinMultGridAnalysis();
  
  /**
   * Creates an object multigrid analysis
   * giving a computational mesh
   */
  TPZNonLinMultGridAnalysis(TPZCompMesh *cmesh);
  
  /**
   * Append a mesh to the meshes vector
   */
  void AppendMesh (TPZCompMesh * mesh);
  
  /**
   * Pop the last mesh of the meshes vector
   */
  TPZCompMesh *PopMesh ();
  
  /**
   * Uses fSolver object to apply a solution
   * algorithm
   */
  virtual void Solve ();

  /**
   * it creates a new established computational mesh in the refinement uniform 
   * of a fixed number of levels of the original geometric mesh
   * coarcmesh : initial computational mesh
   * levelnumbertorefine: number of the levels to be refined
   * setdegree: 
   */
  static TPZCompMesh *UniformlyRefineMesh (TPZCompMesh *coarcmesh,int levelnumbertorefine,int setdegree);

   /**
   * Proceeds the uniformly h-p refinement of mesh
   */
  static TPZCompMesh *AgglomerateMesh (TPZCompMesh *mesh);

  /**
   * Loads the last two solutions and
   * call the error between these two aproximations
   */
  void ComputeError (TPZVec<REAL> &error);

  static void GetRefinedGeoEls(TPZGeoEl *geo,TPZStack<TPZGeoEl *> sub,int &levelnumbertorefine);
  
  /**
   * Validation routines
   */
  static int main ();

};
#endif
