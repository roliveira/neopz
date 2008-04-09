﻿/***************************************************************************
                          tpzgeoelrefpattern.h  -  description
                             -------------------
    begin                : Tue Dec 23 2003
    copyright            : (C) 2003 by cesar
    email                : cesar@labmec.fec.unicamp.br
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TPZGEOELREFPATTERN_H
#define TPZGEOELREFPATTERN_H
#include "pzgeoelrefless.h"
#include "TPZRefPattern.h"
#include "pzvec.h"


/**Implements a geometric element which division is given by a TPZRefPattern object.
  *@author Edimar Cesar Rylo
  *@since 2003-12-23
  */
class TPZGeoElSide;
class TPZCompMesh;
class TPZCompEl;
template<class T,int N>
class TPZStack;
class TPZRefPattern;

/// Implements a generic geometric element which is refined according to a generic refinement pattern
/**
@ingroup geometry
Using this class it is possible to create inconsistent meshes
The consistency of the h-refined mesh using generic refinement patterns is the responsability of the user of the class
*/
template <class TGeo>
class TPZGeoElRefPattern : public TPZGeoElRefLess<TGeo>  {

    TPZVec<int> fSubEl;
    TPZAutoPointer<TPZRefPattern> fRefPattern;

public:
  typedef TGeo Geo;
  TPZGeoElRefPattern();
  ~TPZGeoElRefPattern();
  TPZGeoElRefPattern(int id,TPZVec<int> &nodeindexes,int matind,TPZGeoMesh &mesh);
  TPZGeoElRefPattern(TPZVec<int> &nodeindices,int matind,TPZGeoMesh &mesh);
  TPZGeoElRefPattern(TPZVec<int> &nodeindices,int matind,TPZGeoMesh &mesh,int &index);
  void Initialize(TPZVec<int> &nodeindices,int matind,TPZGeoMesh &mesh,int &index,TPZAutoPointer<TPZRefPattern> refpat);

  /** return 1 if the element has subelements along side*/
  int HasSubElement() {return fSubEl.NElements() && fSubEl[0]!=-1;}

  void SetSubElement(int id, TPZGeoEl *el);

  /**volume of the master element*/
  REAL RefElVolume();

  /**returns the midside node index along a side of the element*/
  void MidSideNodeIndex(int side,int &index);

  /**returns the midside node indices along a side of the element*/
  void MidSideNodeIndices(int side,TPZVec<int> &indices);

  /**
   * return the number of subelements of the element independent of the
   * fact hether the element has already been refined or not
   */
  int NSubElements();

  /**
  * return the number of subelements as returned by GetSubElements2(side)
  */
  int NSideSubElements2(int side);

  /**returns a pointer to the subelement is*/
  TPZGeoEl *SubElement(int is);

  /**return a pointer and a side of the subelement of the element at the side
     and the indicated position. position = 0 indicate first subelement, ...*/
  TPZGeoElSide SideSubElement(int side,int position);

  TPZTransform GetTransform(int side,int son);

  virtual int FatherSide(int side, int son);

  /**divides the element and puts the resulting elements in the vector*/
  virtual void Divide(TPZVec<TPZGeoEl *> &pv);
  

  virtual void GetSubElements2(int side, TPZStack<TPZGeoElSide> &subel);

  /** Defines the refinement pattern. It's used only in TPZGeoElRefPattern objects. */
  virtual void SetRefPattern(TPZAutoPointer<TPZRefPattern> refpat );

  /// return the refinement pattern associated with the element
  virtual TPZAutoPointer<TPZRefPattern> GetRefPattern()
  {
    return fRefPattern;
  }

  virtual void Print(std::ostream & out);

  virtual void ResetSubElements();

	/** Saveable methods */
#ifndef WIN32
	virtual int ClassId() const;
#endif
  virtual void Read(TPZStream &str, void *context);
  virtual void Write(TPZStream &str, int withclassid);
  virtual TPZGeoEl * Clone(TPZGeoMesh &DestMesh) const;

  /**
   * @see class TPZGeoEl
   */
  virtual TPZGeoEl * ClonePatchEl(TPZGeoMesh &DestMesh,
                                  std::map<int,int> &gl2lcNdIdx,
                                  std::map<int,int> &gl2lcElIdx) const;


  TPZGeoElRefPattern(TPZGeoMesh &DestMesh, const TPZGeoElRefPattern<TGeo> &cp);

  /**
   * Clone constructor for patch meshes
   * @param DestMesh destination patch mesh
   * @param cp element to be copied
   * @param gl2lcNdIdx map between the original node indexes and patch node indexes
   * @param gl2lcElIdx map between the original element indexes and patch element index
   */
  TPZGeoElRefPattern ( TPZGeoMesh &DestMesh,
                       const TPZGeoElRefPattern<TGeo> &cp,
                       std::map<int,int> &gl2lcNdIdx,
                       std::map<int,int> &gl2lcElIdx );

};

TPZGeoEl *CreateGeoElementPattern(TPZGeoMesh &mesh,
                                  MElementType type,
                                  TPZVec<int>& nodeindexes,
                                  int matid,
                                  int& index);

//--| IMPLEMENTATION |----------------------------------------------------------

template<class TGeo>
TPZGeoElRefPattern<TGeo>::TPZGeoElRefPattern():TPZGeoElRefLess<TGeo>(){
  fSubEl.Resize(1);
  fSubEl[0] = -1;
}

template<class TGeo>
TPZGeoElRefPattern<TGeo>::~TPZGeoElRefPattern(){
//#warning "Acredito que os objetos deste tipo devem ser administrados pela malha"
//  if (fRefPattern) delete fRefPattern;
}

template<class TGeo>
TPZGeoElRefPattern<TGeo>::TPZGeoElRefPattern(TPZVec<int> &nodeindices,int matind,TPZGeoMesh &mesh) :
  TPZGeoElRefLess<TGeo>(nodeindices,matind,mesh) {
}

template<class TGeo>
TPZGeoElRefPattern<TGeo>::TPZGeoElRefPattern(TPZVec<int> &nodeindices,int matind,TPZGeoMesh &mesh, int &index) :
  TPZGeoElRefLess<TGeo>(nodeindices,matind,mesh,index) {
}

template<class TGeo>
TPZGeoElRefPattern<TGeo>::TPZGeoElRefPattern(int id,TPZVec<int> &nodeindexes,int matind,TPZGeoMesh &mesh) :
  TPZGeoElRefLess<TGeo>(id,nodeindexes,matind,mesh) {
}

template<class TGeo>
void TPZGeoElRefPattern<TGeo>::Initialize(TPZVec<int> &nodeindices, int matind, TPZGeoMesh& mesh, int& index, TPZAutoPointer<TPZRefPattern> refpat) {
  TPZGeoElRefLess<TGeo>::Initialize(nodeindices,matind,mesh,index);
  fRefPattern = refpat;
  if(fRefPattern)
  {
    fSubEl.Resize(fRefPattern->NSubElements());
    fSubEl.Fill(-1);
  }
}

template<class TGeo>
void TPZGeoElRefPattern<TGeo>::SetSubElement(int id, TPZGeoEl *el){
  if (!fRefPattern){
    PZError << "Error at " << __PRETTY_FUNCTION__ << " at line " << __LINE__ << " - this->GetRefPattern() is NULL\n";
    return;
  }//if
  int nsubel = this->GetRefPattern()->NSubElements();
  if (id<0 || id >nsubel){
    PZError << "TPZGeoElRefPattern::Trying do define subelement :" << id << std::endl;
    return;
  }
  fSubEl[id] = el->Index();
  return;
}

template<class TGeo>
REAL TPZGeoElRefPattern<TGeo>::RefElVolume(){
  return TGeo::RefElVolume();
}

template<class TGeo>
void TPZGeoElRefPattern<TGeo>::MidSideNodeIndex(int side,int &index){
  index = -1;
  int i,j;
  if(side<0 || side>this->NSides()-1) {
    PZError << "TPZGeoElRefPattern::MidSideNodeIndex. Bad parameter side = " << side << std::endl;
    return;
  }
  //corner side
  if(side<this->NCornerNodes()) {//o n�medio do lado 0 �o 0 etc.
    index = this->NodeIndex(side);
    return;
  }
  //o n�medio da face �o centro e o n�medio do centro �o centro
  //como n�de algum filho se este existir
  //caso tenha filhos �o canto de algum filho, se n� tiver filhos retorna -1
  if(HasSubElement()) {
    //side -= NCornerNodes();
    //index =(gel->SubElement(MidSideNodes[side][0]))->NodeIndex(MidSideNodes[side][1]);
    int nsubel = this->NSideSubElements(side);
    TPZStack <TPZGeoElSide> subels;
    GetSubElements2(side,subels);
    // Nao sei porque deve se fazer esta execao
    if (nsubel == 1) {
      subels[0].Element()->MidSideNodeIndex(subels[0].Side(),index);
      return;
    }
    TPZStack <int> msnindex;
    // este sidenodeindex pode nao existir. Normalmente o numero de nos de um elemento e igual
    // NNodes. Quer dizer se o lado e maior igual NNodes, este metodo nao devolvera nada

    int subnodeindex;
    for (i=0;i<nsubel;i++){
      if(subels[i].Side() >= subels[i].Element()->NCornerNodes()) continue;
      subnodeindex = subels[i].SideNodeIndex(0);
      msnindex.Push(subnodeindex);
    }
    if(msnindex.NElements() > 1) {
      std::cout << "TPZGeoElRefPattern<TGeo>::MidSideNodeIndex element with more than one midsidenodeindex..." << std::endl;
    }
    if (msnindex.NElements() == 1) index = msnindex[0];
    else if(msnindex.NElements() == 0) {
      index = -1;
    } else {
      TPZVec<REAL> centerpt(this->Dimension(),0.);
      TPZVec<REAL> auxpt(this->Dimension(),0.);
      this->CenterPoint(side,auxpt);
      this->X(auxpt,centerpt);

      REAL dif = 1e6;
      REAL aux = 0;
      index = msnindex[0];
      for (i=0;i<msnindex.NElements();i++){
        for (j=0;j<this->Dimension();j++)
          aux += (this->Mesh()->NodeVec()[msnindex[i]].Coord(j) - centerpt[j]) *
                    (this->Mesh()->NodeVec()[msnindex[i]].Coord(j) - centerpt[j]);
        if (aux < dif){
          dif = aux;
          index = msnindex[i];
        }
      }
    }
  }
}

template<class TGeo>
int TPZGeoElRefPattern<TGeo>::NSubElements(){
  if (!fRefPattern) return 0;
  return this->GetRefPattern()->NSubElements();
}

/*!
    \fn TPZGeoElRefPattern::ResetSubElements()
 */
template<class TGeo>
void
TPZGeoElRefPattern<TGeo>::ResetSubElements()
{
  int is;
  for (is=0;is<NSubElements();is++)
  {
    fSubEl[is] = -1;
  }
}


template<class TGeo>
int TPZGeoElRefPattern<TGeo>::NSideSubElements2(int side){
  if (!fRefPattern) return 0;
  return this->GetRefPattern()->NSideSubElements(side);
}

template<class TGeo>
TPZGeoEl * TPZGeoElRefPattern<TGeo>::SubElement(int is){
  if (!fRefPattern) return 0;
  int nsubel = this->GetRefPattern()->NSubElements();
  if(is<0 || is>nsubel){
    std::cout << "TPZGeoElRefPattern::SubElement index error is= " << is << std::endl;
  }
  return (fSubEl[is] == -1) ? 0 : this->Mesh()->ElementVec()[fSubEl[is]];
}

template<class TGeo>
TPZGeoElSide TPZGeoElRefPattern<TGeo>::SideSubElement(int side,int position){
//  TPZStack<TPZGeoElSide> subs;
//
//  TRef::GetSubElements(this,side,subs);
//  return subs[position];
  TPZGeoElSide tmp;
  if(!fRefPattern) return tmp;
  int sub, sideout;
  this->GetRefPattern()->SideSubElement(side,position,sub,sideout);
  if (fSubEl[sub] == -1) {
    PZError << "TPZGeoElRefPattern<TGeo>::SideSubElement : Error subelement not found for side "
            << side << " position " << position << std::endl;
    return TPZGeoElSide();
  }
  return TPZGeoElSide (SubElement(sub),sideout);
}

template<class TGeo>
TPZTransform TPZGeoElRefPattern<TGeo>::GetTransform(int side,int son){
//  return TRef::GetTransform(side,son);
  TPZTransform trf;
  if(!fRefPattern) return trf;
  return this->GetRefPattern()->Transform(side,son);
}

template<class TGeo>
void
TPZGeoElRefPattern<TGeo>::GetSubElements2(int side, TPZStack<TPZGeoElSide> &subel){
  //TRef::GetSubElements(this,side,subel);
//  int i,nsidesubel;

  if(!fRefPattern) return;
  //A classe TPZRefPattern nao esta contemplando os nos
  TPZGeoEl * reffather = this->GetRefPattern()->Element(0);
  if (side < reffather->NCornerNodes()){
    TPZGeoElSide thisside (reffather,side);
    TPZGeoElSide neighbour = thisside.Neighbour();
    while(neighbour.Exists() && neighbour != thisside){
      TPZGeoEl *gel = neighbour.Element();
      TPZGeoEl *father = gel->Father();
      if (father == reffather) {
        int sonid = neighbour.Element()->Id()-1; //o id 0 e sempre do pai por definicao da classe
        int sonside = neighbour.Side();
        TPZGeoElSide sideson (SubElement(sonid),sonside);
        subel.Push(sideson);
      }
      neighbour = neighbour.Neighbour();
    }
    return;
  }
  this->GetRefPattern()->SidePartition(subel,side);
  int size = subel.NElements();
  int el;
  for(el=0; el<size; el++) {
    TPZGeoEl *gelref = subel[el].Element();
    int subelindex = gelref->Id()-1;
    TPZGeoEl *mysub = SubElement(subelindex);
    subel[el] = TPZGeoElSide(mysub,subel[el].Side());
  }
}


template<class TGeo>
void
TPZGeoElRefPattern<TGeo>::Divide(TPZVec<TPZGeoEl *> &SubElVec){
  if (!fRefPattern) {
    PZError << "TPZGeoElRefPattern<TGeo>::Divide ERROR : Undefined Refinement Pattern!" << std::endl;
    SubElVec.Resize(0);
    return;
  }
  int i;
  int NSubEl = this->GetRefPattern()->NSubElements();
  if(HasSubElement()) {
    SubElVec.Resize(NSubEl);
    for(i=0;i<NSubEl;i++) SubElVec[i] = SubElement(i);
    return;//If exist fSubEl return this sons
  }
  int index,k,j,sub,matid=this->MaterialId();

  int totalnodes = this->GetRefPattern()->NNodes();
  TPZManVector<int> np(totalnodes);
  int nnodes = this->NCornerNodes();

  for(j=0;j<nnodes;j++) {
    np[j] = this->NodeIndex(j);
  }
  this->GetRefPattern()->CreateNewNodes (this, np);
  //std::cout << "NewNodeIndexes : " << np << std::endl;

  // map the nodes of the indices of the refinement pattern father element to the node indices
  std::map<int, int> nodemap;
  for(i=0; i<nnodes; i++)
  {
    nodemap[this->GetRefPattern()->Element(0)->NodeIndex(i)]=i;
  }
  for(;i<totalnodes;i++)
  {
    nodemap[i]=i;
  }

  // I dont think the previous part will work...
  // It should be better structured. One method which is not present within
  // the geometric element is returning the set of nodes present at the
  // interior of a side.
  // I suggest : MidsideNodeIndices(int side, TPZVec<int> &indices)
  // Its default behaviour can be to insert a single node.

  // creating new subelements
  for(i=0;i<NSubEl;i++) {
    int subcorner = this->GetRefPattern()->Element(i+1)->NCornerNodes();
    TPZManVector<int> cornerindexes(subcorner);
    for(j=0;j<subcorner;j++) {
      int cornerid = this->GetRefPattern()->Element(i+1)->NodeIndex(j);
      int mappedid = nodemap[cornerid];
      cornerindexes[j] = np[mappedid];
    }
    //std::cout << "Subel corner " << cornerindexes << std::endl;
    TPZGeoEl *subel = this->CreateGeoElement((MElementType)this->GetRefPattern()->Element(i+1)->Type(),cornerindexes,matid,index);
    SetSubElement(i,subel);
  }

  SubElVec.Resize(NSubEl);
  for(sub=0;sub<NSubEl;sub++) {
    SubElVec[sub] = SubElement(sub);
    SubElVec[sub]->SetFather(this);
    SubElVec[sub]->SetFather(this->fIndex);
  }

//  this->Mesh()->Print(std::cout);

//  fRefPattern->Mesh()->Print(std::cout);

  //std::cout << "conectividades entre os filhos : viz interna  " << std::endl;
  for(i=0;i<NSubEl;i++) {
    //std::cout << "SubElement " << i << std::endl;
    TPZGeoEl *refsubel = this->GetRefPattern()->Element(i+1);
    for (j=0;j<refsubel->NSides();j++){
      //std::cout << "\t Side " << j << std::endl;
      TPZGeoElSide thisside (refsubel,j);
      TPZGeoElSide refneigh = refsubel->Neighbour(j);
      if(refneigh.Exists() && refneigh != thisside) {
        //como nao me deixam guardar o index do elemento geometrico...
        for (k=0;k<NSubEl+1;k++){
          if (this->GetRefPattern()->Element(k) == refneigh.Element() && k-1 != i){
        //    std::cout << "\t\t\tFound subel " << k << "  side of subel " << refneigh.Side()
          //      << "For subelement " << i << " side " << j << std::endl;
            TPZGeoElSide neighbour;
            if(k) {
              neighbour = TPZGeoElSide(SubElement(k-1),refneigh.Side());
            } else {
              neighbour = TPZGeoElSide(this, refneigh.Side());
            }
            TPZGeoElSide gs(SubElement(i),j);
            if(!gs.NeighbourExists(neighbour)) gs.SetConnectivity(neighbour);
            break;
          }
        }
        if(k == NSubEl+1) {
          //std::cout << "TPZGeoElRefPattern<TGeo>::Divide inconsistent datastructure subelement "
          //  << i << " Side " << j << std::endl;
        }
      } else {
        SubElement(i)->SetNeighbour(j,TPZGeoElSide(SubElement(i),j));
      }
    }
  }

//  this->Print(std::cout);
  /*for (i=0;i<NSubEl;i++){
    TPZGeoEl *subel = SubElement(i);
    if (subel) std::cout << "Subel " << i << " OK " << std::endl ; //subel->Print(std::cout);
    else  std::cout << "Erro no subelement0 " << i << std::endl;
}*/

  //std::cout << "Chegou aqui ..." << std::endl;
  this->SetSubElementConnectivities();
}

template<class TGeo> int TPZGeoElRefPattern<TGeo>::FatherSide(int side, int son){
  int res = -1;
  if(!fRefPattern) return res;
  return this->GetRefPattern()->FatherSide(side,son);

}

template<class TGeo>
void TPZGeoElRefPattern<TGeo>::MidSideNodeIndices(int side,TPZVec<int> &indices){
  if(!fRefPattern || !HasSubElement() || side < this->NCornerNodes()) {
    indices.Resize(0);
    return;
  }
  TPZManVector<TPZGeoElSide> gelsides;
  this->GetRefPattern()->SidePartition(gelsides,side);
  int nsub = gelsides.NElements();
  indices.Resize(nsub);
  int is;
  int counter = 0;
  for(is=0; is<nsub; is++) {
    if(gelsides[is].Side() >= gelsides[is].Element()->NCornerNodes()) continue;
    TPZGeoEl *subel = SubElement(gelsides[is].Element()->Id()-1);
    indices[counter] = subel->NodeIndex(gelsides[is].Side());
    counter++;
  }
  indices.Resize(counter);
}

/** Defines the element refinement pattern  */
template<class TGeo>
void TPZGeoElRefPattern<TGeo>::SetRefPattern (TPZAutoPointer<TPZRefPattern> refpat){
#ifdef HUGE_DEBUG
  if (!refpat) {
    PZError << "Error trying to set a null refinement pattern objetct" << std::endl;
    return;
  }
#endif
//  this->Mesh()->InsertRefPattern(refpat);
  //para nao manter copias, caso exista uso o existente;
//  MElementType eltype = refpat->Element(0)->Type();
//  std::string refname = refpat->GetName();
//  refpat = this->Mesh()->GetRefPattern(eltype,refname);
  fRefPattern = refpat;
  int i;
  int nsubel = refpat->NSubElements();
  fSubEl.Resize(nsubel);
  for(i=0;i<nsubel;i++) fSubEl[i] = -1;
}

template<class TGeo>
void TPZGeoElRefPattern<TGeo>::Print(std::ostream & out)
{
  TPZGeoElRefLess<TGeo>::Print(out);

  if(fRefPattern)
  {
    fRefPattern->ShortPrint(out);
    out << std::endl;
  }
}


#endif
