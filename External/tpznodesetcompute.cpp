//
// C++ Implementation: tpznodesetcompute
//
// Description: 
//
//
// Author: Philippe R. B. Devloo <phil@fec.unicamp.br>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "tpznodesetcompute.h"
#include "pzstack.h"
#include <set>
#include <map>

TPZNodesetCompute::TPZNodesetCompute()
{
  fMaxSeqNum = -1;
  fMaxLevel = -1;
}


TPZNodesetCompute::~TPZNodesetCompute()
{
}


    /**
    Group the node graph as passed by the parameters
    */
void TPZNodesetCompute::AnalyseGraph()
{
  int nnodes = fNodegraphindex.NElements()-1;
  fSeqNumber.Resize(nnodes);
  this->fLevel.Resize(nnodes);
  fSeqNumber.Fill(-1);
  fLevel.Fill(0);
  TPZVec<std::set<int> > nodeset(nnodes);
  int in;
  for(in=0; in<nnodes; in++) AnalyseNode(in,nodeset);
  
}

/**
 * This method will analyse the set inclusion of the current node, calling the method
 * recursively if another node need to be analysed first
 */
void TPZNodesetCompute::AnalyseNode(int node, TPZVec< std::set<int> > &nodeset)
{
  if(fSeqNumber[node] != -1) return;
  if(! nodeset[node].size()) nodeset[node].insert(&fNodegraph[fNodegraphindex[node]],&fNodegraph[fNodegraphindex[node+1]]);
  int minlevel = 0;
  std::set<int>::iterator it;
  TPZStack<int> equalnodes;
  for(it = nodeset[node].begin(); it != nodeset[node].end(); it++)
  {
    int othernode = *it;
    if(fSeqNumber[othernode] != -1) 
    {
      minlevel = minlevel < fLevel[othernode]+1 ? fLevel[othernode]+1 : minlevel;
      continue;
    }
    else if(! nodeset[othernode].size())
    {
      nodeset[othernode].insert(&fNodegraph[fNodegraphindex[othernode]],&fNodegraph[fNodegraphindex[othernode+1]]);
    }
    bool inc = includes(nodeset[node].begin(),nodeset[node].end(),nodeset[othernode].begin(),nodeset[othernode].end());
    bool diff = nodeset[node] != nodeset[othernode];
    if( inc && diff) 
    {
      AnalyseNode(othernode,nodeset);
      minlevel = minlevel < fLevel[othernode]+1 ? fLevel[othernode]+1 : minlevel;
    }
    else if(!diff)
    {
      equalnodes.Push(othernode);
    }
  }
  fMaxSeqNum++;
  fSeqNumber[node] = fMaxSeqNum;
  fSeqCard.Push(1);
  fMaxLevel = fMaxLevel < minlevel ? minlevel : fMaxLevel;
  fLevel[node] = minlevel;
  nodeset[node].clear();
  int neq = equalnodes.NElements();
  int ieq;
  for(ieq=0; ieq<neq; ieq++)
  {
    fSeqNumber[equalnodes[ieq]] = fMaxSeqNum;
    fLevel[equalnodes[ieq]] = minlevel;
    nodeset[equalnodes[ieq]].clear();
    fSeqCard[fMaxSeqNum]++;
  }
}

void TPZNodesetCompute::BuildNodeGraph(TPZVec<int> &blockgraph, TPZVec<int> &blockgraphindex)
{
  int nnodes = fSeqNumber.NElements();
  blockgraphindex.Resize(fSeqCard.NElements()+1);
  blockgraph.Resize(nnodes);
  int seq = 0;
  blockgraphindex[0] = 0;
  blockgraphindex[1] = 0;
  for(seq=2; seq<fSeqCard.NElements(); seq++)
  {
    blockgraphindex[seq] = blockgraphindex[seq-1]+fSeqCard[seq-2];
  }
  int in;
  for(in = 0; in< nnodes; in++)
  {
    int seqn = fSeqNumber[in];
    blockgraph[blockgraphindex[seqn+1]] = in;
    blockgraphindex[seqn+1]++;
  }
}

void TPZNodesetCompute::BuildVertexGraph(TPZStack<int> &blockgraph, TPZVec<int> &blockgraphindex)
{
  std::map<int,int> vertices;
  int nnodes = fSeqNumber.NElements();
  int in;
  for(in=0; in<nnodes; in++)
  {
    if(fLevel[in] == this->fMaxLevel) vertices[fSeqNumber[in]] = in;
  }
  int nvert = vertices.size();
  blockgraphindex.Resize(nvert+1);
  blockgraphindex[0] = 0;
  int iv = 1;
  std::map<int,int>::iterator it;
  for(it=vertices.begin(); it != vertices.end(); it++)
  {
    int node = (*it).second;
    std::set<int> vertexset;
    std::set<int> included;
    std::set<int> notincluded;
    BuildNodeSet(node,vertexset);
    included.insert((*it).first);
    std::set<int>::iterator versetit;
    for(versetit = vertexset.begin(); versetit != vertexset.end(); versetit++)
    {
      int seq = fSeqNumber[*versetit];
      if(included.count(seq))
      {
        blockgraph.Push(*versetit);
      }
      else if(notincluded.count(seq))
      {
        continue;
      }
      std::set<int> locset;
      BuildNodeSet(*versetit,locset);
      if(includes(vertexset.begin(),vertexset.end(),locset.begin(),locset.end()))
      {
        included.insert(seq);
        blockgraph.Push(*versetit);
      }
      else
      {
        notincluded.insert(seq);
      }
    }
    blockgraphindex[iv] = blockgraph.NElements();
    iv++;
  }
}

void TPZNodesetCompute::BuildNodeSet(int node, std::set<int> &nodeset)
{
  nodeset.clear();
  nodeset.insert(&fNodegraph[fNodegraphindex[node]],&fNodegraph[fNodegraphindex[node+1]]);
}

/**
 * Look for elements formed by vertices, intersecting with the intersectvertices, one by one
 * If the intersection does not remove any of the intersectvertices, we found an element!
 */
void TPZNodesetCompute::AnalyseForElements(std::set<int> &vertices, std::set< std::set<int> > &elements)
{

  std::set<int> elem;
  std::set<int>::iterator intit,diffit;
  for(intit = vertices.begin(); intit != vertices.end(); intit++)
  {
    std::set<int> locset,diffset,interset,unionset;
    BuildNodeSet(*intit,locset);
    set_difference(vertices.begin(),vertices.end(),locset.begin(),locset.end(),inserter(diffset,diffset.begin()));
    if(diffset.size())
    {
    // some unions need to be made before calling this method
      for(diffit=diffset.begin(); diffit!= diffset.end(); diffit++) 
      {
        locset.clear();
        BuildNodeSet(*diffit,locset);
        unionset.insert(locset.begin(),locset.end());
      }
      diffset.clear();
      set_intersection(unionset.begin(),unionset.end(),vertices.begin(),vertices.end(),inserter(diffset,diffset.begin()));
      AnalyseForElements(diffset,elements);
      set_intersection(vertices.begin(),vertices.end(),locset.begin(),locset.end(),inserter(interset,interset.begin()));
      AnalyseForElements(interset,elements);
      return;
    }
  }
  elem = vertices;
  for(intit = vertices.begin(); intit != vertices.end(); intit++)
  {
    std::set<int> locset,interset;
    BuildVertexSet(*intit,locset);
    set_intersection(elem.begin(),elem.end(),locset.begin(),locset.end(),inserter(interset,interset.begin()));
    elem = interset;
  }
  elements.insert(elem);
}

void TPZNodesetCompute::BuildElementGraph(TPZStack<int> &blockgraph, TPZStack<int> &blockgraphindex)
{
  blockgraph.Resize(0);
  blockgraphindex.Resize(1);
  blockgraphindex[0] = 0;
  int in;
  int nnodes = this->fNodegraphindex.NElements()-1;
  std::set<int> nodeset;
  for(in=0; in<nnodes; in++)
  {
    std::set< std::set<int> > elements;
    BuildNodeSet(in,nodeset);
    SubstractLowerNodes(in,nodeset);
    AnalyseForElements(nodeset,elements);
    std::set< std::set<int> >::iterator itel;
    for(itel = elements.begin(); itel != elements.end(); itel++)
    {
      std::set<int>::iterator it;
      for(it = (*itel).begin(); it!= (*itel).end(); it++)
      {
        blockgraph.Push(*it);
      }
      blockgraphindex.Push(blockgraph.NElements());
    }
  }
}

void TPZNodesetCompute::SubstractLowerNodes(int node, std::set<int> &nodeset)
{
  std::set<int> lownode,lownodeset,unionset;
  std::set<int>::iterator it;
  for(it=nodeset.begin(); it != nodeset.end() && *it < node; it++)
  {
    BuildNodeSet(*it,lownodeset);
    unionset.insert(lownodeset.begin(),lownodeset.end());
    lownodeset.clear();
  }
  set_difference(nodeset.begin(),nodeset.end(),unionset.begin(),unionset.end(),
    inserter(lownode,lownode.begin()));
  unionset.clear();
  for(it=lownode.begin(); it!=lownode.end(); it++)
  {
    BuildNodeSet(*it,lownodeset);
    unionset.insert(lownodeset.begin(),lownodeset.end());
    lownodeset.clear();
  }
  lownode.clear();
  set_intersection(unionset.begin(),unionset.end(),nodeset.begin(),nodeset.end(),
    inserter(lownode,lownode.begin()));
  nodeset = lownode;
}
