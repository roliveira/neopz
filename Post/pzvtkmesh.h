/*
 *  pzvtkmesh.h
 *  NeoPZ
 *
 *  Created by Philippe Devloo on 04/12/08.
 *  Copyright 2008 UNICAMP. All rights reserved.
 *
 */
#ifndef PZVTKMESH
#define PZVTKMESH


#include "pzgraphmesh.h"
#include "pzvec.h"


class TPZBlock;

class TPZVTKGraphMesh : public TPZGraphMesh {
	
public:
	
	TPZVTKGraphMesh(TPZCompMesh *cmesh, int dimension, TPZAutoPointer<TPZMaterial> mat,
					const TPZVec<std::string> &scalnames, const TPZVec<std::string> &vecnames);
	TPZVTKGraphMesh(TPZCompMesh *cmesh,int dim,TPZVTKGraphMesh *graph,TPZAutoPointer<TPZMaterial> mat);
	
	virtual void DrawMesh(int numcases);
	
	virtual void DrawNodes();
	virtual void DrawConnectivity();
	virtual void DrawSolution(int step, REAL time);
	virtual void DrawSolution(TPZBlock &Sol);
	virtual void DrawSolution(char *var = 0);
	
protected:
	virtual void SequenceNodes();
	int fNumCases;
	int fNumSteps;
	
};

#endif
