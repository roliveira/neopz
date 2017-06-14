/**
 * @file
 * @brief Afirst attempt at a multi physics hcurl/h1 formulation
 * @details Adequadte for problems with longitudinal axis symmetry
 * such as some setions of waveguides (closed waveguides).
 * it uses hcurl for tranverse componentes and h1 for longitudinal components
 * Also, it uses a lagrange multiplier (in h1) in order to use a complete
 * hcurl basis, disregarding the nedelec conditions
 * @author Francisco Orlandini
 * @since 2015
 */


#include <iostream>
#include <fstream>
#include <string>
#include "TPZMatMFHCurlH1Lag.h"
#include "pzextractval.h"
#include "TPZMatHCurlProjection.h"
#include "TPZMatWaveguideCutOffAnalysisLag.h"
#include "pzgengrid.h"
#include "pzgmesh.h"
#include "TPZVTKGeoMesh.h"
#include "pzbndcond.h"
#include "pzelchdiv.h"
#include "pzshapequad.h"
#include "pzshapetriang.h"
#include "pzlog.h"
#include "TPZTimer.h"
#include "pzanalysis.h"
#include "pzskylstrmatrix.h"
#include "pzstrmatrix.h"
#include "pzstepsolver.h"
#include "pzsbstrmatrix.h"
#include "pzl2projection.h"
#include "pzmatred.h"
#include "pzsubcmesh.h"
#include "tpzmatredstructmatrix.h"
#include "pzsbndmat.h"
#include "pzfstrmatrix.h"
#include "pzbuildmultiphysicsmesh.h"
#include "pzvisualmatrix.h"
#include "pzcondensedcompel.h"
#include <sys/types.h>
#include <sys/stat.h>



enum meshTypeE{ createRectangular=1, createTriangular, createZigZag};

STATE ur( const TPZVec<REAL> & x){
    //return ( 2.-imaginary*0.1 );
    return 1.;
}
STATE er( const TPZVec<REAL> & x){
    return 1;
}
void RunSimulation( bool isCutOff, bool filterEquations, const int meshType ,bool usingFullMtrx, bool optimizeBandwidth, int pOrder, int nDiv, REAL hDomain, REAL wDomain, REAL f0, int nSolutions, bool generatingResults);

void FilterBoundaryEquations(TPZVec<TPZCompMesh *> cmeshMF , TPZVec<long> &activeEquations , int &neq, int &neqOriginal);

TPZVec<TPZCompMesh *>CMesh(TPZGeoMesh *gmesh, int pOrder, STATE (& ur)( const TPZVec<REAL> &),STATE (& er)( const TPZVec<REAL> &), REAL f0 , bool isCutOff);

void CreateGMesh(TPZGeoMesh * &gmesh, const int meshType, const REAL hDomain, const REAL wDomain,  const int xDiv, const int zDiv);

int main(int argc, char *argv[])
{
    
    HDivPiola = 1;//use piola mapping
#ifdef LOG4CXX
    InitializePZLOG();
#endif
    
    //PARAMETROS FISICOS DO PROBLEMA
    REAL hDomain = 4 * 2.54 * 1e-3;
    REAL wDomain = 9 * 2.54 * 1e-3;
    REAL f0 = 25 * 1e+9;
    int pOrder = 1; //ordem polinomial de aproximacao
    
    
    bool isCutOff = false;
    bool filterEquations = true;
    bool usingFullMtrx = true;
    bool optimizeBandwidth = false;
	
    const int meshType = createTriangular;
    bool generatingResults = false;
	
	if(generatingResults){
		struct stat sb;
		if (stat("../resultsQuali", &sb) == 0 && S_ISDIR(sb.st_mode)){
			std::string fileName("../resultsQuali/ev");
			for (int i = 0; i < 100; i++) {
				std::string testName = fileName;
				testName.append( std::to_string(i) );
				testName.append( ".csv" );
				if (std::ifstream( testName.c_str() ) ) {
					std::remove( testName.c_str() );
				}
			}
		}
	}
    int nDiv = 2;
    int nSim = 1;
    int nSolutions = 10;
    for (int i = 0 ; i < nSim; i++) {
        std::cout<<"iteration "<<i+1<<" of "<<nSim<<std::endl;
        std::cout<<"nDiv = "<<nDiv<<std::endl;
        RunSimulation( isCutOff, filterEquations, meshType, usingFullMtrx, optimizeBandwidth, pOrder, nDiv, hDomain, wDomain, f0 , nSolutions, generatingResults);
        nDiv += 5;
    }
    
    
    
    return 0;
}

void RunSimulation( bool isCutOff, bool filterEquations, const int meshType, bool usingFullMtrx, bool optimizeBandwidth, int pOrder, int nDiv, REAL hDomain, REAL wDomain, REAL f0 , int nSolutions, bool generatingResults){
    TPZTimer timer;
    
    timer.start();
    
    int xDiv = nDiv;
    int zDiv = nDiv;
    TPZGeoMesh *gmesh = new TPZGeoMesh();
    
    CreateGMesh(gmesh, meshType, hDomain, wDomain,  xDiv, zDiv);
    
    TPZVec<TPZCompMesh *>meshVec = CMesh(gmesh, pOrder, ur , er , f0 , isCutOff); //funcao para criar a malha computacional
    TPZCompMesh *cmeshMF = meshVec[0];
    TPZMatMFHCurlH1Lag *matPointer = dynamic_cast<TPZMatMFHCurlH1Lag * >( cmeshMF->MaterialVec()[1]);
    TPZVec<TPZCompMesh *>temporalMeshVec(3);
    temporalMeshVec[ matPointer->H1Index() ] = meshVec[1 + matPointer->H1Index()];
    temporalMeshVec[ matPointer->HCurlIndex() ] = meshVec[1 + matPointer->HCurlIndex()];
    temporalMeshVec[ matPointer->LagrangeIndex() ] = meshVec[1 + matPointer->LagrangeIndex()];
    
    TPZAnalysis an(cmeshMF,optimizeBandwidth);
    //configuracoes do objeto de analise
    TPZManVector<long,1000>activeEquations;
    int neq = 0;
    int neqOriginal = 0;
    if (filterEquations) {
        FilterBoundaryEquations(meshVec, activeEquations , neq , neqOriginal);
    }
    else{
        TPZCompMesh *cmeshH1 = meshVec[1 + matPointer->H1Index()];
        TPZCompMesh *cmeshHCurl = meshVec[1 + matPointer->HCurlIndex()];
        TPZCompMesh *cmeshLag = meshVec[1 + matPointer->LagrangeIndex()];
        std::cout<<"------\t------\t-------"<<std::endl;
        std::cout<<"cmeshH1->NEquations()"<<"\t"<<cmeshH1->NEquations()<<std::endl;
        std::cout<<"cmeshHCurl->NEquations()"<<"\t"<<cmeshHCurl->NEquations()<<std::endl;
        std::cout<<"cmeshLag->NEquations()"<<"\t"<<cmeshLag->NEquations()<<std::endl;
        std::cout<<"cmeshMF->NEquations()"<<"\t"<<cmeshMF->NEquations()<<std::endl;
    }
    
    nSolutions = nSolutions > neq ? neq : nSolutions;
    TPZAutoPointer<TPZSBandStructMatrix> sbstr;
    
    TPZAutoPointer<TPZFStructMatrix> fmtrx;
    
    if(usingFullMtrx){
        fmtrx = new TPZFStructMatrix(cmeshMF);
        fmtrx->SetNumThreads(0);
        if (filterEquations) {
            fmtrx->EquationFilter().SetActiveEquations(activeEquations);
        }
        an.SetStructuralMatrix(fmtrx);
    }
    else{
        sbstr = new TPZSBandStructMatrix(cmeshMF);
        sbstr->SetNumThreads(0);
        if (filterEquations) {
            sbstr->EquationFilter().SetActiveEquations(activeEquations);
        }
        an.SetStructuralMatrix(sbstr);
    }
    
    TPZStepSolver<STATE> step;
    step.SetDirect(ECholesky); //caso simetrico
    an.SetSolver(step);
    int matId = 1;
    TPZMatMFHCurlH1Lag *matAlias = dynamic_cast<TPZMatMFHCurlH1Lag *> (cmeshMF->FindMaterial( matId ) );
    
    std::cout<<"entrando no assemble matrix A"<<std::endl;
    matAlias->SetMatrixA();
    
    an.Assemble();
    TPZMatrix<STATE> *  stiffAPtr = NULL , *stiffBPtr = NULL;
    if (usingFullMtrx) {
        stiffAPtr = new TPZFMatrix<STATE> ( *dynamic_cast< TPZFMatrix<STATE> *>( an.Solver().Matrix().operator->() ) );
        matAlias->SetMatrixB();
        std::cout<<"entrando no assemble matrix B"<<std::endl;
        an.Assemble();
        stiffBPtr = new TPZFMatrix<STATE> ( *dynamic_cast< TPZFMatrix<STATE> *>( an.Solver().Matrix().operator->() ) );
        std::cout<<"saindo do assemble"<<std::endl;
        
        
    }
    else{
        stiffAPtr = new TPZSBMatrix<STATE> ( *dynamic_cast< TPZSBMatrix<STATE> *>( an.Solver().Matrix().operator->() ) );
        matAlias->SetMatrixB();
        std::cout<<"entrando no assemble matrix B"<<std::endl;
        an.Assemble();
        stiffBPtr = new TPZSBMatrix<STATE> ( *dynamic_cast< TPZSBMatrix<STATE> *>( an.Solver().Matrix().operator->() ) );
        std::cout<<"saindo do assemble"<<std::endl;
    }
    
#ifdef PZDEBUG
    std::ofstream fileA("../stiffA.csv");
    std::ofstream fileB("../stiffB.csv");
    char number[256];
    for (int i = 0; i<stiffAPtr->Rows(); i++) {
        for(int j = 0 ; j<stiffAPtr->Rows();j++){
            sprintf(number, "%16.16Lf",(long double) TPZExtractVal::val(std::real(stiffAPtr->GetVal(i,j))) );
            fileA<<number;
            
            fileA<<" + I * ";
            
            sprintf(number, "%16.16Lf",(long double) TPZExtractVal::val(std::imag(stiffAPtr->GetVal(i,j))) );
            fileA<<number;
            if( j != stiffAPtr->Rows() - 1){
                fileA<<" , ";
            }
            
            sprintf(number, "%16.16Lf",(long double) TPZExtractVal::val(std::real(stiffBPtr->GetVal(i,j))) );
            fileB<<number;
            
            fileB<<" + I * ";
            
            sprintf(number, "%16.16Lf",(long double) TPZExtractVal::val(std::imag(stiffBPtr->GetVal(i,j))) );
            fileB<<number;
            if( j != stiffAPtr->Rows() - 1){
                fileB<<" , ";
            }
        }
        fileA<<std::endl;
        fileB<<std::endl;
    }
    fileA.close();
    fileB.close();
#endif
    TPZVec<STATE> eValues;
    TPZFMatrix< STATE > eVectors;
    std::cout<<"entrando no calculo dos autovalores"<<std::endl;
    if (usingFullMtrx) {
        TPZFMatrix<STATE> *stiffA = dynamic_cast<TPZFMatrix<STATE> *>(stiffAPtr);
        TPZFMatrix<STATE> *stiffB = dynamic_cast<TPZFMatrix<STATE> *>(stiffBPtr);
        stiffA->SolveGeneralisedEigenProblem( *stiffB, eValues , eVectors);
    }
    else{
        TPZSBMatrix<STATE> *stiffA = dynamic_cast<TPZSBMatrix<STATE> *>(stiffAPtr);
        TPZSBMatrix<STATE> *stiffB = dynamic_cast<TPZSBMatrix<STATE> *>(stiffBPtr);
        stiffA->SolveGeneralisedEigenProblem( *stiffB, eValues , eVectors);
    }
    
    std::cout<<"saindo do calculo dos autovalores"<<std::endl;
    timer.stop();
    
    std::set<std::pair<REAL,TPZFMatrix<STATE> > > eigenValuesRe;
    TPZFMatrix<STATE> eVector( eVectors.Rows() , 1);
    std::pair<REAL,TPZFMatrix<STATE> > duplet;
    
    for(int i = 0 ; i< eValues.size();i++){
        eVectors.GetSub(0, i, eVectors.Rows(), 1 , eVector);
        duplet.first = eValues[i].real();
        duplet.second = eVector;
        eigenValuesRe.insert(duplet);
    }
    int i = 0;
	std::string pathName;
	if(generatingResults){
		struct stat sb;
		std::string command;
		pathName = "../resultsQuali";
		if (!(stat(pathName.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)))
		{
			command = "mkdir";
			command.append(" ../resultsQuali");
			std::system(command.c_str());
		}
	}
	else{
		pathName = "..";
	}
	
	std::string fileName = pathName;
	fileName.append("/ev");
	fileName.append(std::to_string(nDiv));
	fileName.append(".csv");

    std::ofstream fileEigenValues(fileName.c_str());
	
    for (std::set<std::pair<REAL,TPZFMatrix<STATE> > > ::iterator iT = eigenValuesRe.begin(); iT != eigenValuesRe.end(); iT++) {
        if(isCutOff){
            if(std::abs(iT->first) < 1e-2 ) continue;
            std::cout<< iT->first <<std::endl;
            
            
            
            i++;
            fileEigenValues<<iT->first<<std::endl;
            if( i >= nSolutions)
                break;
        }
        else
        {
            
            fileEigenValues<<iT->first<<std::endl;
            i++;
            if( i >= nSolutions)
                continue;
            if( i > 50)
                break;
            std::cout<< iT->first <<std::endl;
        }
    }
    if (isCutOff||generatingResults) {
        return;
    }
    std::string fileNameEVec("../evec");
    fileNameEVec.append(std::to_string(nDiv));
    fileNameEVec.append(".txt");
    std::ofstream fileEigenVectors(fileNameEVec.c_str());
    for (std::set<std::pair<REAL,TPZFMatrix<STATE> > > ::iterator iT = eigenValuesRe.begin(); iT != eigenValuesRe.end(); iT++) {
        fileEigenVectors<<iT->second<<std::endl;
        //std::cout<< "eigvec "<<iT->second <<std::endl;
        i++;
        if( i >= nSolutions)
            continue;
        if( i > 50)
            break;
    }
    
    std::cout << "Post Processing..." << std::endl;
    
    
    {
        std::ofstream fileA("../EV.csv");
        char number[256];
        std::cout<<eigenValuesRe.begin()->first<<std::endl;
        for (int i = 0; i<eigenValuesRe.begin()->second.Rows(); i++) {
            for(int j = 0 ; j<eigenValuesRe.begin()->second.Cols();j++){
                sprintf(number, "%32.32Lf",(long double) TPZExtractVal::val(std::real(eigenValuesRe.begin()->second.GetVal(i,j))) );
                fileA<<number;
                
                fileA<<" + I * ";
                
                sprintf(number, "%32.32Lf",(long double) TPZExtractVal::val(std::imag(eigenValuesRe.begin()->second.GetVal(i,j))) );
                fileA<<number;
                if( j != eigenValuesRe.begin()->second.Cols() - 1){
                    fileA<<" , ";
                }
            }
            fileA<<std::endl;
        }
        fileA.close();
    }
    TPZFMatrix<STATE> solMat(neqOriginal,1,0.);
    
    TPZStack<std::string> scalnames, vecnames;
    scalnames.Push("Ez");//setando para imprimir u
    scalnames.Push("p");
    vecnames.Push("Et");
    std::string plotfile= "../waveguideModes.vtk";//arquivo de saida que estara na pasta debug
    int dim = 2;
    an.DefineGraphMesh(dim, scalnames, vecnames, plotfile);//define malha grafica
    int postProcessResolution = 0;//define resolucao do pos processamento
    
    
    std::set<std::pair<REAL,TPZFMatrix<STATE> > > ::iterator iT = eigenValuesRe.begin();
    for (int iSol = 0; iSol < nSolutions; iSol ++) {
        if( iT == eigenValuesRe.end() ){
            DebugStop();
        }
        for (int i = 0 ; i < neq; i++) {
            solMat( activeEquations[i] ,0) = (iT->second).GetVal(i, 0);
        }
        an.LoadSolution( solMat );
        TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(temporalMeshVec, cmeshMF);
        an.PostProcess(postProcessResolution);
        iT++;
    }
    std::cout << "FINISHED!" << std::endl;
}

void FilterBoundaryEquations(TPZVec<TPZCompMesh *> meshVec , TPZVec<long> &activeEquations , int &neq , int &neqOriginal)
{
    TPZCompMesh *cmeshMF = meshVec[0];
    TPZCompMesh *cmeshHCurl = meshVec[1];
    TPZCompMesh *cmeshH1 = meshVec[2];
    TPZCompMesh *cmeshLag = meshVec[3];
    
    TPZManVector<long,1000> allConnects;
    std::set<long> boundaryConnects;
    
    
    for (int iel = 0; iel < cmeshMF->NElements(); iel++) {
        TPZCompEl *cel = cmeshMF->ElementVec()[iel];
        if ( cel == NULL) {
            continue;
        }
        if ( cel->Reference() == NULL) {
            
            continue;
        }
        if(cel->Reference()->MaterialId() == -1 ){
            std::set<long> connectsEl;
            cel->BuildConnectList(connectsEl);
            //            std::cout<<"# connects "<<cel->NConnects()<<std::endl;
            //            std::cout<<"all"<<std::endl;
            for (std::set<long>::iterator iT = connectsEl.begin(); iT != connectsEl.end(); iT++) {
                const long val = *iT;
                if( boundaryConnects.find(val) == boundaryConnects.end() ){
                    boundaryConnects.insert(val);
                }
            }
        }
    }
    for (int iCon = 0; iCon < cmeshMF->NConnects(); iCon++) {
        if( boundaryConnects.find( iCon ) == boundaryConnects.end() ){
            TPZConnect &con = cmeshMF->ConnectVec()[iCon];
            if( con.HasDependency() ){
                std::cout<<iCon<<std::endl;
                continue;
            }
            int seqnum = con.SequenceNumber();
            int pos = cmeshMF->Block().Position(seqnum);
            int blocksize = cmeshMF->Block().Size(seqnum);
            if(blocksize == 0) continue;
            
            int vs = activeEquations.size();
            activeEquations.Resize(vs+blocksize);
            for(int ieq = 0; ieq<blocksize; ieq++)
            {
                activeEquations[vs+ieq] = pos+ieq;
            }
        }
    }
    
    std::cout<<"------\t------\t-------"<<std::endl;
    std::cout<<"cmeshH1->NEquations()"<<"\t"<<cmeshH1->NEquations()<<std::endl;
    std::cout<<"cmeshHCurl->NEquations()"<<"\t"<<cmeshHCurl->NEquations()<<std::endl;
    std::cout<<"cmeshLag->NEquations()"<<"\t"<<cmeshLag->NEquations()<<std::endl;
    std::cout<<"cmeshMF->NEquations()"<<"\t"<<cmeshMF->NEquations()<<std::endl;
    neqOriginal = cmeshMF->NEquations();
    
    int nHCurlEquations = 0 , nH1Equations = 0 , nLagEquations = 0;
    std::cout<<"activeEquations"<<std::endl;
    long nEq = 0;
    TPZMatMFHCurlH1Lag *mat = dynamic_cast<TPZMatMFHCurlH1Lag*>(cmeshMF->FindMaterial(1));
    for (int iCon = 0; iCon < cmeshMF->NConnects(); iCon++) {
        bool isH1, isLag;
        if( boundaryConnects.find( iCon ) == boundaryConnects.end() ){
            if( cmeshMF->ConnectVec()[iCon].HasDependency() ) continue;
            int seqnum = cmeshMF->ConnectVec()[iCon].SequenceNumber();
            int blocksize = cmeshMF->Block().Size(seqnum);
            if( iCon > cmeshH1->NConnects() + cmeshHCurl->NConnects()){
                isLag = true;
            }
            else{
                isLag = false;
                if( mat->H1Index() == 0 && iCon < cmeshH1->NConnects() ){
                    isH1 = true;
                }
                else if( mat->H1Index() == 1 && iCon >= cmeshHCurl->NConnects() ){
                    isH1 = true;
                }
                else{
                    isH1 = false;
                }
            }
            
            for(int ieq = 0; ieq<blocksize; ieq++)
            {
                nEq++;
                if (isLag) {
                    nLagEquations++;
                }
                else{
                    isH1 == true ? nH1Equations ++ : nHCurlEquations++;
                }
            }
        }
    }
    std::cout<<"# H1 equations: "<< nH1Equations<<std::endl;
    std::cout<<"# HCurl equations: "<<nHCurlEquations<<std::endl;
    std::cout<<"# Lagrange equations: "<<nLagEquations<<std::endl;
    std::cout<<"# equations: "<<nEq<<std::endl;
    //std::cout<<activeEquations<<std::endl;
    neq = nEq;
    return;
}

void CreateGMesh(TPZGeoMesh * &gmesh, const int meshType, const REAL hDomain, const REAL wDomain,  const int xDiv, const int yDiv)
{
    
    TPZManVector<int,3> nx(3,0);
    TPZManVector<REAL,3> llCoord(3,0.) , ulCoord(3,0.) , urCoord(3,0.) , lrCoord(3,0.);
    llCoord[0] = -wDomain/2;
    llCoord[1] = -hDomain/2;
    
    ulCoord[0] = -wDomain/2;
    ulCoord[1] = hDomain/2;
    
    urCoord[0] = wDomain/2;
    urCoord[1] = hDomain/2;
    
    lrCoord[0] = wDomain/2;
    lrCoord[1] = -hDomain/2;
    
    nx[0]=xDiv;
    nx[1]=yDiv;
    int numl = 1;
    TPZGenGrid *gengrid = NULL;
    switch (meshType) {
        case createRectangular:
        {
            REAL rot = 0.0;
            gengrid = new TPZGenGrid(nx, llCoord, urCoord,  numl, rot);
            gengrid->SetElementType(EQuadrilateral);
        }
            break;
        case createTriangular:
        {
            REAL rot = 0.0;
            gengrid = new TPZGenGrid(nx, llCoord, urCoord,  numl, rot);
            gengrid->SetElementType(ETriangle);
        }
            break;
        case createZigZag:
        {
            REAL rot = 0.0;
            gengrid = new TPZGenGrid(nx, llCoord, urCoord,  numl, rot);
            gengrid->SetElementType(EQuadrilateral);
            gengrid->SetZigZagPattern();
        }
            break;
        default:
            DebugStop();
            break;
    }
    gmesh = new TPZGeoMesh();
    const int matId = 1; //define id para um material(formulacao fraca)
    const int bc0 = -1; //define id para um material(cond contorno dirichlet)
    gengrid->Read(gmesh , matId);
    
    
    gengrid->SetBC(gmesh, ulCoord, llCoord, bc0);
    gengrid->SetBC(gmesh, urCoord, ulCoord, bc0);
    gengrid->SetBC(gmesh, lrCoord, urCoord, bc0);
    gengrid->SetBC(gmesh, llCoord, lrCoord, bc0);
    
    //gmesh->ResetConnectivities();
    
    gmesh->BuildConnectivity();
#ifdef PZDEBUG
    std::ofstream outTxt , outVtk;
    switch (meshType) {
        case createRectangular:
        {
            outTxt.open("../gmeshRectangular.txt"); //define arquivo de saida para impressao da malha no
            outVtk.open("../gmeshRectangular.vtk"); //define arquivo de saida para impressao da malha no paraview
            
        }
            break;
        case createTriangular:
        {
            outTxt.open("../gmeshTriangular.txt"); //define arquivo de saida para impressao da malha no
            outVtk.open("../gmeshTriangular.vtk"); //define arquivo de saida para impressao da malha no paraview
            
        }
            break;
        case createZigZag:
        {
            outTxt.open("../gmeshZigZag.txt"); //define arquivo de saida para impressao da malha no
            outVtk.open("../gmeshZigZag.vtk"); //define arquivo de saida para impressao da malha no paraview
        }
            break;
        default:
            DebugStop();
            break;
    }
    gmesh->Print(outTxt);
    outTxt.close();
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh, outVtk, true); //imprime a malha no formato vtk
    outVtk.close();
#endif
    delete gengrid;
}

TPZVec<TPZCompMesh *>CMesh(TPZGeoMesh *gmesh, int pOrder, STATE (& ur)( const TPZVec<REAL> &),STATE (& er)( const TPZVec<REAL> &), REAL f0 , bool isCutOff)
{
    
    const int dim = 2; //dimensao do problema
    const int matId = 1; //define id para um material(formulacao fraca)
    const int bc0 = -1; //define id para um material(cond contorno dirichlet)
    enum{ dirichlet = 0, neumann, mixed}; //tipo da condicao de contorno do problema
    // Criando material
    
    
    ///criar malha computacional H1
    
    TPZCompMesh * cmeshH1 = new TPZCompMesh(gmesh);
    cmeshH1->SetDefaultOrder(pOrder);//seta ordem polimonial de aproximacao
    cmeshH1->SetDimModel(dim);//seta dimensao do modelo
    // Inserindo material na malha
    const int nState = 1;
    TPZVec<STATE> sol;//only for creating material. this material will not be used in reality
    TPZL2Projection *matH1 = new TPZL2Projection( matId, dim, nState, sol);
    cmeshH1->InsertMaterialObject(matH1);
    
    ///electrical conductor boundary conditions
    TPZFNMatrix<1,STATE> val1(1,1,0.), val2(1,1,0.);
    
    TPZMaterial * BCondH1Dir = matH1->CreateBC(matH1, bc0, dirichlet, val1, val2);//cria material que implementa a condicao de contorno de dirichlet
    
    cmeshH1->InsertMaterialObject(BCondH1Dir);//insere material na malha
    //Cria elementos computacionais que gerenciarao o espaco de aproximacao da malha
    cmeshH1->AutoBuild();
    cmeshH1->CleanUpUnconnectedNodes();
    
    ///criar malha computacional HCurl
    
    TPZCompMesh * cmeshHCurl = new TPZCompMesh(gmesh);
    cmeshHCurl->SetDefaultOrder(pOrder);//seta ordem polimonial de aproximacao
    cmeshHCurl->SetDimModel(dim);//seta dimensao do modelo
    // Inserindo material na malha
    TPZMatHCurlProjection *matHCurl = new TPZMatHCurlProjection(matId);
    cmeshHCurl->InsertMaterialObject(matHCurl);
    
    val1( 0, 0 ) = 0.;
    val2( 0, 0 ) = 0.;
    TPZMaterial * BCondHCurlDir = matHCurl->CreateBC(matHCurl, bc0, dirichlet, val1, val2);//cria material que implementa a condicao de contorno de dirichlet
    
    cmeshHCurl->InsertMaterialObject(BCondHCurlDir);//insere material na malha
    
    cmeshHCurl->SetAllCreateFunctionsHDiv();//define espaco de aproximacao
    cmeshHCurl->AutoBuild();
    
    
    TPZAdmChunkVector< TPZCompEl* > elVec = cmeshHCurl->ElementVec();
	
    cmeshHCurl->CleanUpUnconnectedNodes();
    
    
    ///creates lagrange multiplier mesh
    
    TPZCompMesh * cmeshLag = new TPZCompMesh(gmesh);
    cmeshLag->SetDefaultOrder(pOrder);//seta ordem polimonial de aproximacao
    cmeshLag->SetDimModel(dim);//seta dimensao do modelo
    // Inserindo material na malha
    
    TPZL2Projection *matLag = new TPZL2Projection( matId, dim, nState, sol);
    cmeshLag->InsertMaterialObject(matLag);
    
    ///electrical conductor boundary conditions
    
    TPZMaterial * BCondLagDir = matLag->CreateBC(matLag, bc0, dirichlet, val1, val2);//cria material que implementa a condicao de contorno de dirichlet
    
    cmeshLag->InsertMaterialObject(BCondLagDir);//insere material na malha
    //Cria elementos computacionais que gerenciarao o espaco de aproximacao da malha
    cmeshLag->AutoBuild();
    
    for (int iT = 0; iT < cmeshLag->NConnects(); iT++) {
        cmeshLag->ConnectVec()[iT].SetLagrangeMultiplier(1);
    }
    
    cmeshLag->CleanUpUnconnectedNodes();
    
    TPZMatMFHCurlH1Lag *matMultiPhysics = NULL;
    TPZVec<TPZCompMesh *> meshVec(3);
    if (isCutOff) {
        TPZMatWaveguideCutOffAnalysisLag * dummy = new TPZMatWaveguideCutOffAnalysisLag(matId , f0 , ur , er);
        matMultiPhysics = dummy;
    }
    else{
        TPZMatMFHCurlH1Lag * dummy = new TPZMatMFHCurlH1Lag(matId , f0 , ur , er);
        matMultiPhysics = dummy;
    }
    meshVec[ matMultiPhysics->H1Index() ] = cmeshH1;
    meshVec[ matMultiPhysics->HCurlIndex() ] = cmeshHCurl;
    meshVec [ matMultiPhysics->LagrangeIndex() ] = cmeshLag;
    
    //  TPZMatHCurl2D *matMultiPhysics = new TPZMatHCurl2D(matId , lambda , ur , er , e0 , theta, scale);
    
    
    TPZCompMesh *cmeshMF = new TPZCompMesh( gmesh );
    
    val1( 0, 0 ) = 0.;
    val2( 0, 0 ) = 0.;
    TPZMaterial * BCondMFDir = matMultiPhysics->CreateBC(matMultiPhysics, bc0, dirichlet, val1, val2);//cria material que implementa a condicao de contorno de dirichlet
    
    
    cmeshMF->InsertMaterialObject(matMultiPhysics);
    cmeshMF->InsertMaterialObject(BCondMFDir);//insere material na malha
    std::set<int> set;
    set.insert(matId);
    set.insert(bc0);
    
    
    cmeshMF->SetDimModel(dim);
    cmeshMF->SetAllCreateFunctionsMultiphysicElem();
    
    cmeshMF->AutoBuild(set);
    cmeshMF->CleanUpUnconnectedNodes();
    
    TPZBuildMultiphysicsMesh::AddElements(meshVec, cmeshMF);
    TPZBuildMultiphysicsMesh::AddConnects(meshVec, cmeshMF);
    TPZBuildMultiphysicsMesh::TransferFromMeshes(meshVec, cmeshMF);
    cmeshMF->CleanUpUnconnectedNodes();
    
    
    cmeshMF->ExpandSolution();
    cmeshMF->CleanUpUnconnectedNodes();
    cmeshMF->ComputeNodElCon();
    cmeshMF->CleanUpUnconnectedNodes();
    std::ofstream fileH1("../cmeshH1.txt");
    cmeshH1->Print(fileH1);
    std::ofstream fileHCurl("../cmeshHCurl.txt");
    cmeshHCurl->Print(fileHCurl);
    std::ofstream fileLag("../cmeshLag.txt");
    cmeshLag->Print(fileLag);
    std::ofstream fileMF("../cmeshMFHCurl.txt");
    cmeshMF->Print(fileMF);
    
    TPZVec< TPZCompMesh *>meshVecOut(4);
    
    meshVecOut[0] = cmeshMF;
    meshVecOut[1 + matMultiPhysics->H1Index()] = cmeshH1;
    meshVecOut[1 + matMultiPhysics->HCurlIndex()] = cmeshHCurl;
    meshVecOut[1 + matMultiPhysics->LagrangeIndex()] = cmeshLag;
    
    return meshVecOut;
}