/**
 * @file
 * @brief Contains the declaration of multiphysic interface class
 * @author Agnaldo
 * @since 10/26/11.
 */

#ifndef TPZMULTIPHYSICSINTERFACEELH
#define TPZMULTIPHYSICSINTERFACEELH 

#include <iostream>

#include "pzcompel.h"


/**
 * @brief Computes the contribution over an interface between two discontinuous elements. \ref CompElement "Computational Element"
 */
class TPZMultiphysicsInterfaceElement : public TPZCompEl {

protected:

	/** @brief Element vector the left of the normal a interface */
	TPZManVector<TPZCompElSide, 10> 	fLeftElSideVec;
		
	/** @brief Element vector the right of the normal a interface */
	TPZManVector<TPZCompElSide, 10> 	fRightElSideVec;
    
    /** @brief indexes of the connects */
    TPZManVector<int,100> fConnectIndexes;
	
public:
	/** @brief Default constructor */
	TPZMultiphysicsInterfaceElement();	
	/** @brief Constructor */
	TPZMultiphysicsInterfaceElement(TPZCompMesh &mesh, TPZGeoEl *ref, int &index);
	/** @brief Default destructor */
	virtual ~TPZMultiphysicsInterfaceElement();
	
	/**
	 * @brief Compute the transform of a paramenter point in the multiphysic interface element to a parameter point in the neighbor super element
	 * @param Neighbor [in] may be this->LeftElementSide() or this->RightElementSide()
	 * @param transf [out] vector of Transforms 
	 */
	void ComputeSideTransform(TPZManVector<TPZCompElSide> &Neighbor, TPZManVector<TPZTransform> &transf);
	
	/**
	 * @brief Maps qsi coordinate at this master element to qsi coordinate at neighbor master element.
	 * @param Neighbor [in] may be this->LeftElementSide() or this->RightElementSide()
	 * @param qsi [in] is the point at this element master
	 * @param NeighIntPoint [out] is the point at neighbor element master. X[qsi] is equal to X[NeighIntPoint]
	 */
	void MapQsi(TPZManVector<TPZCompElSide> &Neighbor, TPZVec<REAL> &qsi, TPZVec<REAL> &NeighIntPoint);
    
    /**
     * Add elements to the list of left and right elements
     */
    void AddLeftRightEement(TPZCompElSide &leftel, TPZCompElSide &rightel, int index);
			
    /**
     * Compute the stiffness matrix of the interface element
     */
    void CalcStiff(TPZElementMatrix &ek, TPZElementMatrix &ef);
    
    /** @brief Initialize the structure of the stiffness matrix */
    void InitializeElementMatrix(TPZElementMatrix &ek, TPZElementMatrix &ef);
    
};

#endif