// Generated by Together

#include "pzmetis.h"

#ifdef USING_METIS
#include <math.h>
extern "C" {
#include "metis.h"
};
#endif

#include <iostream>
using namespace std;

TPZMetis::TPZMetis(int NElements, int NNodes) : TPZRenumbering(NElements,NNodes),
	fNodeWeights(0), fElementGraph(0), fElementGraphIndex(0) {
}

void TPZMetis::ClearDataStructures(){
	fNodeWeights.Resize(0);
   fElementGraph.Resize(0);
   fElementGraphIndex.Resize(0);
}

void TPZMetis::SetElementGraph(TPZVec<int> &elgraph, TPZVec<int> &elgraphindex){
	fElementGraph = elgraph;
	fElementGraphIndex = elgraphindex;
}
void TPZMetis::SetNodeWeights(TPZVec<int> &weights){
	fNodeWeights = weights;
}

void TPZMetis::Print(ostream &out,char * title) {

 	int nel = fElementGraphIndex.NElements()-1;
   int el;
   out << title;
   out << "\nTPZMetis::Print fNElements = " << fNElements << " fNNodes = " << fNNodes << endl;
   for(el=0;el<nel;el++) {
   	int firstindex = fElementGraphIndex[el];
   	int lastindex = fElementGraphIndex[el+1];
      int index;
      out << "Element number " << el << " : ";
      for(index=firstindex;index<lastindex;index++) {
      	out << fElementGraph[index] << " ";
      }
      out << endl;
   }
	TPZVec<int> nodegraph(0),nodegraphindex(0);
	ConvertGraph(fElementGraph,fElementGraphIndex,nodegraph,nodegraphindex);
	int numelnodegraph = nodegraphindex[fNNodes];
	if(numelnodegraph == nodegraph.NElements() ) {
		nodegraph.Resize(numelnodegraph+1);
	}
	int nod;
	for(nod = numelnodegraph; nod>0; nod--) nodegraph[nod] = nodegraph[nod-1];
   for(el=0;el<fNNodes;el++) {
   	int firstindex = nodegraphindex[el];
   	int lastindex = nodegraphindex[el+1];
      int index;
      out << "Node number " << el << " : ";
      for(index=firstindex;index<lastindex;index++) {
      	out << nodegraph[index+1] << " ";
      }
      out << endl;
   }
}

void TPZMetis::Print(ostream &out) {

  // 	int nel = fElementGraphIndex.NElements()-1;
   int el;
   out << fNNodes << endl;
/*   for(el=0;el<nel;el++) {
   	int firstindex = fElementGraphIndex[el];
   	int lastindex = fElementGraphIndex[el+1];
      int index;
      for(index=firstindex;index<lastindex;index++) {
      	if(fElementGraph[index] == index) continue;//o atual n�o deve aparecer
      	out << (fElementGraph[index]+1) << " ";
      }
      out << endl;
   } */
	TPZVec<int> nodegraph(0),nodegraphindex(0);
	ConvertGraph(fElementGraph,fElementGraphIndex,nodegraph,nodegraphindex);
	int numelnodegraph = nodegraphindex[fNNodes];
	if(numelnodegraph == nodegraph.NElements() ) {
		nodegraph.Resize(numelnodegraph+1);
	}
	int nod;
	for(nod = numelnodegraph; nod>0; nod--) nodegraph[nod] = nodegraph[nod-1];
   for(el=0;el<fNNodes;el++) {
   	int firstindex = nodegraphindex[el];
   	int lastindex = nodegraphindex[el+1];//come�o do pr�ximo
      int index;
      for(index=firstindex;index<lastindex;index++) {
      	out << (nodegraph[index+1]+1) << " ";
      }
      out << endl;
   }        
}

void TPZMetis::Resequence(TPZVec<int> &perm, TPZVec<int> &inverseperm){
	TPZVec<int> nodegraph(0),nodegraphindex(0);
	ConvertGraph(fElementGraph,fElementGraphIndex,nodegraph,nodegraphindex);
	int numelnodegraph = nodegraphindex[fNNodes]; 
	if(numelnodegraph == nodegraph.NElements() ) {
		nodegraph.Resize(numelnodegraph+1);
	}
	int nod;
	for(nod = numelnodegraph; nod>0; nod--) nodegraph[nod] = nodegraph[nod-1];
//	int numflag = 0;
//	int options = 0;
	perm.Resize(fNNodes);
	inverseperm.Resize(fNNodes);
	for(nod=0;nod<fNNodes;nod++) {perm[nod] = inverseperm[nod] = nod;}
#ifdef USING_METIS
	METIS_NodeND(&fNNodes,&nodegraphindex[0],&nodegraph[1],&numflag,&options,&perm[0],&inverseperm[0]);
#endif
}	



