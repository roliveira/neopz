//We are using matrices and solvers
#include <pzvec.h>
#include <pzgmesh.h>
#include <pzcompel.h>
#include <pzgeoel.h>
#include <pzquad.h>
#include <pzmat2dlin.h> 
#include <TPZGeoElement.h>
#include <pzskylstrmatrix.h>
#include <pzcmesh.h>
//IO
#include <iostream>
using namespace std;

// nx = number of nodes in x direction
// ny = number of nodes in y direction
TPZGeoMesh * GetMesh(int nx,int ny);

int main(){
	cout << "***********************************************************************\n";
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	cout << "PZ - Class 5 -->> Geometric Meshes and h refinment\n";
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	cout << "***********************************************************************\n\n";
	cout << "Number of nodes: x direction and y direction:     ";
	int nx,ny;
	cin >> nx >> ny;
  int order;
  cout << "Enter the order of the element:\n";
  cin >> order;
  TPZCompEl::gOrder = order;

	//Creates the geometric mesh
	TPZGeoMesh *mesh = GetMesh(nx,ny);

	//Prints the refined mesh
	mesh->Print(cout);

  TPZCompMesh *cmesh = new TPZCompMesh(mesh);

  TPZMat2dLin *mat2d = new TPZMat2dLin (1);
  TPZFMatrix xkin (1,1,1e6);
  TPZFMatrix xcin (1,1,0.);
  TPZFMatrix xfin (1,1,1e3);
  mat2d->SetMaterial(xkin,xcin,xfin);
  cmesh->InsertMaterialObject (mat2d);
//  TPZSkylStructMatrix skystr(cmesh);
  cmesh->AutoBuild();
  TPZVec<int> subelindex(4,0);
  cmesh->ElementVec()[0]->Divide(0,subelindex,0);
  cmesh->Reference()->Print(cout);

  delete cmesh;
  delete mesh;
  return 0;
  
}

TPZGeoMesh *GetMesh (int nx,int ny){
  int i,j;
  int id, index;

  //Let's try with an unitary domain
  REAL lx = 1.;
  REAL ly = 1.;

  //Creates the geometric mesh... The nodes and elements
  //will be inserted into mesh object during initilize process
  TPZGeoMesh *gmesh = new TPZGeoMesh();

  //Auxiliar vector to store a coordinate  
  TPZVec <REAL> coord (3,0.);
  
  //Nodes initialization
  for(i = 0; i < nx; i++){
    for(j = 0; j < ny; j++){
      id = i*ny + j;
      coord[0] = (i)*lx/(nx - 1);
      coord[1] = (j)*ly/(ny - 1);
      //using the same coordinate x for z
      coord[2] = 0.;
      //cout << coord << endl;
      //Get the index in the mesh nodes vector for the new node
      index = gmesh->NodeVec().AllocateNewElement();
      //Set the value of the node in the mesh nodes vector
      gmesh->NodeVec()[index] = TPZGeoNode(id,coord,*gmesh);
    }
  }

	//Creates a vector of pointers to geometric elements
  TPZGeoEl * elvec[(const int)((nx-1)*(ny-1))];
  
  //Auxiliar vector to store a element connectivities
  TPZVec <int> connect(4,0);
  
  //Element connectivities
  for(i = 0; i < (nx - 1); i++){
    for(j = 0; j < (ny - 1); j++){
      index = (i)*(ny - 1)+ (j);
      connect[0] = (i)*ny + (j);
      connect[1] = connect[0]+(ny);
      connect[2] = connect[1]+1;
      connect[3] = connect[0]+1;
      //cout << connect << endl;
      //Allocates and define the geometric element
      elvec[index] = gmesh->CreateGeoElement(EQuadrilateral,connect,1,id);
    }
  }
  //Generate neighborhod information
  gmesh->BuildConnectivity();
  return gmesh;
}
