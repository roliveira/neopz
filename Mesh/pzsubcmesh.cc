//$Id: pzsubcmesh.cc,v 1.6 2003-11-05 16:02:21 tiago Exp $

// subcmesh.cpp: implementation of the TPZSubCompMesh class.
//
//////////////////////////////////////////////////////////////////////

#include "pzsubcmesh.h"
#include "pzgmesh.h"
#include "pzcompel.h"
#include "pzcmesh.h"
#include "pzelgc3d.h"
#include "pzelmat.h"
#include "pzmathyperelastic.h"
#include "pznonlinanalysis.h"
#include "pzskylmat.h"
#include "pzsolve.h"
#include "pzstepsolver.h"
#include "pzskylstrmatrix.h"
#include "pzfstrmatrix.h"
#include "TPZFrontStructMatrix.h"
#include "pzsmfrontalanal.h"

#include <stdio.h>


const int numel=1;

static REAL angle = 0.2;
void Forcing(TPZVec<REAL> &x, TPZVec<REAL> &disp);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
int TPZSubCompMesh::main() {
//	int index;
	
	//Create the Geometric Mesh	
	TPZGeoMesh geo;

	//Define the output file name
	ofstream output("output.dat");\

	//Set the basic coordinate nodes
	double coordstore[4][2] = {{0.,0.},{1.,0.},{1.,1.},{0.,1.}};
	
	TPZVec<REAL> coord(3,0.); 
	int i,j;

	//Set the node coordinates
	for(i=0; i<4*(numel+1); i++) {
		for (j=0; j<2; j++) {
			coord[j] = coordstore[i%4][j];
			coord[2] = i/4;
		}
		//   	int nodeindex = geo.NodeVec().AllocateNewElement();
	geo.NodeVec()[i].Initialize(i,coord,geo);
	}
	
	// create the elements
	TPZGeoEl *gel[numel];
	TPZVec<int> indices(8);

	// Set the connectivities
	for(i=0; i<numel; i++) {
		// initialize node indexes
		for(j=0; j<8; j++) indices[j] = 4*i+j;
		gel[i] = new TPZGeoElC3d(i,indices,1,geo);
	}
	//	TPZGeoElBC t3(gel[0],20,-1,geo); 	
	//	TPZGeoElBC t4(gel[numel-1],25,-2,geo); 
	geo.BuildConnectivity();

	//Create the computacional mesh
	TPZCompMesh mesh(&geo);

	// Insert the materials
	TPZMaterial *meumat = new TPZMatHyperElastic(1,1.e5,0.25);
	mesh.InsertMaterialObject(meumat);

	int numeq;
	TPZVec<int> skyline;

	// Insert the boundary conditions
	TPZFMatrix val1(3,3,0.),val2(3,1,0.);
	TPZMaterial *bnd = meumat->CreateBC (-1,0,val1,val2);
	mesh.InsertMaterialObject(bnd);
	bnd = meumat->CreateBC (-2,0,val1,val2);
	bnd->SetForcingFunction(Forcing);
	mesh.InsertMaterialObject(bnd);

	mesh.AutoBuild();
	mesh.InitializeBlock();

	
	numeq = mesh.NEquations();

	// Teste 1 colocar os elementos, inclusive intermedi�rios, como sub elementos
	TPZSubCompMesh *sub[numel];

	int index = -1;
	for (i=0;i<numel;i++){
		sub[i] = new TPZSubCompMesh(mesh,index);
	}

	//Teste 2 - Passar todos os sub elementos para os subelementos

	for (i=0;i<numel;i++){
		sub[i]->TransferElement(&mesh,i);
	}

	for (i=0;i<numel;i++){
		sub[i]->MakeAllInternal();
//		sub[i]->Prints(output);
	}

//	mesh.ComputeNodElCon();
//	mesh.Print(output);
	TPZNonLinearAnalysis an(&mesh,output);


//	mesh.Print(output);
	output.flush();
//	TPZFMatrix *rhs = new TPZFMatrix(skyline);
	TPZSkylineStructMatrix strskyl(&mesh);
	an.SetStructuralMatrix(strskyl);
	an.Solution().Zero();
	TPZStepSolver sol;
//	sol.ShareMatrix(an.Solver());
	sol.SetDirect(ELDLt);
	an.SetSolver(sol);
	//	an.Solver().SetDirect(ELDLt);
	an.IterativeProcess(output,0.00001,5);

	//mesh.Print(output);
	sub[0]->LoadSolution();
	sub[0]->SetName("sub[0]");
	sub[0]->Prints(output);
	output.flush();

	
//	int a=1;
	/*int m1, m2;
	while (a != 0){
		cout << "mesh 1 e 2\n";
		cin >> m1 >> m2;

		for (int i=0; i<11; i++){
			if (&mesh == sub[m1]->CommonMesh(sub[m2])){
				cout << 10 << "\n";
				break;
			}
			if (sub[i] == sub[m1]->CommonMesh(sub[m2])){
				cout << i << "\n";
				break;
			}
			else {
				cout << sub[i] << "\t" << sub[m1]->CommonMesh(sub[m2]) << "\n";
			}
		}
		cout << "Digite 0 para sair \n";
		cin >> a;
	}  */
//	sub[4]->TransferElementFrom(&mesh,0);//gel[0]->Index());
//	sub[5]->TransferElementFrom(sub[4],1);

//	sub[5]->TransferElementFrom(&mesh,0);

//	sub[5]->TransferElementTo(sub[4],0);//gel[0]->Index());
//	sub[0]->TransferElementFrom(&mesh,1);//gel[1]->Index());
//	sub[1]->TransferElementFrom(&mesh,1);//gel[1]->Index());
//	sub[2]->TransferElementFrom(&mesh,1);//gel[1]->Index());
//	sub[2]->TransferElementTo(sub[8],0);
/*	sub[8]->TransferElementFrom(&mesh,0);
	sub[8]->TransferElementTo(&mesh,0);
	sub[4]->TransferElementFrom(&mesh,0);
	sub[4]->TransferElementTo(&mesh,1);
*/

	//sub[2]->TransferElementTo(sub[4],0);//gel[0]->Index());
	
	/*TPZVec<int> indices(3,0);
	int nodeindex = geo.NodeVec().AllocateNewElement();
	geo.NodeVec().Initialize(nodeindex,indices,sub[9]);
	sub[9]->NodeIndex(nodeindex,sub[2]);

	TPZFMatrix dep(2,2,2.);
	int is1 = sub[9]->AllocateNewConnect();
	int is2 = sub[9]->AllocateNewConnect();
	int is3 = sub[9]->AllocateNewConnect();
	(sub[9]->fConnectVec[is1]).AddDependency(is1,is2,dep,0,0,2,2);
	sub[9]->AllocateNewConnect();
	int lastcreated = sub[9]->AllocateNewConnect();
	sub[9]->MakeExternal(lastcreated);
	sub[9]->MakeExternal(is1);
	sub[9]->NodeIndex(lastcreated,sub[2]);
	sub[2]->NodeIndex(sub[2]->AllocateNewConnect(),sub[9]);	*/

/*	sub[0]->Prints(output);
	output.flush();
	sub[1]->Prints(output);
	output.flush();
	sub[2]->Prints(output);
	output.flush();
	sub[3]->Prints(output);
	output.flush();
	sub[4]->Prints(output);
	output.flush();
	sub[5]->Prints(output);
	output.flush();
	sub[6]->Prints(output);
	output.flush();
	sub[7]->Prints(output);
	output.flush();
	sub[8]->Prints(output);
	output.flush();
	sub[9]->Prints(output);
	output.flush();
	//sub[10]->Prints(out);
	//out.flush();  */


	return 0;
}



TPZSubCompMesh::TPZSubCompMesh(TPZCompMesh &mesh, int &index) : TPZCompMesh(mesh.Reference()), TPZCompEl(mesh,index)  {

	fAnalysis = NULL;

}


TPZSubCompMesh::~TPZSubCompMesh(){
}


TPZCompMesh * TPZSubCompMesh::FatherMesh(){
	return Mesh();
}


TPZCompMesh * TPZSubCompMesh::CommonMesh(TPZCompMesh *mesh){

	TPZStack<TPZCompMesh *> s1, s2;
	int pos1=0, pos2, comind;
	TPZCompMesh *father = FatherMesh();
	s1.Push(this);
	while (father){
		s1.Push((father));
		pos1++;
		father = s1[pos1]->FatherMesh();
	}
	pos2 = 0;
	s2.Push(mesh);
	father = mesh->FatherMesh();
	while (father){
		s2.Push(father);
		pos2++;
		father = s2[pos2]->FatherMesh();
	}
	if (s1[pos1] != s2[pos2]) return 0;
	comind=0; //The first mesh is common for all submeshes
	for (; pos1>=0 && pos2>=0; pos1--, pos2--) {
		if((s1[pos1])!=(s2[pos2])) {
			comind=pos1+1;
			return (s1[comind]);
		}
	}
	return (pos1 >=0 ) ? (s1[pos1+1]) : s2[pos2+1];
}

int TPZSubCompMesh::NConnects(){
	return fConnectIndex.NElements();
}

int TPZSubCompMesh::ConnectIndex(int i){
	return fConnectIndex[i];
}

int TPZSubCompMesh::Dimension() const {
	return -1;
}

TPZMaterial *TPZSubCompMesh::Material() const{
	return 0;
}

void TPZSubCompMesh::SetMaterial(TPZMaterial *mat){
}

int TPZSubCompMesh::NodeIndex(int nolocal, TPZCompMesh *neighbour){
	TPZCompMesh *root = CommonMesh(neighbour);
	int rootindex = PutinSuperMesh(nolocal,root);
	return neighbour->GetFromSuperMesh(rootindex,root);
}

int TPZSubCompMesh::AllocateNewConnect(int blocksize){
/*	int connectindex = fConnectIndex.AllocateNewElement();
	int blocknum = fBlock.NBlocks();
	fBlock.SetNBlocks(blocknum+1);
	fConnectVec[connectindex].SetSequenceNumber(blocknum);
	return connectindex; 
	*/

	int connectindex = TPZCompMesh::AllocateNewConnect();
	int seqnum = fConnectVec[connectindex].SequenceNumber();
	fBlock.Set(seqnum,blocksize);
	int i,oldsize = fExternalLocIndex.NElements();

	if(oldsize <= connectindex) {
		fExternalLocIndex.Resize(connectindex+1);
		for(i=oldsize; i<=connectindex;i++) fExternalLocIndex[i] = -1;
	} else {
		fExternalLocIndex[connectindex] = -1;
	}
/*
	int nodeindex;

	if (fConnectIndex.NElements()<connectindex){
		fConnectIndex.Resize(fConnectIndex.NElements()+1);
		fExternalLocIndex.Resize(fExternalLocIndex.NElements()+1);
		fExternalLocIndex[fExternalLocIndex.NElements()-1]=-1;
		nodeindex = NodeIndex (connectindex, 
		MakeExternal(connectindex);
	}
*/
return connectindex;
}


void TPZSubCompMesh::MakeExternal(int local){
	if(fExternalLocIndex[local] == -1) {
	//Allocate the dependent nodes of the selected local node in father mesh
		int extconnect;
		int lastext = fConnectIndex.NElements();
		fConnectIndex.Resize(lastext+1);
	//Allocate the selected local node in father mesh
		int blocksize = fConnectVec[local].NDof(*(TPZCompMesh *)this);
		extconnect = FatherMesh()->AllocateNewConnect(blocksize);

//		int lastext = fConnectIndex.NElements();
//		fConnectIndex.Resize(lastext+1);

		fConnectIndex[lastext] = extconnect;
		fExternalLocIndex[local] = lastext;
		TPZConnect::TPZDepend *listdepend = fConnectVec[local].FirstDepend();
		while(listdepend) {
			int depindex = listdepend->fDepConnectIndex;
			MakeExternal(listdepend->fDepConnectIndex);
			int depextind = fConnectIndex[fExternalLocIndex[depindex]];
			int r = listdepend->fDepMatrix.Rows();
			int c = listdepend->fDepMatrix.Cols();
			FatherMesh()->ConnectVec()[extconnect].AddDependency(extconnect,depextind,listdepend->fDepMatrix,0,0,r,c);
			fConnectVec[local].RemoveDepend(local,depindex);
			listdepend = fConnectVec[local].FirstDepend();
		}
	} else {
		if(fConnectVec[local].FirstDepend() ) {
			cout << "TPZSubCompMesh iconsistent data structure !";
		}
	}
}

int TPZSubCompMesh::PutinSuperMesh(int local, TPZCompMesh *super){
	if(super == this) return local;
	if(fExternalLocIndex[local] == -1) MakeExternal(local);
	return FatherMesh()->PutinSuperMesh(fConnectIndex[fExternalLocIndex[local]],super);
}

int TPZSubCompMesh::GetFromSuperMesh(int superind, TPZCompMesh *super){
	if(super == this) return superind;
	if(super != FatherMesh()) superind = FatherMesh()->GetFromSuperMesh(superind,super);
	int i,nc = fConnectIndex.NElements();
	for(i=0; i<nc; i++) if(fConnectIndex[i] == superind) break;
	if(i== nc) {
		int blocksize=super->ConnectVec()[superind].NDof(*(TPZCompMesh *)super);
		int gl = AllocateNewConnect(blocksize);
		fConnectIndex.Resize(fConnectIndex.NElements()+1);
		fConnectIndex[fConnectIndex.NElements()-1] = superind;
		fExternalLocIndex[gl] = fConnectIndex.NElements()-1;
		return gl;
	} else {
		int j;
		for(j=0; j<fExternalLocIndex.NElements(); j++) {
			if(fExternalLocIndex[j] == i) return j;
		}
		int blocksize=super->ConnectVec()[superind].NDof(*(TPZCompMesh *)super);
		j = AllocateNewConnect(blocksize);
		fExternalLocIndex[j] = i;
		return j;
	}
}

void TPZSubCompMesh::Prints(ostream &out){
	out << "Sub Mesh" << (void *) this;	
	TPZCompMesh::Print(out);
	out.flush();
	int i;
	for (i=0; i<fConnectVec.NElements(); i++){
		out << "Node[" << i <<"]\t" << fExternalLocIndex[i];
		if (fExternalLocIndex[i] != -1) out << "Index in father mesh:\t" << fConnectIndex[fExternalLocIndex[i]];
		out << endl;
	}
}
		


void TPZSubCompMesh::MakeInternal(int local){
	if (fExternalLocIndex[local] == -1) return;
	TPZCompMesh *father = FatherMesh();
	int superind = fConnectIndex[fExternalLocIndex[local]];
	if(father && RootMesh(local) != father) {
		cout << "ERROR";
	}
	TPZConnect::TPZDepend *listdepend = father->ConnectVec()[superind].FirstDepend();
	while(listdepend) {
		int depfatherindex = listdepend->fDepConnectIndex;
		int depindexlocal = GetFromSuperMesh(depfatherindex,father);
		int r = listdepend->fDepMatrix.Rows();
		int c = listdepend->fDepMatrix.Cols();
		ConnectVec()[local].AddDependency(local,depindexlocal,listdepend->fDepMatrix,0,0,r,c);
		father->ConnectVec()[depfatherindex].RemoveDepend(superind,depfatherindex);
		listdepend = father->ConnectVec()[superind].FirstDepend();
	}
	int i;
	int localindex = fExternalLocIndex[local];
	for (i=fExternalLocIndex[local]; i<fConnectIndex.NElements()-1; i++){
		fConnectIndex[i]= fConnectIndex[i+1];	
	}
	for(i=0; i<fConnectVec.NElements(); i++) {
		if(fExternalLocIndex[i] != -1 && fExternalLocIndex[i] > localindex) fExternalLocIndex[i]--;
	}
	fConnectIndex.Resize(fConnectIndex.NElements()-1);
	fExternalLocIndex[local]= -1;
}


TPZCompMesh * TPZSubCompMesh::RootMesh(int local){
	if (fExternalLocIndex[local] == -1) return this;
	else return (FatherMesh()->RootMesh(fConnectIndex[fExternalLocIndex[local]]));
	return NULL;
}

void TPZSubCompMesh::MakeAllInternal(){
	TPZStack<int> stack;
	int i,j;
	TPZCompMesh *father = FatherMesh();
	father->ComputeNodElCon();
	//TPZCompMesh::Print();
	//father->Print();
	for (i=0;i<fConnectVec.NElements();i++){
		if (fExternalLocIndex[i]==-1) continue;
		if (father->ConnectVec()[fConnectIndex[fExternalLocIndex[i]]].NElConnected() == 1) stack.Push(i);
	}
	while(stack.NElements()) {
		int locind = stack.Pop();
		TPZConnect &coni = father->ConnectVec()[fConnectIndex[fExternalLocIndex[locind]]];
		int can = 0;
		if (coni.FirstDepend()){
			TPZConnect::TPZDepend *listdepend = coni.FirstDepend();
			for(j=0;j<stack.NElements(); j++){
				int jlocind = stack[j];
				if (jlocind == locind) continue;
				//				TPZConnect &conj = father->ConnectVec()[fConnectIndex[fExternalLocIndex[jlocind]]];
				if (listdepend->HasDepend(fConnectIndex[fExternalLocIndex[jlocind]])) break;
			}
			if (j == stack.NElements()) can=1;
			else {
				int jlocind = stack[j];
				stack[j] = locind;
				stack.Push(jlocind);
			}
		}
		else {
			can=1;
		}
		if(can && RootMesh(locind) != FatherMesh()) can = 0;
		if (can) {
			MakeInternal(locind);
		}
	}
	//TPZCompMesh::Print();
	//father->Print();
	//cout.flush();
}

void TPZSubCompMesh::SetConnectIndex(int inode, int index){
	fConnectIndex[inode] = index;
}

int TPZSubCompMesh::TransferElementFrom(TPZCompMesh *mesh, int elindex){
	if(mesh == this) return elindex;
		if (! IsAllowedElement(mesh,elindex)) { 
		cout <<"TPZSubCompMesh::TransferElementFrom ERROR: trying to transfer an element not allowed" << endl;	
		return -1;
	}
	if (mesh != FatherMesh()){
		elindex = FatherMesh()->TransferElementFrom(mesh,elindex);
	}
	if (CommonMesh(mesh) != mesh){
		cout <<"TPZSubCompMesh::TransferElementFrom ERROR: mesh is not supermesh" << endl;
		return -1;
	}
	TPZCompMesh *father = FatherMesh();
	TPZCompEl *cel = father->ElementVec()[elindex];
	if (!cel) {
		cout <<"TPZSubCompMesh::TransferElementFrom ERROR: element not existing" << endl;
		return -1;
	}
	int i,ncon = cel->NConnects();
	for (i=0; i<ncon; i++){
		int superind = cel->ConnectIndex(i);
		int subindex = GetFromSuperMesh(superind,father);
		cel->SetConnectIndex(i,subindex);
	}
	cel->SetMesh(this);
//	int blocksize=mesh->ConnectVec()[elindex].NDof((TPZCompMesh *)mesh);
	int newelind = fElementVec.AllocateNewElement();
	fElementVec[newelind] = cel;
	father->ElementVec()[elindex] = 0;
	father->ElementVec().SetFree(elindex);
	return newelind;
}

int TPZSubCompMesh::TransferElementTo(TPZCompMesh *mesh, int elindex){
	TPZCompMesh *common = CommonMesh(mesh);
	if ( common!= mesh){
		cout <<"TPZSubCompMesh::TransferElementTo ERROR: mesh is not supermesh" << endl;
		return -1;
	}
	if(mesh == this) return elindex;

	if (mesh != FatherMesh()){
		FatherMesh()->TransferElementTo(mesh,elindex);
	}
	

	TPZCompMesh *father = FatherMesh();
	if(elindex >= ElementVec().NElements()){
		cout <<"TPZSubCompMesh::TransferElementTo ERROR: not possible transfer non existing element" << endl;
		return -1;
	}
	TPZCompEl *cel = ElementVec()[elindex];
	if (!cel) {
		cout <<"TPZSubCompMesh::TransferElementTo ERROR: not possible transfer null element" << endl;
		return -1;
	}
	int i,ncon = cel->NConnects();
	for (i=0; i<ncon; i++){
		int subindex = cel->ConnectIndex(i);
		MakeExternal(subindex);
		int superind = fConnectIndex[fExternalLocIndex[subindex]];
		cel->SetConnectIndex(i,superind);
	}
//	int blocksize=father->ConnectVec()[elind].NDof(father);
	int newelind = father->ElementVec().AllocateNewElement();
	father->ElementVec()[newelind] = cel;
	cel->SetMesh(father);
	ElementVec()[elindex] = 0;
	ElementVec().SetFree(elindex);
	return newelind;
}

int TPZSubCompMesh::TransferElement(TPZCompMesh *mesh, int elindex){
	TPZCompMesh *comm = CommonMesh(mesh);
	int newelind = mesh->TransferElementTo(comm,elindex);
	int ell=TransferElementFrom(comm,newelind);
	InitializeBlock();
	return ell;
}

int TPZSubCompMesh::IsAllowedElement(TPZCompMesh *mesh, int elindex){
	if (CommonMesh(mesh) == mesh){
		TPZCompMesh *father = this;
		while(father->FatherMesh() != mesh) {
			father = father->FatherMesh();
		}
		TPZSubCompMesh *sub = (TPZSubCompMesh *) father;
		int index = sub->Index();
		return (elindex != index);
	}
	return 1;
}

void TPZSubCompMesh::CalcStiff(TPZElementMatrix &ek, TPZElementMatrix &ef){
	if(!fAnalysis) this->SetAnalysis();
	int i=0;
	CleanUpUnconnectedNodes();
	PermuteExternalConnects();
	
	TPZBlock &block = Mesh()->Block();
	//	TPZFMatrix &MeshSol = Mesh()->Solution();
	// clean ek and ef
	if(!ek.fMat) ek.fMat = new TPZFMatrix();
	if(!ef.fMat) ef.fMat = new TPZFMatrix();
	if(!ek.fBlock) ek.fBlock = new TPZBlock(ek.fMat);
	if(!ef.fBlock) ef.fBlock = new TPZBlock(ef.fMat);

	int nmeshnodes = fConnectVec.NElements();
	int numeq=0;
	//??

	for (i=0; i< nmeshnodes; i++){
		if(fExternalLocIndex[i] == -1) {
				TPZConnect &df = fConnectVec[i];
				if(df.HasDependency() || !df.NElConnected() || df.SequenceNumber() == -1) continue;
				int seqnum = df.SequenceNumber();
				numeq += Block().Size(seqnum);
		}
	}
	numeq = (TPZCompMesh::NEquations()) - numeq;
//??

	ek.fMat->Redim(numeq,numeq);
	ef.fMat->Redim(numeq,1);

	int nelemnodes = NConnects();
	
	ek.fBlock->SetNBlocks(nelemnodes);
	ef.fBlock->SetNBlocks(nelemnodes);
	for (i = 0; i < nelemnodes ; i++)	{
		int nodeindex = ConnectIndex(i);
  		ek.fBlock->Set(i,block.Size(nodeindex));
  		ef.fBlock->Set(i,block.Size(nodeindex));
	  }

	  if( !ek.fMat || !ef.fMat || !ek.fBlock || !ef.fBlock){
		cout << "TPZSubCompMesh.Calc_stiff : not enough storage for local stifness"
		  " matrix \n";
		TPZCompMesh::Print(cout);
		if(ek.fMat)   delete ek.fMat;
		if(ek.fBlock) delete ek.fBlock;
		if(ef.fMat)   delete ef.fMat;
		if(ef.fBlock) delete ef.fBlock;
		ek.fMat=  NULL;
		ek.fBlock = NULL;
		ef.fMat = NULL;
		ef.fBlock = NULL;
		return;
	  }
	  ek.fConnect.Resize(nelemnodes);
	  ef.fConnect.Resize(nelemnodes);

	  for(i=0; i<nelemnodes; ++i){
		(ef.fConnect)[i] = ConnectIndex(i);
		(ek.fConnect)[i] = ConnectIndex(i);
	  }
	if (! fAnalysis){
		Assemble (*ek.fMat,*ef.fMat);
	}
	else{
		fAnalysis->Run(cout);
		fAnalysis->CondensedSolution(*ek.fMat,*ef.fMat);
//		ek.fMat->Print("ek reduzido");
//		cout.flush();
	}
	//ek.fMat->Print();
}

void TPZSubCompMesh::SetAnalysis(){
	if(fAnalysis) delete fAnalysis;
	fAnalysis = new TPZSubMeshFrontalAnalysis(this);
	//	int numint = NumInternalEquations();
	TPZFrontStructMatrix<TPZFrontSym> fstr(this);
	fAnalysis->SetStructuralMatrix(fstr);
	TPZStepSolver solver;
	fAnalysis->SetSolver(solver);
	PermuteExternalConnects();
//	ofstream out("subcmesh.dat");
//	Prints(out);
}

void TPZSubCompMesh::PermuteExternalConnects(){
	//compute number of internal nodes -> numinternal
//	TPZCompMesh::Print();

	int i=0, numinternal=0;
	int nconnects = fConnectVec.NElements();
//cout << "fExternalLocIndex\n";
//for(i=0; i<nconnects; i++) cout << fExternalLocIndex[i] << ' ';
//cout << endl;
	for(i=0;i<nconnects; i++){
		if (fExternalLocIndex[i]==-1){
			// which are not free and which are not constrained
			TPZConnect &no = fConnectVec[i];

			if(no.NElConnected() == 0) continue;
			//se n�o tiver elemento conectado tambe'm
			numinternal+= 1;
		}
	}
	// initialize a counter for internal nodes
	i=0;
	int seqnum=0;
	int countint=0;
	TPZManVector<int> permute(nconnects);
	for (i=0;i<nconnects;i++) permute[i] = i;

	// loop over all nodes
	for (i=0;i<fConnectVec.NElements();i++){
		// take seqnum = the sequencenumber of the node
		//TPZConnect &no = fConnectIndex[fExternalLocIndex[i]];
		TPZConnect &no = fConnectVec[i];
		seqnum = no.SequenceNumber();
		// if the node is free or constrained 
		if (no.HasDependency() || no.NElConnected() == 0) {
			//->set permute[sequnum] to itself
			continue;
		}
		// if the node is internal 	
		if (fExternalLocIndex[i] == -1){
			//-> set permute[sequnum] to counter
			permute[seqnum] = countint;
			//-> increment counter
			countint += 1;
		}
		// if the node is external
		else{
			// ->set permute[seqnum] = fExternalConnectIndex+numinternal
			permute [seqnum] = fExternalLocIndex[i]+numinternal;
		// end loop
		}
	}
	//for (i=0;i<NConnects();i++){
	//	cout << "Permute [" <<i <<"]\n";
	//}
	Permute(permute);
}

void TPZSubCompMesh::LoadSolution(){
  //	int count = 0;
	int i=0;
	int seqnumext;
	int seqnumint;
	//	int numinteq = NumInternalEquations();
	int size;
	TPZFMatrix &sol = Mesh()->Solution();

	for (i=0;i<fConnectVec.NElements(); i++) {
		if (fExternalLocIndex[i] != -1) {
			TPZConnect &noext = Mesh()->ConnectVec()[fConnectIndex[fExternalLocIndex[i]]];
			TPZConnect &noint = fConnectVec[i];
			seqnumext = noext.SequenceNumber();
			size = (Mesh()->Block()).Size(seqnumext);
			seqnumint = noint.SequenceNumber();
			int posext = Mesh()->Block().Position(seqnumext);
			int posint = fBlock.Position(seqnumint);
			int l;
			for(l=0;l<size;l++) {
				fSolution(posint+l,0) = sol(posext+l,0);
			}
		}
	}
	if(fAnalysis) fAnalysis->LoadSolution(fSolution);
	TPZCompMesh::LoadSolution(fSolution);
}


void Forcing(TPZVec<REAL> &x, TPZVec<REAL> &disp){
	disp[0] = -(x[1]-0.5)*sin(angle)+(x[0]-0.5)*cos(angle)-(x[0]-0.5);
	disp[1] = (x[1]-0.5)*cos(angle)+(x[0]-0.5)*sin(angle)-(x[1]-0.5);
	disp[2] = 0.;
}


void TPZSubCompMesh::Skyline(TPZVec<int> &skyline) {
	TPZCompMesh::Skyline(skyline);
	skyline.Resize(NumInternalEquations());
}

int TPZSubCompMesh::NumInternalEquations() {
	int nmeshnodes = fConnectVec.NElements();
	int numeq=0;
	//??

	int i;
	for (i=0; i< nmeshnodes; i++){
		if(fExternalLocIndex[i] == -1) {
				TPZConnect &df = fConnectVec[i];
				if(df.HasDependency() || !df.NElConnected() || df.SequenceNumber() == -1) continue;
				int seqnum = df.SequenceNumber();
				numeq += Block().Size(seqnum);
		}
	}
	return numeq;

}


REAL TPZSubCompMesh::CompareElement(int var, char *matname){
	return CompareMesh(var,matname);
}


void TPZSubCompMesh::LoadElementReference()
{
	TPZCompMesh::LoadReferences();
}


void TPZSubCompMesh::GetExternalConnectIndex (TPZVec<int> &extconn){
	int i;
	int count = 0;
	for(i=0; i<fExternalLocIndex.NElements(); i++){
		if (fExternalLocIndex[i] != -1) count++;
	}
	extconn.Resize(count);
	
	count=0;
	for(i=0; i<fExternalLocIndex.NElements(); i++){
		if (fExternalLocIndex[i] != -1){
			extconn[count] = i;
			count++;
		}
	}
	return;
}
	

TPZAnalysis * TPZSubCompMesh::GetAnalysis()
{
	return fAnalysis;
}
