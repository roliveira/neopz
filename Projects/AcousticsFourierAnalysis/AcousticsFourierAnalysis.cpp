/**
 * @file
 * @brief TODO:DESCRIBE IT
 * @details BLABLABLA
 *
 * @author Francisco Orlandini
 * @since 2018
 */

#include <TPZSSpStructMatrix.h>
#include <TPZSpStructMatrix.h>
#include <pzskylstrmatrix.h>
#include <pzsbstrmatrix.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <TPZGmshReader.h>
#include <pzl2projection.h>
#include <TPZSkylineNSymStructMatrix.h>
#include <pzskylnsymmat.h>
#include <pzgeopoint.h>
#include <pzgeoelrefless.h>
#include "TPZVTKGeoMesh.h"
#include "pzanalysis.h"
#include "pzbndcond.h"
#include "pzelchdiv.h"
#include "pzfstrmatrix.h"
#include "pzgengrid.h"
#include "pzgmesh.h"
#include "pzshapequad.h"
#include "pzshapetriang.h"
#include "pzstepsolver.h"
#include "TPZMatAcousticsFourier.h"
#include "TPZMatAcousticsPml.h"


struct SPZAcousticData{
//    enum pmltype{ TODO: tarefa pra 2019 (2020?)
//        xp=0,yp,xm,ym,xpyp,xmyp,xmym,xpym
//    };
    enum EBoundType{
        softwall = 0, hardwall = 1
    };
    enum ESimulationType{
        transient = 0, frequencyDomain = 1
    };
    struct SPZSimulationSettings{
        std::string resultsDir = "results/";
        std::string meshName = "iDontExist.geo";
        TPZVec<SPZAcousticData::EBoundType> boundType;
        int nElemPerLambda;
        REAL totalTime;
        REAL cfl;
        REAL nTimeSteps;
        bool isCflBound;
        int pOrder;
    };
    struct SPZSourceSettings{
        REAL posX;
        REAL posY;
        REAL amplitude;
        REAL peakTime;
        REAL centralFrequency;
    };
    struct SPZFrequencyDomainSettings{
        REAL wSample;
        REAL wMax;
        REAL alphaFreqShift;
    };
    struct SPZOutputSettings{
        bool printGmesh;
        bool printCmesh;
        bool vtkSol;
        int vtkResolution;
    };

    SPZSimulationSettings fSimulationSettings;
    SPZSourceSettings fSourceSettings;
    SPZFrequencyDomainSettings fFrequencyDomainSettings;
};



void CreateGMesh(TPZGeoMesh *&gmesh, const std::string mshFileName, TPZVec<int> &matIdVec, const bool &print,
        REAL nElemPerLambdaTimesOmega, const std::string &prefix, TPZVec<REAL> &elSizes,
        std::map<int,REAL> &velocityMap, std::map<int,REAL> &rhoMap);

void CreateSourceNode(TPZGeoMesh * &gmesh, const int &matIdSource, const REAL &sourcePosX, const REAL &sourcePosY);

void FilterBoundaryEquations(TPZCompMesh *cmesh, TPZVec<int64_t> &activeEquations, int &neq, int &neqOriginal);

void
CreateCMesh(TPZCompMesh *&cmesh, TPZGeoMesh *gmesh, int pOrder, void (&loadVec)(const TPZVec<REAL> &, TPZVec<STATE> &),
            const std::string &prefix, const bool &print, const TPZVec<int> &matIdVec,
            const std::map<int,REAL> &rhoMap,const std::map<int,REAL> &velocityMap,
//          const REAL &alphaPml, const TPZVec< SPZAcousticData::pmltype > &pmlTypeVec,
            const TPZVec<SPZAcousticData::EBoundType> &boundTypeVec);

void GetMaterialProperties(std::map<std::string,int> &materialNames, std::map<int,REAL> &rhoMap,
                           std::map<int,REAL> &velocityMap);

void RunSimulation(SPZAcousticData &simData);

int main(int argc, char *argv[]) {
#ifdef LOG4CXX
    InitializePZLOG();
#endif
    const std::string prefix = "results/";//PARAMS
    int pOrder = 2; //PARAMS
    const int nDivIni = 4; //PARAMS
    const int nPcycles = 1;
    const int nHcycles = 1;
    SPZAcousticData::caseNames whichCase = SPZAcousticData::caseNames::realWell1;
    const REAL wZero = (whichCase == SPZAcousticData::caseNames::concentricMesh) || (whichCase == SPZAcousticData::caseNames::realWell1) ?
            18 * 1000 * 2 *M_PI : 100 * 2 * M_PI;
    boost::posix_time::ptime t1 =
            boost::posix_time::microsec_clock::local_time();
    for (int iP = 0; iP < nPcycles; ++iP, ++pOrder) {
        int nDiv = nDivIni;
        for (int iH = 0; iH < nHcycles; ++iH) {
            std::cout << iH+1<<"/"<<nHcycles<<": Beginning simulation with nEl = "
                      << nDiv * nDiv * 2
                      <<" and p = "<<pOrder<< std::endl;
            RunSimulation(nDiv, pOrder, prefix,wZero,whichCase);
            nDiv *= 2;
            std::cout << "************************************" << std::endl;
        }
    }
    boost::posix_time::ptime t2 =
            boost::posix_time::microsec_clock::local_time();
    std::cout<<"Total time: "<<t2-t1<<std::endl;
    return 0;
}

void RunSimulation(const int &nDiv, const int &pOrder, const std::string &prefix, const REAL &wZero, SPZAcousticData::caseNames whichCase) {

    ///////////TODO: MISSING
    REAL wSample = wMax/nSamples;
    if(2 * M_PI /wSample < totalTime){
        std::cout<<"sampling is not good. hmmmmmmm."<<std::endl;
    }




    TPZAnalysis an(cmesh);
    // configuracoes do objeto de analise
    TPZManVector<int64_t, 1000> activeEquations;
    int neqReduced = 0;
    int neqOriginal = 0;


//#define USING_SKYLINE

#ifdef USING_SKYLINE
    TPZSkylineNSymStructMatrix structMatrix(cmesh);
#else
    TPZSpStructMatrix structMatrix(cmesh);
#endif
    structMatrix.SetNumThreads(nThreads);
    bool filter = true;
    if(filter){
        FilterBoundaryEquations(cmesh, activeEquations, neqReduced, neqOriginal);
        structMatrix.EquationFilter().SetActiveEquations(activeEquations);
    }else{
        neqOriginal = cmesh->NEquations();
    }
    TPZStepSolver<STATE> step;
    step.SetDirect(ELU);
    an.SetSolver(step);
    an.SetStructuralMatrix(structMatrix);

    ////////////////////////////////////////////////////////////////////////
    std::set<int> matsPml;
    std::set<int> matsNonPml;
    std::set<int> matSource;
    for(auto itMap : cmesh->MaterialVec()) {
        TPZMatAcousticsPml *matPml = dynamic_cast<TPZMatAcousticsPml *>(itMap.second);
        TPZMatAcousticsFourier *matH1 = dynamic_cast<TPZMatAcousticsFourier *>(itMap.second);
        if (matH1 != nullptr && matPml == nullptr) {
            if(matH1->Id() == matIdSource){
                matSource.insert(matH1->Id());
                continue;
            }
            matsNonPml.insert(matH1->Id());
        } else if (matPml != nullptr) {
            matsPml.insert(matPml->Id());
        }
    }


#ifdef USING_SKYLINE
    TPZSkylNSymMatrix<STATE> matM;
    TPZSkylNSymMatrix<STATE> matK;
#else
    TPZFYsmpMatrix<STATE> matM;
    TPZFYsmpMatrix<STATE> matK;
#endif


    {
        TPZAutoPointer<TPZStructMatrix> structMatrixPtr = an.StructMatrix();
        structMatrixPtr->SetMaterialIds(matsNonPml);
        //set structMatrix M
        for (std::set<int>::iterator it=matsNonPml.begin(); it!=matsNonPml.end(); ++it){
            TPZMatAcousticsFourier *matH1 = dynamic_cast<TPZMatAcousticsFourier *>(cmesh->MaterialVec()[*it]);
            matH1->SetAssemblingMatrix(TPZMatAcousticsFourier::M);
        }
        an.Assemble();
        //get structMatrix M

#ifdef USING_SKYLINE
        auto mat = dynamic_cast<TPZSkylNSymMatrix<STATE> *>( an.Solver().Matrix().operator->() );
#else
        auto mat = dynamic_cast<TPZFYsmpMatrix<STATE> *>( an.Solver().Matrix().operator->() );
#endif
        if(!mat){
            DebugStop();
        }
        matM = *mat;
        //set structMatrix K
        for (std::set<int>::iterator it=matsNonPml.begin(); it!=matsNonPml.end(); ++it){
            TPZMatAcousticsFourier *matH1 = dynamic_cast<TPZMatAcousticsFourier *>(cmesh->MaterialVec()[*it]);
            matH1->SetAssemblingMatrix(TPZMatAcousticsFourier::K);
        }
        an.Assemble();
        //get structMatrix K
#ifdef USING_SKYLINE
        mat = dynamic_cast<TPZSkylNSymMatrix<STATE> *>( an.Solver().Matrix().operator->() );
#else
        mat = dynamic_cast<TPZFYsmpMatrix<STATE> *>( an.Solver().Matrix().operator->() );
#endif
        if(!mat){
            DebugStop();
        }
        matK = *mat;
    }


    ////////////////////////////////////////////////////////////////////////
    TPZFMatrix<STATE> timeDomainSolution(neqOriginal,nTimeSteps,0.);
    TPZFMatrix<STATE> rhsFake(cmesh->NEquations(),1);

    TPZCompEl * sourceCompel = nullptr;
    {
        TPZGeoElRefLess<pzgeom::TPZGeoPoint> *sourceEl = nullptr;
        for (int iEl = 0; iEl < gmesh->NElements(); ++iEl) {
            if (gmesh->Element(iEl)->MaterialId() == matIdSource){
                sourceEl = dynamic_cast<TPZGeoElRefLess<pzgeom::TPZGeoPoint>*>(gmesh->Element(iEl));
                if(sourceEl){
                    break;
                }else{
                    DebugStop();
                }
            }
        }
        if(sourceEl == nullptr){
            std::cout<<"could not find source element"<<std::endl;
            DebugStop();
        }
        sourceCompel = sourceEl->Reference();
    }

    const REAL aFactor = log(50)/(2*M_PI/wSample);
    TPZFMatrix<STATE> frequencySolution(nSamples-1,2);

    auto source = [wZero,peakTime,amplitude](const STATE &currentW, STATE & val){
        val = 0.;
        val += -1. * SPZAlwaysComplex<STATE>::type(0,1)  * amplitude * currentW * M_SQRT2/wZero;
        val*= exp(SPZAlwaysComplex<STATE>::type(0,1)  * currentW * peakTime - (currentW/wZero)*(currentW/wZero));
    };

    {
        TPZMatAcousticsFourier *mat = dynamic_cast<TPZMatAcousticsFourier *>(cmesh->MaterialVec()[matIdSource]);
        if(mat == nullptr){
            DebugStop();
        }
        mat->SetSource(source);
    }

    for(int iW = 0; iW < nSamples-1; iW++){

#ifdef USING_SKYLINE
        TPZAutoPointer<TPZMatrix<STATE>> matFinal(new TPZSkylNSymMatrix<STATE>(matK));
#else
        TPZAutoPointer<TPZMatrix<STATE>> matFinal(new TPZFYsmpMatrix<STATE>(matK.Rows(),matK.Cols()));
#endif
        const STATE currentW = (iW+1) * wSample + SPZAlwaysComplex<STATE>::type(0,1)*aFactor;
        //-w^2 M + K
        boost::posix_time::ptime t1_sum =
                boost::posix_time::microsec_clock::local_time();

#ifdef USING_SKYLINE
        for(int iCol = 0; iCol < matFinal->Cols(); iCol++){
            TPZSkylNSymMatrix<STATE> * matFinalDummy = dynamic_cast<TPZSkylNSymMatrix<STATE> *>(matFinal.operator->());
            const int nRows = matFinalDummy->SkyHeight(iCol);
            for(int iRow = iCol; iRow >= iCol - nRows; iRow --){
                matFinal->PutVal(iRow,iCol, -1.*currentW*currentW*matM.GetVal(iRow,iCol) + matK.GetVal(iRow,iCol));
                matFinal->PutVal(iCol,iRow, -1.*currentW*currentW*matM.GetVal(iCol,iRow) + matK.GetVal(iCol,iRow));
            }
        }
#else
        TPZVec<int64_t> ia;
        TPZVec<int64_t> ja;
        TPZVec<STATE> aM, aK, aFinal;
        matM.GetData(ia,ja,aM);
        matK.GetData(ia,ja,aK);
        aFinal.Resize(aK.size());

        TPZFYsmpMatrix<STATE> * matFinalDummy = dynamic_cast<TPZFYsmpMatrix<STATE> *>(matFinal.operator->());
        for(int i = 0; i < aFinal.size(); i++){
            aFinal[i] = -1.*currentW*currentW*aM[i] + aK[i];
        }
        matFinalDummy->SetData(ia,ja,aFinal);
#endif


        boost::posix_time::ptime t2_sum =
                boost::posix_time::microsec_clock::local_time();
        std::cout<<"Summing matrices took "<<t2_sum-t1_sum<<std::endl;


        //pml elements
        TPZAutoPointer<TPZStructMatrix> structMatrixPtr = an.StructMatrix();
        //assemble pml
        if(nPmls > 0){
            structMatrixPtr->SetMaterialIds(matsPml);
            for(auto itMap : matsPml) {
                TPZMatAcousticsPml *mat = dynamic_cast<TPZMatAcousticsPml *>(cmesh->FindMaterial(itMap));
                if (mat != nullptr) {
                    mat->SetW(currentW);
                } else {
                    DebugStop();
                }
            }
            structMatrixPtr->Assemble(matFinal,rhsFake,nullptr);
        }



        structMatrixPtr->SetMaterialIds(matSource);

        TPZMatAcousticsFourier *matSourcePtr = dynamic_cast<TPZMatAcousticsFourier *>(cmesh->MaterialVec()[matIdSource]);
        if(matSourcePtr == nullptr){
            DebugStop();
        }
        matSourcePtr->SetW(currentW);//FONTE IGUAL A DO COMSOL
        //assemble load vector
        an.AssembleResidual();
        //solve system
        an.Solver().SetMatrix(matFinal);

        std::cout<<"Beginning solver step "<<iW+1<<" out of "<<nSamples - 1<<std::endl;
        boost::posix_time::ptime t1_solve =
                boost::posix_time::microsec_clock::local_time();
        an.Solve();
        boost::posix_time::ptime t2_solve =
                boost::posix_time::microsec_clock::local_time();
        std::cout<<"solver step "<<iW+1<<" out of "<<nSamples - 1<<" took "<<t2_solve-t1_solve<<std::endl;
        //get solution
        TPZFMatrix<STATE> &currentSol = an.Solution();//p omega

        //get spectrum
        frequencySolution(iW,0) = currentW;
        TPZVec<REAL> qsi(1,0);
        int var = 1;
        TPZVec<STATE> sol(1,0);
        sourceCompel->Solution(qsi,var,sol);
        frequencySolution(iW,1) = sol[0];
        //inverse transform

        REAL timeStepSize = totalTime/nTimeSteps;
        for(int iPt = 0; iPt < neqOriginal; iPt++){
            for(int iTime = 0; iTime < nTimeSteps; iTime++){
                const REAL currentTime = (REAL)iTime * timeStepSize;
                timeDomainSolution(iPt,iTime) +=
                        std::exp(aFactor*currentTime)*//fator para compensar o shift na frequência
                        wSample *//fator para compensar o trem de impulso com intensidade wSample
                        0.5*(1.+std::cos(std::real(currentW)*M_PI/wMax))*//Hanning windowing, atenua frequencias altas
                        std::real(currentSol(iPt,0)) * std::cos(-1. * currentTime * std::real(currentW))*
                        M_2_SQRTPI * M_SQRT2 * M_SQRT1_2;//1/sqrt(2pi), para ser coerente com a
                        // transformada feita no mathematica
            }
        }
        //TODO:tirar isso, Fran. fix temporario para debugar mkl
        an.Solver().SetMatrix(nullptr);
    }

    if (genVTK) {
        std::cout<<"Post processing... "<<std::endl;
        {
            std::string matFileName = prefix+"freqSpectrum.csv";
            //void TPZMatrix<TVar>::Print(const char *name, std::ostream& out,const MatrixOutputFormat form) const {
            std::ofstream matFile(matFileName);
            if(!matFile.is_open()){
                DebugStop();
            }
            for(int irow = 0; irow < frequencySolution.Rows(); irow++){
                REAL omega = std::real(frequencySolution.Get(irow,0));
                REAL reSpectrum = std::real(frequencySolution.Get(irow,1));
                REAL imSpectrum = std::imag(frequencySolution.Get(irow,1));
                matFile<<omega<<"  ,  ";
                matFile<<reSpectrum<<" + I * "<<imSpectrum<<std::endl;
            }
            matFile.close();
        }

        TPZStack<std::string> scalnames, vecnames;
        scalnames.Push("Pressure");
        std::string plotfile = prefix+"sol";
        plotfile.append(std::to_string(cmesh->NElements()));
        plotfile.append(".vtk");

        an.DefineGraphMesh(2, scalnames, vecnames,
                           plotfile);  // define malha grafica
        int postProcessResolution = postprocessRes; // define resolucao do pos processamento

        TPZFMatrix<STATE> currentSol(neqOriginal,1);
        for(int iTime = 0; iTime < nTimeSteps; iTime++){
            std::cout<<"\rtime: "<<iTime+1<<" out of "<<nTimeSteps<<std::flush;
            for(int iPt = 0; iPt < neqOriginal; iPt++){
                currentSol(iPt,0) = timeDomainSolution(iPt,iTime);
            }
            an.LoadSolution(currentSol);
            an.PostProcess(postProcessResolution);
        }
        std::cout<<std::endl<<" Done!"<<std::endl;
    }
    //TODO:arrumar isso
//    gmesh->SetReference(nullptr);
//    for(int icon = 0; icon < cmesh->NConnects(); icon++){
//        TPZConnect &con = cmesh->ConnectVec()[icon];
//        con.RemoveDepend();
//    }
//    cmesh->SetReference(nullptr);
//    delete cmesh;
//    cmesh=nullptr;
//    delete gmesh;
//    gmesh=nullptr;
}

void GetMaterialProperties(std::map<std::string,int> &materialNames, std::map<int,REAL> &rhoMap,
        std::map<int,REAL> &velocityMap) {

    std::map<std::string,std::pair<REAL,REAL>> dataBase;//rho,velocity
    dataBase.insert(std::make_pair<>("water",std::make_pair<REAL,REAL>(1000,1500)));
    dataBase.insert(std::make_pair<>("steel",std::make_pair<REAL,REAL>(7850,5960)));
    dataBase.insert(std::make_pair<>("cement",std::make_pair<REAL,REAL>(3150,3500)));
    dataBase.insert(std::make_pair<>("rock",std::make_pair<REAL,REAL>(2750,5950)));

    for(auto &itMap : materialNames){
        auto possibleMat = dataBase.find(itMap.first);
        if(possibleMat == dataBase.end()){
            PZError << "you are trying to create a nonexistent material."<<std::endl;
            DebugStop();
        }
        rhoMap[itMap.second] = possibleMat->second.first;
        velocityMap[itMap.second] = possibleMat->second.second;
    }
}

void
CreateGMesh(TPZGeoMesh *&gmesh, const std::string mshFileName, TPZVec<int> &matIdVec, const bool &print,
        TPZVec<std::string> paramsName, TPZVec<REAL> paramsVal, REAL nElemPerLambdaTimesOmega,
        const std::string &prefix, TPZVec<REAL> &elSizes, std::map<int,REAL> &velocityMap, std::map<int,REAL> &rhoMap){
    std::map<std::string,int> materialNames;
    //FIRST RUN (JUST TO READ THE MATERIALS AND TO SET EL SIZE)
    {
        std::string command = "gmsh " + mshFileName + " -0 -v 0";
        command += " -o " + prefix + "wellMesh.msh";
        std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) throw std::runtime_error("popen() failed!");

        TPZGmshReader meshReader;
        gmesh = meshReader.GeometricGmshMesh(prefix+"wellMesh.msh", nullptr, false);
        materialNames = meshReader.fPZMaterialId[2];
        elSizes.Resize(materialNames.size());
        GetMaterialProperties(materialNames, rhoMap, velocityMap);
        int i =0;
        for(auto iVelocity : velocityMap){
            elSizes[i] = 2 *M_PI*iVelocity.second / nElemPerLambdaTimesOmega;
            i++;
        }
        delete gmesh;
        gmesh = nullptr;
    }

    std::string command = "gmsh " + mshFileName + " -2 -match ";
    command += " -v 3 ";
#ifdef PZDEBUG
    if(paramsName.size() != paramsVal.size()){
        DebugStop();
    }
#endif
    for(int i = 0; i < elSizes.size(); i++){
        std::ostringstream str_elSize;
        str_elSize << std::setprecision(16) << elSizes[i];
        command += " -setnumber el_size_"+std::to_string(i+1)+" "+str_elSize.str();
    }
    const int nParams = paramsName.size();
    for(int i = 0; i < nParams; i++){
        std::ostringstream val;
        val <<std::setprecision(16)<< paramsVal[i];
        command += " -setnumber "+paramsName[i]+" "+val.str();
    }
    command += " -o " + prefix + "wellMesh.msh";
    std::cout<<"Generating mesh with: "<<std::endl<<command<<std::endl;

    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), 128, pipe.get()) != nullptr){
        result += buffer.data();
    }
    std::cout<<result<<std::endl;


    TPZGmshReader meshReader;
    gmesh = meshReader.GeometricGmshMesh(prefix+"wellMesh.msh");
#ifdef PZDEBUG
    TPZCheckGeom * Geometrytest = new TPZCheckGeom(gmesh);
    int isBadMeshQ = Geometrytest->PerformCheck();

    if (isBadMeshQ) {
        DebugStop();
    }
#endif
    auto matIds = meshReader.fMaterialDataVec;
    const int n1dMat = meshReader.fMatIdTranslate[1].size();
    const int n2dMat = meshReader.fMatIdTranslate[2].size();
    matIdVec.Resize(n2dMat+n1dMat);
    int imat = 0;
    ///at this point, the materials in the .geo must be declared in a crescent order
    for (auto& kv : meshReader.fMatIdTranslate[2]) {
        matIdVec[imat] = kv.first;
        imat++;
    }
    for (auto& kv : meshReader.fMatIdTranslate[1]) {
        matIdVec[imat] = kv.first;
        imat++;
    }
//    for (auto& kv : meshReader.fMaterialDataVec[1]) {
//        matIdVec[imat] = kv.first;
//        imat++;
//    }
    if(print){
        std::string meshFileName = prefix + "gmesh";
        const size_t strlen = meshFileName.length();
        meshFileName.append(".vtk");
        std::ofstream outVTK(meshFileName.c_str());
        meshFileName.replace(strlen, 4, ".txt");
        std::ofstream outTXT(meshFileName.c_str());

        TPZVTKGeoMesh::PrintGMeshVTK(gmesh, outVTK, true);
        gmesh->Print(outTXT);
        outTXT.close();
        outVTK.close();
    }

    return;
}

void CreateSourceNode(TPZGeoMesh * &gmesh, const int &matIdSource, const REAL &sourcePosX, const REAL &sourcePosY) {
    int64_t sourceNodeIndex = -1;
    REAL minDist = 1e12;
    //////
    for(int iEl = 0; iEl < gmesh->NElements(); iEl++){
        const TPZGeoEl & currentEl = *(gmesh->ElementVec()[iEl]);
        const int nNodes = currentEl.NCornerNodes();
        for(int iNode = 0; iNode < nNodes; iNode++){
            const TPZGeoNode & currentNode = currentEl.Node(iNode);
            const REAL xNode = currentNode.Coord(0);
            const REAL yNode = currentNode.Coord(1);
            const REAL currentDist =
                    (sourcePosX - xNode)*(sourcePosX - xNode) + (sourcePosY - yNode)*(sourcePosY - yNode);
            if(currentDist < minDist){
                minDist = currentDist;
                sourceNodeIndex = currentNode.Id();
            }
        }
    }
    TPZVec<int64_t> nodeIdVec(1,sourceNodeIndex);
    TPZGeoElRefLess<pzgeom::TPZGeoPoint > *zeroDEl =
            new TPZGeoElRefLess<pzgeom::TPZGeoPoint >(nodeIdVec, matIdSource,
                                                      *gmesh);
    gmesh->BuildConnectivity();
}

void FilterBoundaryEquations(TPZCompMesh *cmeshHCurl,
                             TPZVec<int64_t> &activeEquations, int &neq,
                             int &neqOriginal) {
    TPZManVector<int64_t, 1000> allConnects;
    std::set<int64_t> boundConnects;

    for (int iel = 0; iel < cmeshHCurl->NElements(); iel++) {
        TPZCompEl *cel = cmeshHCurl->ElementVec()[iel];
        if (cel == NULL) {
            continue;
        }
        if (cel->Reference() == NULL) {
            continue;
        }
        TPZBndCond *mat = dynamic_cast<TPZBndCond *>(cmeshHCurl->MaterialVec()[cel->Reference()->MaterialId()]);
        if (mat && mat->Type() == 0) {
            std::set<int64_t> boundConnectsEl;
            cel->BuildConnectList(boundConnectsEl);

            for (std::set<int64_t>::iterator iT = boundConnectsEl.begin();
                 iT != boundConnectsEl.end(); iT++) {
                const int64_t val = *iT;
                if (boundConnects.find(val) == boundConnects.end()) {
                    boundConnects.insert(val);
                }
            }
        }
    }
    neqOriginal = cmeshHCurl->NEquations();
    for (int iCon = 0; iCon < cmeshHCurl->NConnects(); iCon++) {
        if (boundConnects.find(iCon) == boundConnects.end()) {
            TPZConnect &con = cmeshHCurl->ConnectVec()[iCon];
            int seqnum = con.SequenceNumber();
            int pos = cmeshHCurl->Block().Position(seqnum);
            int blocksize = cmeshHCurl->Block().Size(seqnum);
            if (blocksize == 0)
                continue;

            int vs = activeEquations.size();
            activeEquations.Resize(vs + blocksize);
            for (int ieq = 0; ieq < blocksize; ieq++) {
                activeEquations[vs + ieq] = pos + ieq;
                neq++;
            }
        }
    }
    neqOriginal = cmeshHCurl->NEquations();
    std::cout << "# equations(before): " << neqOriginal << std::endl;
    std::cout << "# equations(after): " << neq << std::endl;
    return;
}

void
CreateCMesh(TPZCompMesh *&cmesh, TPZGeoMesh *gmesh, int pOrder, void (&loadVec)(const TPZVec<REAL> &, TPZVec<STATE> &),
            const REAL &alphaPml, const std::string &prefix, const bool &print,
            const TPZVec<int> &matIdVec, const std::map<int,REAL> &rhoMap,const std::map<int,REAL> &velocityMap,
            const TPZVec< SPZAcousticData::pmltype > &pmlTypeVec,
            const TPZVec<SPZAcousticData::boundtype> &boundTypeVec){

    TPZManVector<int, 8> sourceMatIdVec(1);
    for (int i = 0; i < 1; i++) {
        sourceMatIdVec[i] = matIdVec[i];
    }

    TPZManVector<int, 8> volMatIdVec(matIdVec.size() - boundTypeVec.size() - pmlTypeVec.size() - sourceMatIdVec.size());
    for (int i = 0; i < volMatIdVec.size(); i++) {
        volMatIdVec[i] = matIdVec[i+sourceMatIdVec.size()];
    }

    TPZManVector<int, 8> pmlMatIdVec(pmlTypeVec.size());
    for (int i = 0; i < pmlMatIdVec.size(); i++) {
        pmlMatIdVec[i] = matIdVec[volMatIdVec.size() + sourceMatIdVec.size() + i];
    }
    TPZManVector<int, 8> boundMatIdVec(boundTypeVec.size());
    for (int i = 0; i < boundMatIdVec.size(); i++) {
        boundMatIdVec[i] = matIdVec[volMatIdVec.size() + sourceMatIdVec.size()  + pmlMatIdVec.size() + i];
    }


    const int64_t outerMaterialIndex = volMatIdVec[volMatIdVec.size() - 1];

    const int dim = 2;   // dimensao do problema
    cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDefaultOrder(pOrder); // seta ordem polimonial de aproximacao
    cmesh->SetDimModel(dim);        // seta dimensao do modelo
    // Inserindo material na malha
    TPZMatAcousticsFourier *matAcoustics = nullptr, *matOuter = nullptr;
    REAL rhoOuter=-1, velocityOuter=-1;
    for (int i = 0; i < volMatIdVec.size(); ++i) {
        const REAL rho = rhoMap.at(volMatIdVec[i]);
        const REAL velocity = velocityMap.at(volMatIdVec[i]);
        matAcoustics = new TPZMatAcousticsFourier(volMatIdVec[i], rho, velocity);
        cmesh->InsertMaterialObject(matAcoustics);
        if(volMatIdVec[i] == outerMaterialIndex){
            rhoOuter = rho;
            velocityOuter = velocity;
            matOuter = matAcoustics;
        }
    }

    for (int i = 0; i < sourceMatIdVec.size(); ++i) {
        const REAL rho = rhoMap.at(volMatIdVec[i]);
        const REAL velocity = velocityMap.at(volMatIdVec[i]);
        matAcoustics = new TPZMatAcousticsFourier(sourceMatIdVec[i], rho, velocity);//rho and v wont be used
        cmesh->InsertMaterialObject(matAcoustics);
    }
//    auto exactSol = [](const TPZVec<REAL> &coord, TPZVec<STATE> &result, TPZFMatrix<STATE> &grad) {
//        result.Resize(1, 0.);
//        result[0] = M_PI * cos((1/10.) * 2 * M_PI * coord[0]) * sin((1/10.) * 2 * M_PI  * coord[1]);
//
//        grad.Resize(2, 1);
//        grad(0, 0) = -1 * M_PI * M_PI * sin(M_PI * coord[0]) * sin(M_PI * coord[1]);
//        grad(1, 0) = M_PI * M_PI * cos(M_PI * coord[0]) * cos(M_PI * coord[1]);
//    };
//    matAcoustics->SetExactSol(exactSol);
    //TODO: achar material sobre o qual a PML foi criada. Isso é essencial para casos mais complicados.
    for (int i = 0; i < pmlMatIdVec.size(); i++) {
        TPZMatAcousticsPml *matAcousticsPML = NULL;
        REAL xMax = -1e20, xMin = 1e20, yMax = -1e20, yMin = 1e20, elSizeTol = 1e20;
        TPZGeoEl *leftmostEl=nullptr, *rightmostEl=nullptr, *highestEl=nullptr, *lowestEl=nullptr;
        TPZGeoMesh *gmesh = cmesh->Reference();
        for (int iel = 0; iel < gmesh->NElements(); ++iel) {
            TPZGeoEl *geo = gmesh->Element(iel);
            if (geo->MaterialId() == pmlMatIdVec[i]) {
                const REAL elSize = geo->ElementRadius();
                if(elSize < elSizeTol){
                    elSizeTol = elSize;
                }
                for (int iNode = 0; iNode < geo->NCornerNodes(); ++iNode) {
                    TPZManVector<REAL, 3> co(3);
                    geo->Node(iNode).GetCoordinates(co);
                    const REAL &xP = co[0];
                    const REAL &yP = co[1];
                    if (xP > xMax) {
                        xMax = xP;
                        rightmostEl = geo;
                    }
                    if (xP < xMin) {
                        xMin = xP;
                        leftmostEl = geo;
                    }
                    if (yP > yMax) {
                        yMax = yP;
                        highestEl = geo;
                    }
                    if (yP < yMin) {
                        yMin = yP;
                        lowestEl = geo;
                    }
                }
            }
        }
        bool attx, atty;
        REAL xBegin, yBegin, d;
        switch (pmlTypeVec[i]) {
            case SPZAcousticData::xp:
                attx = true;
                atty = false;
                xBegin = xMin;
                yBegin = -1;
                d = xMax - xMin;
                //TODO: usar leftmostEl para encontrar material vizinho
                break;
            case SPZAcousticData::yp:
                attx = false;
                atty = true;
                xBegin = -1;
                yBegin = yMin;
                d = yMax - yMin;
                //TODO: usar lowestEl para encontrar material vizinho
                break;
            case SPZAcousticData::xm:
                attx = true;
                atty = false;
                xBegin = xMax;
                yBegin = -1;
                d = xMax - xMin;
                //TODO: usar rightmostEl para encontrar material vizinho
                break;
            case SPZAcousticData::ym:
                attx = false;
                atty = true;
                xBegin = -1;
                yBegin = yMax;
                d = yMax - yMin;
                //TODO: usar highestEl para encontrar material vizinho
                break;
            case SPZAcousticData::xpyp:
                attx = true;
                atty = true;
                xBegin = xMin;
                yBegin = yMin;
                d = xMax - xMin;
                //TODO: usar elSizeTol para encontrar material vizinho
                break;
            case SPZAcousticData::xmyp:
                attx = true;
                atty = true;
                xBegin = xMax;
                yBegin = yMin;
                d = xMax - xMin;
                //TODO: usar elSizeTol para encontrar material vizinho
                break;
            case SPZAcousticData::xmym:
                attx = true;
                atty = true;
                xBegin = xMax;
                yBegin = yMax;
                d = xMax - xMin;
                //TODO: usar elSizeTol para encontrar material vizinho
                break;
            case SPZAcousticData::xpym:
                attx = true;
                atty = true;
                xBegin = xMin;
                yBegin = yMax;
                d = xMax - xMin;
                //TODO: usar elSizeTol para encontrar material vizinho
                break;
        }

        matAcousticsPML =
                new TPZMatAcousticsPml(pmlMatIdVec[i], *matOuter, attx, xBegin, atty, yBegin, alphaPml,d);
        cmesh->InsertMaterialObject(matAcousticsPML);
    }

    TPZFMatrix<STATE> val1(1, 1, 0.), val2(1, 1, 0.);
    val1(0, 0) = 0.;
    val2(0, 0) = 0.;
    TPZMaterial *bcond = nullptr;
    for (int i = 0; i < boundMatIdVec.size(); i++) {
        bcond = matAcoustics->CreateBC(matAcoustics, boundMatIdVec[i], boundTypeVec[i], val1, val2);
        cmesh->InsertMaterialObject(bcond);
    }
    cmesh->SetAllCreateFunctionsContinuous();
    cmesh->AutoBuild();
    if(print){
        std::string meshFileName = prefix + "cmesh";
        const size_t strlen = meshFileName.length();
        meshFileName.append(".vtk");
        std::ofstream outVTK(meshFileName.c_str());
        meshFileName.replace(strlen, 4, ".txt");
        std::ofstream outTXT(meshFileName.c_str());
        TPZVTKGeoMesh::PrintCMeshVTK(cmesh,outVTK,true);
        cmesh->Print(outTXT);
        outTXT.close();
        outVTK.close();
    }
}
