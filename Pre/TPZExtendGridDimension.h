/**
 * @file
 * @brief Contains the TPZExtendGridDimension class which generates a three dimensional mesh as an extension of a two dimensional mesh.
 */
/**
 * This classroom extends a dimension mesh n = 1,2 
 * for a dimension mesh n+1 = 2,3 respectively, 
 * the entrance data are the fine mesh and the thickness of the new mesh
 * restrictions:
 * - for now the mesh it must be plain
 * - the nodes of the mesh must be given with three co-ordinated
 */

#ifndef TPZEXTENDGRID_HPP
#define TPZEXTENDGRID_HPP

class TPZGeoMesh;
#include "pzstack.h"
#include <fstream>
#include <iostream>

/**
 * @ingroup pre
 * @brief Generates a three dimensional mesh as an extension of a two dimensional mesh. \ref pre "Getting Data"
 */
class TPZExtendGridDimension {
	
	/**
	 * @brief thickness of the mesh (+  or -)
	 */
	REAL fThickness;
	
	/**
	 * @brief name of the fine mesh to be extended
	 */
	std::ifstream fFineFileMesh;
	
	/**
	 * @brief fine geometric mesh generated by the NeoPZ
	 */
	TPZGeoMesh *fFineGeoMesh;
	
public:
	
	TPZExtendGridDimension(char *geofile,REAL thickness);
	TPZExtendGridDimension(TPZGeoMesh *finegeomesh,REAL thickness);
	
	~TPZExtendGridDimension(){};
	
	/*
	 * @brief It reads the mesh since the archive of entrance finemesh,
	 * passed in the constructor, and returns extended mesh
	 */
	TPZGeoMesh *ExtendedMesh();
	
	/*
	 * @brief Prints the generated mesh
	 */
	void PrintGeneratedMesh(std::ostream &out = std::cout);
	
};

#endif
