//
//  CircularCMesh.hpp
//  PZ
//
//  Created by Nathalia Batalha on 9/8/17.
//
//

#ifndef CircularCMesh_hpp
#define CircularCMesh_hpp

#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "pzvec.h"
#include "pzadmchunk.h"
#include "pzcmesh.h"
#include "pzvec_extras.h"
#include "pzdebug.h"
#include "pzcheckgeom.h"

#include "pzgeoel.h"
#include "pzgnode.h"
#include "pzgeoelside.h"
#include "pzgeoelbc.h"

#include "pzintel.h"
#include "pzcompel.h"

#include "pzmatrix.h"

#include "pzanalysis.h"
#include "pzfstrmatrix.h"
#include "pzskylstrmatrix.h"
#include "pzstepsolver.h"

#include "pzmaterial.h"
#include "pzbndcond.h"
#include "pzelasmat.h"
#include "pzplaca.h"
#include "pzmat2dlin.h"
#include "pzmathyperelastic.h"
#include "pzmattest3d.h"
#include "pzmatplaca2.h"

#include "pzfunction.h"
#include "TPZVTKGeoMesh.h"

#include <time.h>
#include <stdio.h>
#include <fstream>

#include "pzlog.h"
#include "pzgmesh.h"
#include "TPZMatElasticity2D.h"
#include <iostream>
#include <fstream>
#include <string>
#include "TPZVTKGeoMesh.h"
#include "pzanalysis.h"
#include "pzbndcond.h"

#include "pzstepsolver.h"
#include "math.h"
#include "TPZSkylineNSymStructMatrix.h"
#include "TPZReadGIDGrid.h"
#include "TPZParFrontStructMatrix.h"

#include <cmath>
#include <set>

#include "pzelast3d.h"
#include <pzgengrid.h>

#include "pzrandomfield.h"

#define MATERIAL_ID 1
#define DIMENSION_2D 2

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.elasticity"));
#endif

TPZCompMesh *QuarterCMesh(TPZGeoMesh, int, TPZFMatrix<REAL>);

TPZCompMesh *CircularCMesh(TPZGeoMesh *gmesh, int pOrder, int projection, int inclinedwellbore,
                           int analytic, REAL SigmaV, REAL Sigmah, REAL SigmaH, REAL Pwb, REAL rw,
                           REAL rext, REAL direction, REAL inclination, bool isStochastic,
                           int nSquareElements,	TPZFMatrix<STATE> &M);

#endif /* CircularCMesh_hpp */