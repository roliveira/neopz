/**
 * \file
 * @brief Contains implementations of the TPZPlaneFracture methods.
 */
/*
 *  TPZPlaneFracture.cpp
 *  Crack
 *
 *  Created by Cesar Lucci on 09/08/10.
 *  Copyright 2010 LabMeC. All rights reserved.
 *
 */

#define DEBUGcaju

#include "TPZPlaneFracture.h"
#include "TPZGeoLinear.h"
#include "TPZGeoCube.h"
#include "pznoderep.h"
#include "pzgeoel.h"
#include "TPZVTKGeoMesh.h"
#include "TPZPoligonalChain.h"
#include "tpzgeoelrefpattern.h"
#include "pzgeoelside.h"


std::map<double,double>::iterator f_it;


TPZPlaneFracture::TPZPlaneFracture(double lw, double bulletDepthIni, double bulletDepthFin, 
                                   TPZVec< std::map<double,double> > & pos_stress)
{
    fpos_stress = pos_stress;
    fTrimQTD = __minTrimQTD;
    
    fPlaneMesh = new TPZGeoMesh;
    fFullMesh = new TPZGeoMesh;
        
    std::set<double> espacamentoVertical;
    
    espacamentoVertical.insert(0.);
    espacamentoVertical.insert(lw);
    espacamentoVertical.insert(bulletDepthIni);
    espacamentoVertical.insert(bulletDepthFin);
    
    int nstretches = pos_stress.NElements();
    for(int s = 0; s < nstretches; s++)
    {
        for(f_it = pos_stress[s].begin(); f_it != pos_stress[s].end(); f_it++)
        {
            double pos = f_it->first;
            espacamentoVertical.insert(pos);
        }
    }
    
    double pos0 = 0.;
    std::set<double>::iterator it = espacamentoVertical.begin(); it++;
    for(; it != espacamentoVertical.end(); it++)
    {
        double pos1 = *it;
        double deltaZ = pos1 - pos0;
        
        int nrows = deltaZ/__maxLength;
        deltaZ = deltaZ/nrows;
        
        for(int r = 1; r < nrows; r++)
        {
            double z = pos0 + r*deltaZ;
            espacamentoVertical.insert(z);
        }
        pos0 = pos1;
    }
    
    GeneratePlaneMesh(espacamentoVertical);
    GenerateFullMesh(espacamentoVertical);
}
//------------------------------------------------------------------------------------------------------------


TPZPlaneFracture::~TPZPlaneFracture()
{
    delete fPlaneMesh;
    delete fFullMesh;
    fpos_stress.Resize(0);
	fTrimQTD = 0;
}
//------------------------------------------------------------------------------------------------------------


void TPZPlaneFracture::GeneratePlaneMesh(std::set<double> & espacamento, double lengthFactor)
{
    TPZVec< TPZVec<REAL> > NodeCoord(0);
    int nrows, ncols;
    double Y = 0.;
    GenerateNodesAtPlaneY(espacamento, lengthFactor, NodeCoord, nrows, ncols, Y);

    int nNodesByLayer = nrows*ncols;
	
	//initializing gmesh->NodeVec()
	fPlaneMesh->NodeVec().Resize(nNodesByLayer);
	TPZVec <TPZGeoNode> Node(nNodesByLayer);
	for(int n = 0; n < nNodesByLayer; n++)
	{
		Node[n].SetNodeId(n);
		Node[n].SetCoord(NodeCoord[n]);
		fPlaneMesh->NodeVec()[n] = Node[n]; 
	}
	
	//inserting quadrilaterals
	int elId = 0;
	TPZVec <int> Topol(4);
	for(int r = 0; r < (nrows-1); r++)
	{
		for(int c = 0; c < (ncols-1); c++)
		{
			Topol[0] = ncols*r+c; Topol[1] = ncols*(r+1)+c; Topol[2] = ncols*(r+1)+c+1; Topol[3] = ncols*r+c+1;
			new TPZGeoElRefPattern< pzgeom::TPZGeoQuad > (elId,Topol,__2DfractureMat_outside,*fPlaneMesh);
			elId++;
		}
	}
	
	fPlaneMesh->BuildConnectivity();
}
//------------------------------------------------------------------------------------------------------------

void TPZPlaneFracture::GenerateFullMesh(std::set<double> & espacamento, double lengthFactor)
{
    TPZVec< TPZVec<REAL> > NodeCoord(0);
    int nrows, ncols;
    
    std::set<double>::iterator it = espacamento.end(); it--;
    double lastPos = *it;
    
lastPos = 4.;//AQUICAJU
    int nDirRef = int(log(lastPos/__maxLength)/log(2.));
    int nLayers = nDirRef + 2;
    
    double Y = 0.;
    GenerateNodesAtPlaneY(espacamento, lengthFactor, NodeCoord, nrows, ncols, Y);
    
    Y = lastPos/pow(2.,nDirRef);
    for(int lay = 1; lay < nLayers; lay++)
    {
        GenerateNodesAtPlaneY(espacamento, lengthFactor, NodeCoord, nrows, ncols, Y);
        Y *= 2.;
    }
    int nNodesByLayer = nrows*ncols;
    int Qnodes = nNodesByLayer * nLayers;
	
	//initializing gmesh->NodeVec()
	fFullMesh->NodeVec().Resize(Qnodes);
    
    int pos = 0;
	TPZGeoNode Node;
    for(int l = 0; l < nLayers; l++)
    {
        for(int n = 0; n < nNodesByLayer; n++)
        {
            Node.SetNodeId(pos);
            Node.SetCoord(NodeCoord[pos]);
            fFullMesh->NodeVec()[pos] = Node;
            pos++;
        }
    }
	
	//inserting quadrilaterals
	int elId = 0;
	TPZVec <int> Topol(4);
	for(int r = 0; r < (nrows-1); r++)
	{
		for(int c = 0; c < (ncols-1); c++)
		{
			Topol[0] = ncols*r+c;  Topol[1] = ncols*(r+1)+c; Topol[2] = ncols*(r+1)+c+1; Topol[3] = ncols*r+c+1;
			new TPZGeoElRefPattern< pzgeom::TPZGeoQuad > (elId,Topol,__2DfractureMat_outside,*fFullMesh);
			elId++;
		}
	}
    
    //inserting hexaedrons
	Topol.Resize(8);
    for(int l = 0; l < (nLayers-1); l++)
    {
        for(int r = 0; r < (nrows-1); r++)
        {
            for(int c = 0; c < (ncols-1); c++)
            {
                Topol[0] = ncols*r+c + l*nNodesByLayer;
                Topol[1] = ncols*(r+1)+c + l*nNodesByLayer;
                Topol[2] = ncols*(r+1)+c+1 + l*nNodesByLayer;
                Topol[3] = ncols*r+c+1 + l*nNodesByLayer;
                //
                Topol[4] = ncols*r+c + (l+1)*nNodesByLayer;
                Topol[5] = ncols*(r+1)+c + (l+1)*nNodesByLayer;
                Topol[6] = ncols*(r+1)+c+1 + (l+1)*nNodesByLayer;
                Topol[7] = ncols*r+c+1 + (l+1)*nNodesByLayer;
                
                new TPZGeoElRefPattern< pzgeom::TPZGeoCube > (elId,Topol,__3DrockMat,*fFullMesh);
                elId++;
            }
        }
    }
    
    
	fFullMesh->BuildConnectivity();
}
//------------------------------------------------------------------------------------------------------------

void TPZPlaneFracture::GenerateNodesAtPlaneY(std::set<double> & espacamento, double lengthFactor, 
                                             TPZVec< TPZVec<REAL> > & NodeCoord, int & nrows, int & ncols,
                                             double Y)
{
    nrows = espacamento.size();
    double deltaX = __maxLength;//espacamento entre colunas
    
    std::set<double>::iterator it = espacamento.end(); it--;
    double lastPos = *it;
    
    ncols = (lengthFactor * lastPos) / deltaX;
    
	const int nNodesByLayer = nrows*ncols;
    int oldNNodes = NodeCoord.NElements();
    int newNNodes = oldNNodes + nNodesByLayer;
    
	NodeCoord.Resize(newNNodes);
	
	int nodeId = oldNNodes;
	
	//setting nodes coords
	for(it = espacamento.begin(); it != espacamento.end(); it++)
	{
		for(int c = 0; c < ncols; c++)
		{
            NodeCoord[nodeId].Resize(3,0.);
			NodeCoord[nodeId][0] = c*deltaX;
            NodeCoord[nodeId][1] = Y;
			NodeCoord[nodeId][2] = *it;
			nodeId++;
		}
	}
}
//------------------------------------------------------------------------------------------------------------

TPZGeoMesh * TPZPlaneFracture::GetFractureMesh(const TPZVec<REAL> &poligonalChain)
{
	#ifdef DEBUG
	int ncoord = poligonalChain.NElements();
	if(ncoord%2 != 0)
	{
		std::cout << "poligonalChain boundary dont have groups of 2 coordinates (x,z)!" << std::endl;
		std::cout << "See " << __PRETTY_FUNCTION__ << std::endl;
		DebugStop();
	}
	#endif
	
    TPZGeoMesh * fullMesh = new TPZGeoMesh(*fFullMesh);
	TPZGeoMesh * planeMesh = new TPZGeoMesh(*fPlaneMesh);
	int nelem = planeMesh->NElements();
    
	std::map< int, std::set<double> > elId_TrimCoords;
	std::list< std::pair<int,double> > elIdSequence;
	
	DetectEdgesCrossed(poligonalChain, planeMesh, elId_TrimCoords, elIdSequence);
	
	//Refining 1D elements
	TPZVec<TPZGeoEl*> sons;
	std::map< int, std::set<double> >::iterator it;
	for(it = elId_TrimCoords.begin(); it != elId_TrimCoords.end(); it++)
	{
		int el1DId = it->first;
        TPZGeoEl * el1D = planeMesh->ElementVec()[el1DId];
        
		TPZAutoPointer<TPZRefPattern> linRefp = Generate1DRefPatt(it->second);

		el1D->SetRefPattern(linRefp);
		el1D->Divide(sons);
	}

	//Refining 2D and 3D elements
	for(int el = 0; el < nelem; el++)
	{
		TPZGeoEl * gel = planeMesh->ElementVec()[el];//2D element in 2D mesh

		TPZAutoPointer<TPZRefPattern> elRefp = TPZRefPatternTools::PerfectMatchRefPattern(gel);
		if(elRefp)
		{
            gel = fullMesh->ElementVec()[el];//2D element in 3D mesh
            if(gel)
            {
                gel->SetRefPattern(elRefp);
                gel->Divide(sons);
                
                int innerSide = gel->NSides() - 1;
                gel = gel->Neighbour(innerSide).Element();//3D element in 3D mesh

                elRefp = TPZRefPatternTools::PerfectMatchRefPattern(gel);
                if(elRefp)
                {
                    gel->SetRefPattern(elRefp);
                    gel->Divide(sons);
                }
            }
            else
            {
                DebugStop();
            }
		}
	}
	
    TPZVec<int> crackBoundaryElementsIds(0);
	GenerateCrackBoundary(planeMesh, fullMesh, elIdSequence, crackBoundaryElementsIds);    
    ChangeMaterialsOfFractureInterior(fullMesh, crackBoundaryElementsIds);
    
    delete planeMesh;
	
	return fullMesh;
}
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
#include "TPZVTKGeoMesh.h"

void TPZPlaneFracture::DetectEdgesCrossed(const TPZVec<REAL> &poligonalChain, TPZGeoMesh * planeMesh,
                                          std::map< int, std::set<double> > &elId_TrimCoords, 
                                          std::list< std::pair<int,double> > &elIdSequence)
{
	int npoints = (poligonalChain.NElements())/2;
	int nelem = planeMesh->NElements();
	
    TPZGeoEl * firstGel = PointElement(0, planeMesh, poligonalChain);
	TPZGeoEl * gel = firstGel;
	TPZGeoEl * nextGel = NULL;
	
    #ifdef DEBUG
	if(!gel)
	{
		std::cout << "first point of crack tip boundary does NOT belong to any 2D element" << std::endl;
		std::cout << "(or was given a mesh with zero quantity of 2D elements)!" << std::endl;
		std::cout << "See " << __PRETTY_FUNCTION__ << std::endl;
		DebugStop();
	}
    #endif
	
	double alphaMin;
	bool reachNextPoint;
	int nElsCrossed, thispoint, nextpoint;
	std::map< int, std::set<double> >::iterator it;
	std::set<double> trim;
	TPZVec< TPZVec<REAL> > intersectionPoint;
	TPZVec<REAL> x(3), dx(3);
	TPZVec<REAL> xNext(3), qsi2D(2);
	TPZVec <int> Topol(2), edgeVec;
	
	int p;
	for(p = 0; p < (npoints-1); p++)
	{
		nElsCrossed = 0;
		alphaMin = 0.;
		thispoint = 2*p;
		nextpoint = 2*(p+1);
        
        //-----------------------
        double xcoord = poligonalChain[thispoint+0];
        x[0] = xcoord;
        
        double dxcoord = poligonalChain[nextpoint+0] - poligonalChain[thispoint+0];
        dx[0] = dxcoord;
        
        double xnextcoord = poligonalChain[nextpoint+0];
        xNext[0] = xnextcoord;
        //-----------------------
        x[1] = 0.;
        dx[1] = 0.;
        xNext[1] = 0.;
        //-----------------------
        xcoord = poligonalChain[thispoint+1];
        x[2] = xcoord;
        
        dxcoord = poligonalChain[nextpoint+1] - poligonalChain[thispoint+1];
        dx[2] = dxcoord;
        
        xnextcoord = poligonalChain[nextpoint+1];
        xNext[2] = xnextcoord;
        //-----------------------
    
		double norm = 0.;
		for(int c = 0; c < 3; c++)
		{
			norm += dx[c]*dx[c];
		}
		norm = sqrt(norm);		
		for(int c = 0; c < 3; c++)
		{
			dx[c] = dx[c]/norm;
		}
		
		reachNextPoint = gel->ComputeXInverse(xNext, qsi2D);
		while(reachNextPoint == false && nElsCrossed < nelem)
		{
			nextGel = CrossToNextNeighbour(gel, x, dx, alphaMin, elId_TrimCoords, elIdSequence, true);
			
			alphaMin = __smallNum;//qdo vai de vizinho a vizinho ateh chegar no proximo ponto, nao deve-se incluir os (alphaX=0)
			if(nextGel->ComputeXInverse(xNext, qsi2D))
			{
				reachNextPoint = true;
			}
			gel = nextGel;
			nElsCrossed++;
		}
		if(nElsCrossed == nelem)
		{
			//Deve ter alternado entre vizinhos!!!
			DebugStop();
		}
	}
    
    TPZGeoEl * lastGel = gel;
	
	dx[0] = -1.;//direcao oposta ao eixo x do sistema de coordenadas
    dx[1] =  0.;//direcao oposta ao eixo x do sistema de coordenadas
    dx[2] =  0.;//direcao oposta ao eixo x do sistema de coordenadas
	
	p = 0;//Fechando o inicio da fratura
	thispoint = 2*p;
	nextpoint = 2*(p+1);

    x[0] = poligonalChain[thispoint+0];
    x[1] = 0.;
    x[2] = poligonalChain[thispoint+1];

	nextGel = firstGel;
	gel = NULL;
	alphaMin = 0.;
	while(gel != nextGel)
	{
		gel = nextGel;
		nextGel = CrossToNextNeighbour(gel, x, dx, alphaMin, elId_TrimCoords, elIdSequence, false);
		alphaMin = __smallNum;
	}
	
	p = npoints;//Fechando o final da fratura
	thispoint = 2*(p-1);

    x[0] = poligonalChain[thispoint+0];
    x[1] = 0.;
    x[2] = poligonalChain[thispoint+1];
    
	nextGel = lastGel;
	gel = NULL;
	alphaMin = 0.;
	while(gel != nextGel)
	{
		gel = nextGel;
		nextGel = CrossToNextNeighbour(gel, x, dx, alphaMin, elId_TrimCoords, elIdSequence, true);
		alphaMin = __smallNum;
	}
}
//------------------------------------------------------------------------------------------------------------


TPZGeoEl * TPZPlaneFracture::CrossToNextNeighbour(TPZGeoEl * gel, TPZVec<REAL> &x, TPZVec<REAL> dx, double alphaMin,
												  std::map< int, std::set<double> > &elId_TrimCoords, std::list< std::pair<int,double> > &elIdSequence, 
                                                  bool pushback)
{
	bool thereIsAn1DElemAlready;
	int edge;
	std::map< int, std::set<double> >::iterator it;
	std::set<double> trim;
	TPZVec< TPZVec<REAL> > ExactIntersectionPoint, ModulatedIntersectionPoint;
	TPZVec<REAL> qsi1Dvec(1), xCrackBoundary(3);
	TPZVec<int> Topol(2), edgeVec;
	
	TPZGeoMesh * planeMesh = gel->Mesh();
	
	bool haveIntersection = EdgeIntersection(gel, x, dx, edgeVec, ExactIntersectionPoint, ModulatedIntersectionPoint, alphaMin);
	
	if(haveIntersection == false)
	{
		haveIntersection = EdgeIntersection(gel, x, dx, edgeVec, ExactIntersectionPoint, ModulatedIntersectionPoint, 0.);
        
        #ifdef DEBUG
		if(haveIntersection == false)
		{
            TPZVec<REAL> qsi2D(2);
            if(gel->ComputeXInverse(x, qsi2D))
            {
                std::cout << "The point inside element does NOT intersect its edges! EdgeIntersection method face an exeption!" << std::endl;
                std::cout << "See " << __PRETTY_FUNCTION__ << std::endl;
                
                std::cout << "Element " << gel->Id() << std::endl;
                for(int n = 0; n < gel->NNodes(); n++)
                {
                    std::cout << "Node " << n << std::endl;
                    gel->NodePtr(n)->Print(std::cout);
                    std::cout << std::endl;
                }
                std::cout << "x: " << x[0] << " , " << x[1] << " , " << x[2] << std::endl;
                std::cout << "dx: " << dx[0] << " , " << dx[1] << " , " << dx[2] << std::endl;
                std::cout << "alphaMin = " << alphaMin << std::endl << std::endl;
                
                DebugStop();
            }
            else
            {
                std::cout << "Trying to find edge intersection from an point that doesnt belong to given element!" << std::endl;
                std::cout << "See " << __PRETTY_FUNCTION__ << std::endl;

                DebugStop();
            }
		}
        #endif
	}
	TPZVec<REAL> xLin(3);
	for(int edg = 0; edg < edgeVec.NElements(); edg++)
	{
		edge = edgeVec[edg];
		TPZVec<REAL> n0(3), n1(3);
		planeMesh->NodeVec()[gel->SideNodeIndex(edge, 0)].GetCoordinates(n0);
		planeMesh->NodeVec()[gel->SideNodeIndex(edge, 1)].GetCoordinates(n1);
		for(int c = 0; c < 3; c++)
		{
            double coordM = ModulatedIntersectionPoint[edg][c];
			xLin[c] = coordM;
		}
		double qsi1D = LinearComputeXInverse(xLin, n0, n1);
		
		TPZGeoElSide gelEdge(gel, edge);
		TPZGeoElSide neighEdge = gelEdge.Neighbour();
		thereIsAn1DElemAlready = false;
		while(neighEdge != gelEdge)
		{
			if(neighEdge.Element()->Dimension() == 1)//jah existe um elemento 1D inserido nesta aresta!
			{
				thereIsAn1DElemAlready = true;
				int neighEdgeId = neighEdge.Element()->Id();
				it = elId_TrimCoords.find(neighEdgeId);
				TPZTransform transBetweenNeigh = neighEdge.NeighbourSideTransform(gelEdge);
				qsi1D *= transBetweenNeigh.Mult()(0,0);
				it->second.insert(qsi1D);
				if(pushback)
				{
					elIdSequence.push_back(std::make_pair(neighEdgeId, qsi1D));
				}
				else // push_FRONT
				{
					elIdSequence.push_front(std::make_pair(neighEdgeId, qsi1D));
				}

				break;
			}
			neighEdge = neighEdge.Neighbour();
		}
		if(thereIsAn1DElemAlready == false)//nao existe um elemento 1D nesta aresta!
		{
			trim.clear();
			trim.insert(qsi1D);
			Topol[0] = gel->SideNodeIndex(edge, 0);
			Topol[1] = gel->SideNodeIndex(edge, 1);
			TPZGeoElRefPattern< pzgeom::TPZGeoLinear > * linGeo = 
                new TPZGeoElRefPattern< pzgeom::TPZGeoLinear >(Topol, __aux1DEl_Mat, *planeMesh);
            
			planeMesh->BuildConnectivity();
			
			int linGeoId = linGeo->Id();
			elId_TrimCoords[linGeoId] = trim;
			
			if(pushback) // push_BACK
			{
				elIdSequence.push_back(std::make_pair(linGeoId, qsi1D));
			}
			else // push_FRONT
			{
				elIdSequence.push_front(std::make_pair(linGeoId, qsi1D));
			}
		}
	}
	x = ExactIntersectionPoint[ExactIntersectionPoint.NElements() - 1];
	TPZGeoElSide gelEdge(gel, edge);
	TPZGeoElSide neighEdge = gelEdge.Neighbour();
	while(neighEdge != gelEdge)
	{
		if(neighEdge.Element()->Dimension() == 2)
		{
			break;
		}
		neighEdge = neighEdge.Neighbour();
	}
	gel = neighEdge.Element();
	
	return gel;
}
//------------------------------------------------------------------------------------------------------------


bool TPZPlaneFracture::EdgeIntersection(TPZGeoEl * gel, TPZVec<REAL> &x, TPZVec<REAL> &dx, TPZVec<int> &edge,
										TPZVec< TPZVec<REAL> > &ExactIntersect, TPZVec< TPZVec<REAL> > &ModulatedIntersect, double alphaMin)
{
	int nearNode;
	bool IsNearNode = TPZPlaneFracture::NearestNode(gel, x, nearNode, __smallNum);
	
	edge.Resize(0);
	ExactIntersect.Resize(0);
	ModulatedIntersect.Resize(0);
	
	int ncnodes = gel->NCornerNodes();
	
	TPZVec< TPZVec<REAL> > node(ncnodes), dnode(ncnodes);
	TPZVec<REAL> nodeCoord(3);
	for(int n = 0; n < ncnodes; n++)
	{
		node[n].Resize(3,1);
		dnode[n].Resize(3,1);
		gel->NodePtr(n)->GetCoordinates(nodeCoord);
		for(int c = 0; c < 3; c++)
		{
			(node[n])[c] = nodeCoord[c];
		}
	}
	std::list<double> edgeNorm;
	/** <double: alpha , <double: alphaNodemod, double: alphaNodesmooth, double: norm, int: edge that intersect> > */
    std::map<double, TPZVec<REAL> > alpha;
    
	double alphaX, alphaNodemod, alphaNodesmooth, norm;
	for(int n = 0; n < ncnodes; n++)
	{	
		norm = 0.;
		for(int c = 0; c < 3; c++)//computing the direction from node N to node N+1
		{
			if(n != (ncnodes-1) )
			{
				(dnode[n])[c] = node[n+1][c] - node[n][c];
			}
			else
			{
				dnode[n][c] = node[0][c] - node[n][c];
			}
			norm += dnode[n][c]*dnode[n][c];
		}
		norm = sqrt(norm);
		for(int c = 0; c < 3; c++)//normalizing computed direction
		{
			dnode[n][c] /= norm;
		}
		
		alphaX = ComputeAlphaX(x, dx, node[n], dnode[n]);
		alphaNodemod = ComputeAlphaNode(x, dx, node[n], dnode[n], norm, true, false);
		alphaNodesmooth = ComputeAlphaNode(x, dx, node[n], dnode[n], norm, IsNearNode, true);
		if(alphaX >= alphaMin && alphaNodesmooth >= 0. && alphaNodesmooth <= norm)
		{
			int thisEdge = n+ncnodes;
			TPZVec<REAL> someData(4);
			someData[0] = alphaNodemod; someData[1] = alphaNodesmooth; someData[2] = norm; someData[3] = thisEdge;
			alpha[alphaX] = someData;
		}
	}
	
	// a aresta que serah interseccionada serah a que tiver menor alphaX não negativo, i.e.: o primeiro par do mapa alpha!
	std::map<double, TPZVec<REAL> >::iterator it = alpha.begin();
	if(it != alpha.end())
	{		
		alphaX = it->first;
		alphaNodemod = (it->second)[0];
		alphaNodesmooth = (it->second)[1];
		norm = (it->second)[2];
		edge.Resize(1); edge[0] = int((it->second)[3]);	
		
		ModulatedIntersect.Resize(1); ModulatedIntersect[0].Resize(3,1);
		for(int c = 0; c < 3; c++)
		{
			ModulatedIntersect[0][c] = node[edge[0]-ncnodes][c] + alphaNodemod*dnode[edge[0]-ncnodes][c];
		}
		ExactIntersect.Resize(1); ExactIntersect[0].Resize(3,1);
		for(int c = 0; c < 3; c++)
		{
			ExactIntersect[0][c] = node[edge[0]-ncnodes][c] + alphaNodesmooth*dnode[edge[0]-ncnodes][c];
		}
		
		if(alphaX <= alphaMin && alpha.size() > 1)
		{
			it++;
			alphaX = it->first;
			alphaNodemod = (it->second)[0];
			alphaNodesmooth = (it->second)[1];
			norm = (it->second)[2];
			edge.Resize(2); edge[1] = int((it->second)[3]);	
			
			ModulatedIntersect.Resize(2); ModulatedIntersect[1].Resize(3,1);
			for(int c = 0; c < 3; c++)
			{
				ModulatedIntersect[1][c] = node[edge[1]-ncnodes][c] + alphaNodemod*dnode[edge[1]-ncnodes][c];
			}
			ExactIntersect.Resize(2); ExactIntersect[1].Resize(3,1);
			for(int c = 0; c < 3; c++)
			{
				ExactIntersect[1][c] = node[edge[1]-ncnodes][c] + alphaNodesmooth*dnode[edge[1]-ncnodes][c];
			}
		}
		return true;
	}
	
	//se o ponto p estah sobre um noh e a direcao dp aponta para fora do elemento...
	if(IsNearNode)
	{
		std::cout << "Estah no noh " << nearNode << " e nao foi encontrada interseccao!" << std::endl;
		std::cout << "Tratar este caso!" << std::endl << std::endl;
	}
	
	return false;
}
//------------------------------------------------------------------------------------------------------------


TPZGeoEl * TPZPlaneFracture::PointElement(int p, TPZGeoMesh * planeMesh, const TPZVec<REAL> &poligonalChain)
{
	TPZVec<REAL> x(3), qsi2D(2);
	
    x[0] = poligonalChain[2*p+0];
    x[1] = 0.;
    x[2] = poligonalChain[2*p+1];
    
	TPZGeoEl * gel = NULL;
	int nelem = planeMesh->NElements();
	for(int el = 0; el < nelem; el++)//hunting the first element (that contains the first point)
	{
		TPZGeoEl * firstGel = planeMesh->ElementVec()[el];
		if(firstGel->Dimension() != 2)
		{
			continue;
		}
		if(firstGel->ComputeXInverse(x, qsi2D))
		{
			gel = firstGel;
			break;
		}
	}
	
	return gel;
}
//------------------------------------------------------------------------------------------------------------


double TPZPlaneFracture::ComputeAlphaNode(TPZVec<REAL> &x, TPZVec<REAL> &dx, TPZVec<REAL> &node, TPZVec<REAL> &dnode, double norm, bool modulate, bool smooth)
{
	double fractionNumQ =	dx[2]*node[0] - dx[0]*node[2] - dx[2]*x[0] + dx[0]*x[2];
	
	double fractionDenomQ =	dnode[2]*dx[0] - dnode[0]*dx[2];
	
	double alphaNode = -1.;
	if(fabs(fractionDenomQ) > __smallNum)
	{
		alphaNode = fractionNumQ/fractionDenomQ;
		if(fabs(alphaNode) < __smallNum) alphaNode = 0.;
		if(fabs(alphaNode - norm) < __smallNum) alphaNode = norm;
	}
	
	if(modulate)
	{
		int TrimQTD = fTrimQTD;
		if(smooth)
		{
			TrimQTD *= __TrimQTDmultiplier;
		}
		int nsegm = int(alphaNode/(norm/TrimQTD) + 0.5);
		if(nsegm == 0)//Tirando a interseccao do noh inicial da aresta
		{
			nsegm = 1;
		}
		else if(nsegm == TrimQTD)//Tirando a interseccao do noh final da aresta
		{
			nsegm = TrimQTD - 1;
		}
		alphaNode = nsegm*(norm/TrimQTD);//modulando o ponto para multiplo de (norm/fTrimQTD)
	}
	
	return alphaNode;
}
//------------------------------------------------------------------------------------------------------------


double TPZPlaneFracture::ComputeAlphaX(TPZVec<REAL> &x, TPZVec<REAL> &dx, TPZVec<REAL> &node, TPZVec<REAL> &dnode)
{	
	//computing alpha (dx vector multiplier to intersect element edges)
	double fractionNumP   =	dnode[2]*node[0] - dnode[0]*node[2] -
	dnode[2]*x[0] + dnode[0]*x[2];
	
	double fractionDenomP =	dnode[2]*dx[0] - dnode[0]*dx[2];
	
	double alphaX = -1.;
	if(fabs(fractionDenomP) > __smallNum)
	{
		alphaX = fractionNumP/fractionDenomP;
		if(fabs(alphaX) < __smallNum)
		{
			alphaX = 0.;
		}
	}
	
	return alphaX;
}
//------------------------------------------------------------------------------------------------------------


bool TPZPlaneFracture::NearestNode(TPZGeoEl * gel, TPZVec<REAL> &x, int &node, double tol)
{    
	node = -1;
	bool IsNear = false;
	
	TPZVec<REAL> nodeCoord(3);
	int nnodes = gel->NNodes();
	
	for(int n = 0; n < nnodes; n++)
	{
		double dist = 0.;
		gel->NodePtr(n)->GetCoordinates(nodeCoord);
		for(int c = 0; c < 3; c++)
		{
			dist += (x[c] - nodeCoord[c])*(x[c] - nodeCoord[c]);
		}
		dist = sqrt(dist);
		
		if(dist <= tol)
		{
			node = n;
			IsNear = true;
			break;
		}
	}
	
	return IsNear;
}
//------------------------------------------------------------------------------------------------------------


int TPZPlaneFracture::NearestNode(TPZGeoMesh * gmesh, TPZVec<REAL> &x, double tol)
{
	int node = -1;
	
	TPZVec<REAL> nodeCoord(3);
	int nnodes = gmesh->NNodes();
	
	for(int n = 0; n < nnodes; n++)
	{
		double dist = 0.;
		gmesh->NodeVec()[n].GetCoordinates(nodeCoord);
		for(int c = 0; c < 3; c++)
		{
			dist += (x[c] - nodeCoord[c])*(x[c] - nodeCoord[c]);
		}
		dist = sqrt(dist);
		
		if(dist <= tol)
		{
			node = n;
			break;
		}
	}
	
	if(node == -1)
	{
		std::cout << "Node not found for coordinates ( " << x[0] << " , " << x[1] << " , " << x[2] << " )" << std::endl;
		std::cout << "See " << __PRETTY_FUNCTION__ << std::endl;
		
		DebugStop();
	}
	
	return node;
}
//------------------------------------------------------------------------------------------------------------


double TPZPlaneFracture::LinearComputeXInverse(TPZVec<REAL> x, TPZVec<REAL> n0, TPZVec<REAL> n1)
{
	double dL = 0., L = 0.;
	for(int c = 0; c < 3; c++)
	{
		L += (n1[c]-n0[c])*(n1[c]-n0[c]);
		dL += (x[c]-n0[c])*(x[c]-n0[c]);
	}
	L = sqrt(L);
	dL = sqrt(dL);
	
	#ifdef DEBUG
	if(fabs(L) < __smallNum || fabs(dL) < __smallNum || fabs(L - dL) < __smallNum)
	{
		std::cout << "n0 and n1 are coincident nodes!" << std::endl;
		std::cout << "See " << __PRETTY_FUNCTION__ << std::endl;
		DebugStop();
	}
	#endif
	
	double qsi = -1. + dL/L*2.;
	
	return qsi;
}
//------------------------------------------------------------------------------------------------------------


TPZAutoPointer<TPZRefPattern> TPZPlaneFracture::Generate1DRefPatt(std::set<double> &TrimCoord)
{
	if(TrimCoord.size() == 0)
	{
		return NULL;
	}
	
	int Qnodes = TrimCoord.size() + 2;
	int Qelements = TrimCoord.size() + 2;
	int QsubElements = Qelements - 1;
	TPZVec < TPZVec <REAL> > NodeCoord(Qnodes);
	for(int i = 0; i < Qnodes; i++) NodeCoord[i].Resize(3,0.);
	
	//1. setting initial and final nodes coordinates of 1D element mesh
	NodeCoord[0][0] = -1.;
	NodeCoord[1][0] =  1.;
	//.
	
	//2. setting intermediate nodes coordinates of 1D element mesh
	int c = 2;
	std::set<double>::iterator it;
	for(it = TrimCoord.begin(); it != TrimCoord.end(); it++)
	{
		double coord = *it;
		
		(NodeCoord[c])[0] = coord;
		c++;
	}
	//.
	
	//3. initializing internal mesh of refPattern
	TPZGeoMesh internalMesh;
	internalMesh.SetMaxNodeId(Qnodes-1);
	internalMesh.SetMaxElementId(Qelements-1);
	internalMesh.NodeVec().Resize(Qnodes);
	TPZVec <TPZGeoNode> Node(Qnodes);
	for(int n = 0; n < Qnodes; n++)
	{
		Node[n].SetNodeId(n);
		Node[n].SetCoord(NodeCoord[n]);
		internalMesh.NodeVec()[n] = Node[n]; 
	}
	//.
	
	//4. inserting 1D elements on internal mesh of refPattern
	int elId = 0;
	TPZVec <int> Topol(2);
	
	//4.1 inserting father element
	Topol[0] = 0; Topol[1] = 1;
	TPZGeoElRefPattern< pzgeom::TPZGeoLinear > * father = new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elId,Topol,__aux1DEl_Mat,internalMesh);
	elId++;
	//.
	
	//4.2 inserting subelements
	//first subelement
	Topol[0] = 0; Topol[1] = 2;
	TPZGeoElRefPattern< pzgeom::TPZGeoLinear > * son1 = new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elId,Topol,__aux1DEl_Mat,internalMesh);
	son1->SetFather(father);
	son1->SetFather(father->Index());
	elId++;
	//
	
	//last subelement
	Topol[0] = TrimCoord.size() + 1; Topol[1] = 1;
	TPZGeoElRefPattern< pzgeom::TPZGeoLinear > * son2 = new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elId,Topol,__aux1DEl_Mat,internalMesh);
	son2->SetFather(father);
	son2->SetFather(father->Index());
	elId++;
	//
	
	for(int el = 2; el < QsubElements; el++)
	{
		Topol[0] = el; Topol[1] = el+1;
		TPZGeoElRefPattern< pzgeom::TPZGeoLinear > * son = new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elId,Topol,__aux1DEl_Mat,internalMesh);
		son->SetFather(father);
		son->SetFather(father->Index());
		elId++;
	}
	//.
	//.
	
	internalMesh.BuildConnectivity();
	
	TPZAutoPointer<TPZRefPattern> refPattern = new TPZRefPattern(internalMesh);
	TPZAutoPointer<TPZRefPattern> Found = gRefDBase.FindRefPattern(refPattern);
	if(!Found)
	{
		gRefDBase.InsertRefPattern(refPattern);
		refPattern->InsertPermuted();
		
		return refPattern;
	}
	else
	{
		return Found;
	}
}
//------------------------------------------------------------------------------------------------------------


void TPZPlaneFracture::UpdatePoligonalChain(TPZGeoMesh * gmesh, std::list< std::pair<int,double> > &elIdSequence,
						  TPZVec<REAL> &poligonalChainUpdated)
{
	int nptos = elIdSequence.size();
	poligonalChainUpdated.Resize(2*nptos);

	TPZVec<REAL> qsi1Dvec(1), ptoCoord(3);
	int el1Did, posX, posZ, p = 0;
	double qsi1D;
	
	std::list< std::pair<int,double> >::iterator it;
	for(it = elIdSequence.begin(); it != elIdSequence.end(); it++)
	{
		el1Did = it->first;
		TPZGeoEl * el1D = gmesh->ElementVec()[el1Did];
		
		#ifdef DEBUG
		int elDim = el1D->Dimension();
		if(elDim != 1)
		{
			std::cout << "The elIdSequence supposedly would contains ids of elements exclusively 1D!" << std::endl;
			std::cout << "See " << __PRETTY_FUNCTION__ << std::endl;
			DebugStop();
		}
		#endif
		
		qsi1D = it->second;
		qsi1Dvec[0] = qsi1D;
		
		el1D->X(qsi1Dvec, ptoCoord);
		
		posX = 2*p;
		posZ = 2*p+1;
		
		poligonalChainUpdated[posX] = ptoCoord[0];
		poligonalChainUpdated[posZ] = ptoCoord[2];
		
		p++;
	}
}
//------------------------------------------------------------------------------------------------------------

void TPZPlaneFracture::GenerateCrackBoundary(TPZGeoMesh * gmesh2D,
                                             TPZGeoMesh * gmesh3D,
                                             std::list< std::pair<int,double> > &elIdSequence,
                                             TPZVec<int> &crackBoundaryElementsIds)
{
	TPZVec<REAL> qsi0vec(1), qsi1vec(1), node0coord(3), node1coord(3);
	TPZVec<int> Topol(2);
	int el0id, el1id, n0, n1;
	double qsi0, qsi1;
	std::list< std::pair<int,double> >::iterator crackit0, crackit1, crackitEnd;
	crackitEnd = elIdSequence.end(); crackitEnd--;
	for(crackit0 = elIdSequence.begin(); crackit0 != crackitEnd; crackit0++)
	{
		crackit1 = crackit0; crackit1++;
		
		el0id = crackit0->first;
		TPZGeoEl * el0 = gmesh2D->ElementVec()[el0id];
		qsi0 = crackit0->second;
		qsi0vec[0] = qsi0;
		el0->X(qsi0vec, node0coord);
		n0 = NearestNode(gmesh3D, node0coord, __smallNum);
		Topol[0] = n0;
		
		el1id = crackit1->first;
		TPZGeoEl * el1 = gmesh2D->ElementVec()[el1id];
		qsi1 = crackit1->second;
		qsi1vec[0] = qsi1;
		el1->X(qsi1vec, node1coord);
		n1 = NearestNode(gmesh3D, node1coord, __smallNum);
		Topol[1] = n1;
		
		TPZGeoEl * crack1D = new TPZGeoElRefPattern< pzgeom::TPZGeoLinear >(Topol, __1DcrackTipMat, *gmesh3D);
        int oldSize = crackBoundaryElementsIds.NElements();
        crackBoundaryElementsIds.Resize(oldSize+1);
        crackBoundaryElementsIds[oldSize] = crack1D->Id();
	}
    
    gmesh3D->BuildConnectivity();
}
//------------------------------------------------------------------------------------------------------------

void TPZPlaneFracture::ChangeMaterialsOfFractureInterior(TPZGeoMesh * fullMesh, TPZVec<int> &crackBoundaryElementsIds)
{
    int inner1Dside = 2;
    int n1Dels = crackBoundaryElementsIds.NElements();
    std::map<int,TPZFracture2DEl> fracturedElems;
    
    //Capturando subelementos que encostam no contorno da fratura
    for(int el = 0; el < n1Dels; el++)
    {
        int id = crackBoundaryElementsIds[el];
        TPZGeoEl * gel = fullMesh->ElementVec()[id];//1D element of crach boundary
        
        #ifdef DEBUG
        if(gel->Dimension() != 1)
        {
            DebugStop();
        }
        #endif
        
        TPZVec<REAL> n0(3), n1(3);
        fullMesh->NodeVec()[gel->SideNodeIndex(inner1Dside, 0)].GetCoordinates(n0);
		fullMesh->NodeVec()[gel->SideNodeIndex(inner1Dside, 1)].GetCoordinates(n1);
        
        TPZGeoElSide side1D(gel,inner1Dside);
        TPZGeoElSide sideEl2D = side1D.Neighbour();
        while(sideEl2D != side1D)
        {
            int neighSide = sideEl2D.Side();
            TPZGeoEl * neigh = sideEl2D.Element();
            if(neigh->Dimension() == 2 && !neigh->HasSubElement())
            {
                TPZVec<REAL> neighCenterQSI(neigh->Dimension()), neighCenterX(3);
                neigh->CenterPoint(neigh->NSides()-1, neighCenterQSI);
                neigh->X(neighCenterQSI, neighCenterX);
                
                //Como o contorno da fratura foi construido no sentido antihorario no plano x,z (normal Y > 0),
                //interessam os elementos aa direita do elemento 1D. Portanto eh feito produto vetorial entre os vetores
                //frac=(n1-n0) e cg_neigh=(cg-n0). O vizinho aa direita apresentarah componente em Y positiva.
                double crossYcomp = n0[2]*n1[0] - n0[0]*n1[2] -
                                    n0[2]*neighCenterX[0] + n1[2]*neighCenterX[0] +
                                    n0[0]*neighCenterX[2] - n1[0]*neighCenterX[2];
                
                if(crossYcomp > 0.)
                {
                    neigh->SetMaterialId(__2DfractureMat_inside);
                    
                    TPZFracture2DEl fractEl(neigh);
                    fractEl.RemoveThisEdge(neighSide);
                    fracturedElems[fractEl.Id()] = fractEl;                    
                    
                    break;
                }
            }
            sideEl2D = sideEl2D.Neighbour();
        }
    }
    
    //capturanto demais elementos no interior da fratura
    std::set<int> finishedFracturedElems;
    while(fracturedElems.size() > 0)
    {
        std::map<int,TPZFracture2DEl>::iterator edgIt = fracturedElems.begin();

        TPZFracture2DEl actEl = edgIt->second;
        
        std::set<int>::iterator sideIt;
        for(sideIt = actEl.fEdge.begin(); sideIt != actEl.fEdge.end(); sideIt++)
        {
            int side = *sideIt;
            TPZGeoElSide actElEdge(actEl.fElem2D,side);
            TPZGeoElSide neighElSide = actElEdge.Neighbour();
            
            bool wellDone = false;
            while(actElEdge != neighElSide && wellDone == false)
            {
                int neighSide = neighElSide.Side();
                TPZGeoEl * neighEl = neighElSide.Element();
                if(neighEl->Dimension() == 2 && !neighEl->HasSubElement())
                {
                    int neighElId = neighEl->Id();
                    if(finishedFracturedElems.find(neighElId) != finishedFracturedElems.end())
                    {
                        wellDone = true;
                        continue;
                    }
                    std::map<int,TPZFracture2DEl>::iterator edgIt_temp = fracturedElems.find(neighElId);
                    if(edgIt_temp != fracturedElems.end())
                    {
                        TPZFracture2DEl neighFractEl = edgIt_temp->second;
                        neighFractEl.RemoveThisEdge(neighSide);
                        
                        if(neighFractEl.IsOver())
                        {
                            finishedFracturedElems.insert(neighFractEl.Id());
                            fracturedElems.erase(edgIt_temp);
                        }
                    }
                    else
                    {
                        neighEl->SetMaterialId(__2DfractureMat_inside);
                        
                        TPZFracture2DEl fractEl(neighEl);
                        fractEl.RemoveThisEdge(neighSide);
                        fracturedElems[fractEl.Id()] = fractEl;   
                    }
                    
                    wellDone = true;
                }
                neighElSide = neighElSide.Neighbour();
            }
        }
            
        finishedFracturedElems.insert(actEl.Id());
        fracturedElems.erase(edgIt);
    }
}
//------------------------------------------------------------------------------------------------------------

/** to Phil */
void TPZPlaneFracture::GetSidesCrossedByPoligonalChain(const TPZVec<REAL> &poligonalChain, std::list< std::pair<TPZGeoElSide,double> > &sidesCrossed)
{
    #ifdef DEBUG
	int ncoord = poligonalChain.NElements();
	if(ncoord%2 != 0)
	{
		std::cout << "poligonalChain boundary dont have groups of 2 coordinates (x,z)!" << std::endl;
		std::cout << "See " << __PRETTY_FUNCTION__ << std::endl;
		DebugStop();
	}
    #endif
    
    sidesCrossed.clear();
	
	TPZGeoMesh * planeMesh = new TPZGeoMesh(*fPlaneMesh);
	
	std::map< int, std::set<double> > elId_TrimCoords;
	std::list< std::pair<int,double> > elIdSequence;
	
	DetectEdgesCrossed(poligonalChain, planeMesh, elId_TrimCoords, elIdSequence);
    
    std::list< std::pair<int,double> >::iterator it;
    
    for(it = elIdSequence.begin(); it != elIdSequence.end(); it++)
    {
        int gel1D_Id = it->first;
        double qsiNeigh, qsi = it->second;
        
        TPZGeoEl * gel1D = planeMesh->ElementVec()[gel1D_Id];
        TPZGeoElSide gel1DSide(gel1D,2);
        TPZGeoElSide neighSide = gel1DSide.Neighbour();
        while(neighSide != gel1DSide)
        {
            TPZTransform tr = gel1DSide.NeighbourSideTransform(neighSide);
            qsiNeigh = tr.Mult()(0,0)*qsi;
            
            sidesCrossed.push_back(std::make_pair(neighSide, qsiNeigh));
            neighSide = neighSide.Neighbour();
        }
    }
    
    planeMesh = NULL;
    delete planeMesh;
}
//------------------------------------------------------------------------------------------------------------


