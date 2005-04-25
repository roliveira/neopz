/* Generated by Together */
//class TPZEqnArray;

#ifndef TPZFRONT_H
#define TPZFRONT_H

#include <pzmatrix.h>
#include <pzstack.h>
#include <pzvec.h>

class TPZEqnArray;
/** 
 * The Front matrix itself. \n
 * It is controled by TPZFrontMatrix.
 * @ingroup frontal matrix
 */
class TPZFront {

public:

    /** Static main used for testing */
#ifndef WIN32

	static void main();

#endif
	int NElements();
    /** Simple destructor */
    virtual ~TPZFront();
    /** Simple constructor */
    TPZFront();
    /** Constructor with a initial size parameter */
	TPZFront(
	     int GlobalSize //! Initial size of the Frontal Matrix
	     );

    /**
     * Decompose these equations in a symbolic way and store freed indexes in fFree 
     */
    void SymbolicDecomposeEquations(
          int mineq //!Initial equation
          , int maxeq //!Final equation
          );
	
	/** Add a contribution of a stiffness matrix using the indexes to compute the frontwidth */
	void SymbolicAddKel(
	     TPZVec < int > & destinationindex //!Destination index of each element added
	     );
     /**
          Resizes fData element of Front
          @param newsize New size of fData
          */
	int Work(){
	return fWork;
	}

protected:
	int fWork;
private:    
	
    /**
     * Sets the global equation as freed, allowing the space \n
     * used by this equation to be used \n
     * by future assembly processes. 
     */
    void FreeGlobal(
          int global //!Equation number to be freed
          );
    /**
     * return a local index corresponding to a global equation number 
     */
    int Local(
          int global //!Global equation index which has a local indexation
          );
public:
	/** Extracts the so far condensed matrix */
virtual	void ExtractFrontMatrix(TPZFMatrix &front) {
	std::cout << "TPZFront ExtractFrontMatrix should never be called\n";
}
    /** Returns the number of free equations */
	int NFree();
    /** Resets data structure */
	void Reset(int GlobalSize=0);

    /**
     * It prints TPZFront data 
     */
     void Print(const char *name, std::ostream& out);
     void PrintGlobal(const char *name, std::ostream& out = std::cout);




protected:
    
    /**
     * Maximum size of the front
     */
    int fMaxFront;
    

    /**
     * Global equation associated to each front equation \n
	* If we need a position in globalmatrix of a equation "i" in the frontmatrix \n
	* then we can use fGlobal[i]. If the global equation "i" is not used \n
	* then fGlobal[i]==-1.
     */
    TPZManVector <int> fGlobal;

    /**
     * Front equation to each global equation \n
     * If we need a position in frontmatrix of a global equation "i" \n
	* then we can use fLocal[i]. If the global equation is not represented in the front \n
	* then fLocal[i]==-1.
     */
    TPZVec<int> fLocal;

    /**
     * Actual front size 
     */
    int fFront;

    /**
     * Colection of already decomposed equations still on the front 
     */
    TPZStack <int> fFree;

    /**
     * Frontal matrix data 
     */
    TPZVec<REAL> fData;
    
    /**
     * Expantion Ratio of frontal matrix
     */
    int fExpandRatio;
};
#endif //TPZFRONT_H
