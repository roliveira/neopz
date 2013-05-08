/* Generated by Together */

#include "pzadaptmesh.h"
#include "pzgclonemesh.h"
#include "pzcclonemesh.h"
#include "pzgeoel.h"
#include "pzcompel.h"
#include "pzfstrmatrix.h"
#include "pzstepsolver.h"
#include "pzintel.h"
#include "pzquad.h"
#include "pzmaterial.h"
#include "pzonedref.h"

#include "TPZVTKGeoMesh.h"

using namespace std;

TPZAdaptMesh::TPZAdaptMesh(){
    fReferenceCompMesh = 0;
    fGeoRef.Resize(0);
    fPatch.Resize(0);
    fPatchIndex.Resize(0);
    fElementError.Resize(0);
    fCloneMeshes .Resize(0);
    fFineCloneMeshes .Resize(0);
    fMaxP = 10;
}

TPZAdaptMesh::~TPZAdaptMesh(){
    CleanUp();
}

void TPZAdaptMesh::SetCompMesh(TPZCompMesh * mesh){
    if(!mesh){
        cout <<"TPZAdaptMesh::Error:\n computational reference mesh must not be NULL!\n";
        return;
    }
    CleanUp();
    fReferenceCompMesh = mesh;
    int nel = fReferenceCompMesh->ElementVec().NElements();
    fElementError.Resize(nel);
    fElementError.Fill(0.);
}

void TPZAdaptMesh::SetMaxP(int maxp){
    if (maxp < 1) {
        cout << "TPZAdaptMesh::Error : SetMaxP - maximum p order must be greter than 0... trying to set maximum p to " 
        << maxp << endl;
        return;
    }
    fMaxP = maxp;
    TPZOneDRef::gMaxP = fMaxP;
}

void TPZAdaptMesh::CleanUp(){
    int i;
    for (i=0;i<fCloneMeshes .NElements();i++){
        TPZGeoCloneMesh *gmesh = dynamic_cast<TPZGeoCloneMesh *> (fCloneMeshes [i]->Reference());
        gmesh->ResetReference();
        //Cesar July 2003 ->
        //If some reference element is not used to analyse error its fine clone mesh is not created!
        if (fFineCloneMeshes [i]){
            fFineCloneMeshes [i]->LoadReferences();
            RemoveCloneBC(fFineCloneMeshes [i]);
            DeleteElements(fFineCloneMeshes [i]);
            delete fFineCloneMeshes [i];
        }
        //<-
        fCloneMeshes [i]->LoadReferences();
        RemoveCloneBC(fCloneMeshes [i]);
        DeleteElements(fCloneMeshes [i]);
        delete fCloneMeshes [i];
        delete gmesh;
    }
    fCloneMeshes .Resize(0);
    fFineCloneMeshes .Resize(0);
}

TPZCompMesh * TPZAdaptMesh::GetAdaptedMesh(REAL &error, REAL & truerror, TPZVec<REAL> &ervec, 
                                           void(*f)(const TPZVec<REAL> &loc, TPZVec<REAL> &val, TPZFMatrix<REAL> &deriv),
                                           TPZVec<REAL> &truervec, 
                                           TPZVec<REAL> &effect, int use_trueerror){
    int i;
    //clone analysis
    int cliter;
    int nelmesh = fReferenceCompMesh->ElementVec().NElements();
    fElementError.Resize(nelmesh);
    effect.Resize(nelmesh);
    truervec.Resize(nelmesh);
    ervec.Resize(nelmesh);
    ervec.Fill(0.);
    truervec.Fill(0.);
    effect.Fill(0.);
    fElementError.Fill(0);
    
    for (cliter=0;cliter<nelmesh;cliter++)  fElementError[cliter] = 0.;
    
    //Used to evaluate the error when the true solution is provided======
    TPZVec<int> perm(nelmesh,0);
    TPZVec<REAL> auxerrorvec(nelmesh,0.);
    REAL minerror = 0.;
    //===================================================================
    
    //gets the geometric reference elements that will generate the patch
    GetReferenceElements();
    
    if (use_trueerror && f){
        for (i=0;i<fReferenceCompMesh->ElementVec().NElements();i++){
            TPZInterpolatedElement *el = dynamic_cast<TPZInterpolatedElement *> (fReferenceCompMesh->ElementVec()[i]);
            if (el) 
                auxerrorvec[i] = UseTrueError (el , f);
            else
                auxerrorvec[i] = 0.;
        }
        minerror = SortMinError(auxerrorvec,perm,0.65);
    } 
    
    //Generates the patch - put the data into fPatch and fPatchIndexes
    BuildReferencePatch();
    
    //Creates the patch clones; fReferenceCompMesh is the original computational mesh
	
	// First, asserting connects in computational mesh
    fReferenceCompMesh->ComputeNodElCon();
    
    int printing = 0;
    if(printing) {
        ofstream test("test.txt",ios::app);
        fReferenceCompMesh->Print(test);
        fReferenceCompMesh->Reference()->Print(test);
    }
    
    // Create the clone meshes and put the data in fCloneMeshes
    // Each element of the vector contains a computational mesh
    CreateClones();
    int ncl = fCloneMeshes .NElements();
    fFineCloneMeshes .Resize(ncl);
    
    //Creates an uniformly refined mesh and evaluates the error
    for (cliter = 0; cliter<ncl; cliter++){
        //An�lise dos Clones
        TPZGeoCloneMesh *gcmesh = dynamic_cast<TPZGeoCloneMesh *> (fCloneMeshes [cliter]->Reference());
        //       if (gcmesh && gcmesh->GetMeshRootElement()->MaterialId() < 0) continue;
        if (gcmesh->ReferenceElement(0)->MaterialId() <  0 || (use_trueerror && f && !HasTrueError(cliter,minerror,auxerrorvec))){
            fFineCloneMeshes [cliter] = 0;
            continue;
        }
        fFineCloneMeshes[cliter] = fCloneMeshes[cliter]->UniformlyRefineMesh();
		printing = 0;
        if(printing) {
            ofstream out("output.txt");
            fCloneMeshes[cliter]->Print(out);
			fFineCloneMeshes[cliter]->Print(out);
            out.close();
        }
        fCloneMeshes[cliter]->MeshError(fFineCloneMeshes [cliter],fElementError,f,truervec);
    }
    
    //Ordena o vetor de erros
    
    for(i=0; i<nelmesh; i++) {
        perm[i] = i;
        ervec[i]=fElementError[i];
    }
    Sort(fElementError,perm);
    //somat�rio dos componentes do vetor de erro
    for(i=0; i<nelmesh; i++) error += fElementError[i];
    
    REAL ninetyfivepercent,auxerror = 0.;
    for(i=0;i<nelmesh;i++){
        auxerror += fElementError[perm[i]];
        if (auxerror >= 0.65*error){
            ninetyfivepercent = fElementError[perm[i]];
            break;
        }
    }
    
    if(f) {
        for(i=0; i<nelmesh; i++){
            truerror += truervec[i];
        }
    }
    
    //inicializa effect com o tamanho de trueeerror
    effect.Resize(truervec.NElements());
    effect.Fill(0.);
    if(f) {
        for(i=0; i<nelmesh; i++) {
            if(truervec[i] >= 1.e-4*truerror && truervec[i] >= 5e-20 ) {
                effect[i] = ervec[i]/truervec[i];
            }
            else {
                effect[i]=0.;
                //truervec[i]=0.;
            }
        }
    }
    
    TPZStack <TPZGeoEl*> gelstack;
    TPZStack <int> porder;
    
    //Analyse clone element error and, if necessary, analyse element and changes its refinement pattern
    for (i=0;i<fCloneMeshes .NElements();i++){
        if (! fFineCloneMeshes [i]) continue;
        fCloneMeshes [i]->ApplyRefPattern(ninetyfivepercent,fElementError,fFineCloneMeshes [i],gelstack,porder);
    }
    
    TPZCompMesh * adapted =   CreateCompMesh(fReferenceCompMesh,gelstack,porder);
    return adapted;
}

void TPZAdaptMesh::GetReferenceElements(){
    if (!fReferenceCompMesh){
        cout << "TPZAdaptMesh::Error:\n computational mesh must be initialized to call GetReferenceElements!\n";
        return;
    }
    std::set<TPZGeoEl*> georef;
    //  fReferenceCompMesh->GetRefPatches(fGeoRef);
    fReferenceCompMesh->GetRefPatches(georef);
    
#ifndef CLONEBCTOO
    //This will exclude geometric elements associated to bc from clone creation
    std::set<TPZGeoEl *>::iterator it;
    for (it=georef.begin();it!=georef.end(); it++){ 
        int id = (*it)->MaterialId();
        if (id > 0){
            fGeoRef.Push(*it);
        }
    }
#else
    std::set<TPZGeoEl *>::iterator it;
    for (it=georef.begin();it!=georef.end(); it++){ 
        fGeoRef.Push(*it);
    }
#endif
}

void TPZAdaptMesh::BuildReferencePatch() {
    
    // the fGeoRef elements are a partition of the computational domain (should be)
    // create a computational element based on each reference element
    TPZGeoMesh *gmesh = fReferenceCompMesh->Reference();
    gmesh->ResetReference();
    TPZCompMesh *tmpcmesh = new TPZCompMesh (gmesh);
    int i,j;
    for (i=0;i<fGeoRef.NElements();i++){
        int index;
        tmpcmesh->CreateCompEl(fGeoRef[i],index);
    } 
    tmpcmesh->CleanUpUnconnectedNodes();
	tmpcmesh->ExpandSolution();
    TPZStack <int> patchelindex;
    TPZStack <TPZGeoEl *> toclonegel;
    TPZStack<int> elgraph;
    TPZVec<int> n2elgraph;
    TPZVec<int> n2elgraphid;
    TPZVec<int> elgraphindex;

    tmpcmesh->GetNodeToElGraph(n2elgraph,n2elgraphid,elgraph,elgraphindex);
    // we use the  node to elgraph structure to decide which elements will be included
    int clnel = tmpcmesh->NElements();
    // clnel corresponds to the number of patches
    // fPatch and fPatchIndex form a compacted list which form the patches.
    // Boundary elements will be added to each patch.
    fPatchIndex.Push(0);
    for (int ipatch=0; ipatch<clnel; ipatch++){
        tmpcmesh->GetElementPatch(n2elgraph,n2elgraphid,elgraph,elgraphindex,ipatch,patchelindex);
        for (j=0; j<patchelindex.NElements(); j++){
            TPZGeoEl *gel = tmpcmesh->ElementVec()[patchelindex[j]]->Reference();
            //      int count = 0;
            if(gel) fPatch.Push(gel);
        }
        int sum = fPatch.NElements();
        fPatchIndex.Push(sum);
    }
	
#ifdef DEBUG2 
	// CAJU TOOL
	{
		std::string filename("cMeshVtk.");
		{
			std::stringstream finalname;
			finalname << filename << 0 << ".vtk";
			ofstream file(finalname.str().c_str());
			/** @brief Generate an output of all geometric elements that have a computational counterpart to VTK */
			//static void PrintCMeshVTK(TPZGeoMesh *gmesh, std::ofstream &file, bool matColor = false);
			TPZVTKGeoMesh::PrintCMeshVTK(gmesh,file,true);
		}
		for (int ip=0; ip<clnel; ip++) {
			int firstindex = fPatchIndex[ip];
			int lastindex = fPatchIndex[ip+1];
			gmesh->ResetReference();
			tmpcmesh->LoadReferences();
			std::set<TPZGeoEl *> loaded;
			for (int ind=firstindex; ind<lastindex; ind++) {
				TPZGeoEl *gel = fPatch[ind];
				loaded.insert(gel);
			}
			int ngel = gmesh->NElements();
			for (int el=0; el<ngel; el++) {
				TPZGeoEl *gel = gmesh->ElementVec()[el];
				if (!gel) {
					continue;
				}
				if (gel->Reference() && loaded.find(gel) == loaded.end()) {
					gel->ResetReference();
				}
			}
			std::stringstream finalname;
			finalname << filename << ip+1 << ".vtk";
			ofstream file(finalname.str().c_str());
			/** @brief Generate an output of all geometric elements that have a computational counterpart to VTK */
			//static void PrintCMeshVTK(TPZGeoMesh *gmesh, std::ofstream &file, bool matColor = false);
			TPZVTKGeoMesh::PrintCMeshVTK(gmesh,file,true);
			
		}
	}
#endif
	// cleaning reference to computational elements into temporary cmesh
    gmesh->ResetReference();
    delete tmpcmesh;
	// loading references between geometric and computational meshes (originals)
    fReferenceCompMesh->LoadReferences();
}

void TPZAdaptMesh::CreateClones(){
	// asserting references of the original meshes
    fReferenceCompMesh->Reference()->ResetReference();
    fReferenceCompMesh->LoadReferences();
    TPZGeoMesh *geomesh = fReferenceCompMesh->Reference();
    
    TPZStack<TPZGeoEl*> patch;
    
    int clid,elid;
    for (clid=0; clid<fPatchIndex.NElements()-1;clid++) {
		// making clone of the original geometric mesh, only to construct computational clone
        TPZGeoCloneMesh *geoclone = new TPZGeoCloneMesh(geomesh);
        TPZStack<TPZGeoEl*> patch;
        for (elid=fPatchIndex[clid];elid<fPatchIndex[clid+1];elid++){
            patch.Push(fPatch[elid]);
        }
        geoclone->SetElements(patch,fGeoRef[clid]);
        TPZVec<TPZGeoEl *> sub;
        //    int ngcel = geoclone->ElementVec().NElements();
        int printing = 1;
        if(printing) {
            ofstream out("test.txt",ios::app);
            geoclone->Print(out);
        }
        
        TPZCompCloneMesh *clonecompmesh = new TPZCompCloneMesh(geoclone,fReferenceCompMesh);
        clonecompmesh->AutoBuild();
		// Computational mesh clone is stored
        fCloneMeshes.Push(clonecompmesh);    
    }
}

void TPZAdaptMesh::Sort(TPZVec<REAL> &vec, TPZVec<int> &perm) {
    int i,j;
    int imin = 0;
    int imax = vec.NElements();
    for(i=imin; i<imax; i++) {
        for(j=i+1; j<imax; j++) {
            if(vec[perm[i]] < vec[perm[j]]) {
                int kp = perm[i];
                perm[i] = perm[j];
                perm[j] = kp;
            }
        }
    }
}

void TPZAdaptMesh::HeapSort(TPZVec<REAL> &sol, TPZVec<int> &perm){
    
    int nelem = perm.NElements();
    int i,j;
    for(i=0; i<nelem; i++) perm[i] = i;
    
    if(nelem == 1) return;
    int l, ir,ind;
    REAL q;
    l= nelem/2;
    ir = nelem-1;
    while(l>0 && ir>0) {
        if(l> 0) {
            l--;
            ind = perm[l];
            q=sol[ind];
        } else {
            ind = perm[ir];
            q = sol[ind];
            perm[ir] = perm[0];
            ir--;
        }
        i=l;
        j=l+l+1;
        while(j<=ir) {
            if(j<ir && sol[perm[j]] < sol[perm[j+1]]) j++;
            if(q < sol[perm[j]]) {
                perm[i] = perm[j];
                i=j;
                j= i+i+1;
            } else {
                break;
            }
        }
        perm[i] = ind;
    }
}

TPZCompMesh *TPZAdaptMesh::CreateCompMesh (TPZCompMesh *mesh,                                          //malha a refinar
                                           TPZVec<TPZGeoEl *> &gelstack,   //
                                           TPZVec<int> &porders) {
    
    //Cria um ponteiro para a malha geom�trica de mesh
    TPZGeoMesh *gmesh = mesh->Reference();
    if(!gmesh) {
        cout << "TPZAdaptMesh::CreateCompMesh encountered no geometric mesh\n";
        return 0;
    }
    
    //Reseta as refer�ncias do ponteiro para a malha geom�trica criada
    //e cria uma nova malha computacional baseada nesta malha geom�trica
    gmesh->ResetReference();
    TPZCompMesh *cmesh = new TPZCompMesh(gmesh);
//    int nmat = mesh->MaterialVec().size();
  //  int m;
    
    //Cria um clone do vetor de materiais da malha mesh
    mesh->CopyMaterials(*cmesh);
    /*  for(m=0; m<nmat; m++) {
     TPZMaterial * mat = mesh->MaterialVec()[m];
     if(!mat) continue;
     mat->Clone(cmesh->MaterialVec());
     }
     */
    //Idenifica o vetor de elementos computacionais de mesh
    //  TPZAdmChunkVector<TPZCompEl *> &elementvec = mesh->ElementVec();
    
    int el,nelem = gelstack.NElements();
    //  cmesh->SetName("Antes PRefine");
    //  cmesh->Print(cout);
    for(el=0; el<nelem; el++) {
        
        //identifica os elementos geom�tricos passados em gelstack
        TPZGeoEl *gel = gelstack[el];
        if(!gel) {
            cout << "TPZAdaptMesh::CreateCompMesh encountered an null element\n";
            continue;
        }
        int celindex;
        
        //Cria um TPZIntel baseado no gel identificado
        TPZInterpolatedElement *csint;
        csint = dynamic_cast<TPZInterpolatedElement *> (cmesh->CreateCompEl(gel,celindex));
        if(!csint) continue;
        
        //Refina em p o elemento criado
        //	cmesh->SetName("depois criar elemento");
        //	cmesh->Print(cout);
        
        csint->PRefine(porders[el]);
        //	cmesh->SetName("depois prefine no elemento");
        //	cmesh->Print(cout);
    }
#ifndef CLONEBCTOO
    nelem = gmesh->NElements();
    for (el=0; el<nelem; el++) {
        TPZGeoEl *gel = gmesh->ElementVec()[el];
        if (!gel || gel->Reference()) {
            continue;
        }
        int matid = gel->MaterialId();
        if (matid < 0) {
            TPZStack<TPZCompElSide> celstack;
            int ns = gel->NSides();
            TPZGeoElSide gelside(gel,ns-1);
            gelside.HigherLevelCompElementList2(celstack, 1, 1);
            if (celstack.size()) {
                TPZStack<TPZGeoEl *> subels;
                gel->Divide(subels);
            }
        }
    }
    nelem = gmesh->NElements();
    for (el=0; el<nelem; el++) {
        TPZGeoEl *gel = gmesh->ElementVec()[el];
        if (!gel || gel->Reference()) {
            continue;
        }
        int matid = gel->MaterialId();
        if (matid < 0) {
            TPZStack<TPZCompElSide> celstack;
            int ns = gel->NSides();
            TPZGeoElSide gelside(gel,ns-1);
            gelside.EqualLevelCompElementList(celstack, 1, 0);
            if (celstack.size()) {
                int index;
                cmesh->CreateCompEl(gel, index);
            }
        }
    }
#endif
    //Mais einh!!
    //	cmesh->SetName("Antes Adjust");
    //	cmesh->Print(cout);
    cmesh->AdjustBoundaryElements();
    //  cmesh->SetName("Depois");
    //  cmesh->Print(cout);
    return cmesh;
    
}

void TPZAdaptMesh::RemoveCloneBC(TPZCompMesh *mesh)
{
    int nelem = mesh->NElements();
    int iel;
    for(iel=0; iel<nelem; iel++) {
        TPZCompEl *cel = mesh->ElementVec()[iel];
        if(!cel) continue;
        int matid = cel->Material()->Id();
        if(matid == -1000) delete cel;
    }
}

void TPZAdaptMesh::DeleteElements(TPZCompMesh *mesh)
{
    int nelem = mesh->NElements();
    int iel;
    for(iel=0; iel<nelem; iel++) {
        TPZCompEl *cel = mesh->ElementVec()[iel];
        if(!cel) continue;
        TPZInterpolatedElement *cint = dynamic_cast<TPZInterpolatedElement *> (cel);
        if(!cint) continue;
        while(cint->HasDependency()) {
            TPZInterpolatedElement *large = LargeElement(cint);
            TPZInterpolatedElement *nextlarge = LargeElement(large);
            while(nextlarge != large) {
                large = nextlarge;
                nextlarge = LargeElement(large);
            }
            large->RemoveSideRestraintsII(TPZInterpolatedElement::EDelete);
            delete large;
        }
        cint->RemoveSideRestraintsII(TPZInterpolatedElement::EDelete);
        delete cint;
    }
}

TPZInterpolatedElement * TPZAdaptMesh::LargeElement(TPZInterpolatedElement *cint)
{
    int nc = cint->NConnects();
    int side;
    TPZInterpolatedElement *result = cint;
    for(side=0; side<nc; side++) {
        if(cint->Connect(side).HasDependency()) {
            TPZCompElSide cintside(cint,side);
            TPZCompElSide large = cintside.LowerLevelElementList(1);
            if(!large.Exists()) {
                cout << "TPZAdaptMesh::DeleteElements I dont understand\n";
                large = cintside.LowerLevelElementList(1);
				return cint;
            }
            result = dynamic_cast<TPZInterpolatedElement *> (large.Element());
            break;
        }
    }
    return result;
}


REAL TPZAdaptMesh::UseTrueError(TPZInterpolatedElement *coarse, 
                                void (*f)(const TPZVec<REAL> &loc, TPZVec<REAL> &val, TPZFMatrix<REAL> &deriv)){
    if (coarse->Material()->Id() < 0) return 0.0;
    
    REAL error = 0.;
    
    //  REAL loclocmatstore[500] = {0.},loccormatstore[500] = {0.};
    // TPZFMatrix loccormat(locmatsize,cormatsize,loccormatstore,500);
    
    //Cesar 25/06/03 - Uso a ordem m�xima???
    TPZAutoPointer<TPZIntPoints> intrule = coarse->GetIntegrationRule().Clone();
    
    int dimension = coarse->Dimension();
    int numdof = coarse->Material()->NStateVariables();
        
    //TPZSolVec corsol;
    //TPZGradSolVec cordsol;
    TPZGradSolVec cordsolxy;
//    TPZVec<REAL> corsol(numdof);
//    TPZFNMatrix<9,REAL> cordsol(dimension,numdof),cordsolxy(dimension,numdof);
    
    TPZManVector<int> order(dimension,20);
    intrule->SetOrder(order);
    
    // derivative of the shape function
    // in the master domain
    TPZManVector<REAL,3> coarse_int_point(dimension);
//    TPZFNMatrix<9,REAL> jaccoarse(dimension,dimension),jacinvcoarse(dimension,dimension);
//    TPZFNMatrix<9,REAL> axescoarse(3,3);
//    TPZManVector<REAL,3> xcoarse(3);
    TPZFNMatrix<9,REAL> axesinner(3,3);
    
    
    TPZMaterialData datacoarse;
    coarse->InitMaterialData(datacoarse);
    
//    REAL jacdetcoarse;
    int numintpoints = intrule->NPoints();
    REAL weight;
    
    TPZVec<REAL> truesol(numdof);
    TPZFMatrix<REAL> truedsol(dimension,numdof);
    for(int int_ind = 0; int_ind < numintpoints; ++int_ind) {
        intrule->Point(int_ind,coarse_int_point,weight);
        //coarse->Reference()->X(coarse_int_point, xcoarse);
        coarse->Reference()->X(coarse_int_point, datacoarse.x);
        //if(f) f(xcoarse,truesol,truedsol);
        if(f) f(datacoarse.x,truesol,truedsol);

//        coarse->Reference()->Jacobian(coarse_int_point, jaccoarse, axescoarse, jacdetcoarse, jacinvcoarse);
        coarse->Reference()->Jacobian(coarse_int_point, datacoarse.jacobian, datacoarse.axes, datacoarse.detjac, datacoarse.jacinv);
        //weight *= fabs(jacdetcoarse);
        weight *= fabs(datacoarse.detjac);
//Er        int iv=0;
//        corsol[0].Fill(0.);
//        cordsol[0].Zero();

        //coarse->ComputeSolution(coarse_int_point, corsol, cordsol, axescoarse);
        coarse->ComputeShape(coarse_int_point,datacoarse);
        coarse->ComputeSolution(coarse_int_point,datacoarse);
        
        //int nc = cordsol[0].Cols();
        int nc = datacoarse.dsol[0].Cols();
        for (int col=0; col<nc; col++)
        {
            for (int d=0; d<dimension; d++) {
                REAL deriv = 0.;
                for (int d2=0; d2<dimension; d2++) {
                    deriv += datacoarse.dsol[0](d2,col)*datacoarse.axes(d2,d);
                }
               // cordsolxy[0](d,col) = deriv;
            }
        }
        int jn;
        for(jn=0; jn<numdof; jn++) {
            for(int d=0; d<dimension; d++) {
                error += (datacoarse.dsol[0](d,jn)-truedsol(d,jn))*(datacoarse.dsol[0](d,jn)-truedsol(d,jn))*weight;
            }
        }
    }
    return error;
}


REAL TPZAdaptMesh::SortMinError (TPZVec<REAL> errvec, TPZVec<int> perm, REAL percenterror){
    //Ordena o vetor de erros
    int nelem = errvec.NElements();
    int i;
    REAL error;
    for(i=0; i<nelem; i++) {
        perm[i] = i;
        //ervec[i]=fElementError[i];
    }
    Sort(fElementError,perm);
    //somat�rio dos componentes do vetor de erro
    for(i=0; i<nelem; i++) error += fElementError[i];
    REAL ninetyfivepercent,auxerror = 0.;
    for(i=0;i<nelem;i++){
        auxerror += fElementError[perm[i]];
        if (auxerror >=  percenterror*error){
            ninetyfivepercent = fElementError[perm[i]];
            break;
        }
    }
    return ninetyfivepercent;
}

int TPZAdaptMesh::HasTrueError(int clindex, REAL &minerror, TPZVec<REAL> &ervec){
    
    TPZGeoCloneMesh *gmesh = dynamic_cast<TPZGeoCloneMesh *> (fCloneMeshes [clindex]->Reference());
    int nref = gmesh->NReference();
    int i;
    for (i=0;i<nref;i++){
        TPZGeoEl *gel = gmesh->ReferenceElement(i);
        TPZCompEl *cel = gel->Reference();
        if (!cel) continue;
        int celindex = cel->Index();
        if (ervec[celindex] >= minerror) return 1;
    }
    return 0;
}
