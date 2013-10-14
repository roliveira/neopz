/**
 * @file
 * @brief Contains the TPZGraphElQ2d class which implements the graphical two dimensional element.
 */
#ifndef PZGRAPHELQ2D
#define PZGRAPHELQ2D

#include "pzgraphel.h"
#include "pzvec.h"

/**
 * @ingroup post
 * @brief To export a graphical two dimensional element. \ref post "Post processing"
 */
class TPZGraphElQ2d : public TPZGraphEl {
public:
	/** @brief Constructor for graphical element to computational quadrilateral element */
	TPZGraphElQ2d(TPZCompEl *cel, TPZGraphMesh *gmesh);
	
	virtual ~TPZGraphElQ2d(void);
	
	virtual int NConnects(){ return 9;}
	
	virtual MElementType Type() { return EQuadrilateral;}
	
	virtual int ExportType(TPZDrawStyle st);
	
	virtual int NNodes();
	
	virtual TPZGraphNode *Connect(long i);
	
	virtual int NPoints(TPZGraphNode *n);
	
	virtual int NElements();
	
	virtual void SetNode(long i,TPZGraphNode *n);
	
	virtual long EqNum(TPZVec<int> &co);
	
	virtual void Connectivity(TPZDrawStyle st = EDXStyle);
	
	
protected:
	
	virtual void FirstIJ(int connect, TPZVec<int> &co, int &incr);
	
	virtual void NextIJ(int connect,TPZVec<int> &co, int incr);
	
	protected :   
	
	/** @brief Graphical nodes vector (by connect of the computational element) */
	TPZGraphNode *fConnects[9];	
	
};

#endif
