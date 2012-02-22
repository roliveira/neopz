/**
 * @file
 * @brief Contains the TPZDohrPrecond class which implements a matrix which computes the preconditioner developed by Dohrmann. \n
 * Also contains TPZDohrPrecondThreadV1Data, TPZDohrPrecondV2SubData and TPZDohrPrecondV2SubDataList structure.
 */
/***************************************************************************
 *   Copyright (C) 2006 by Philippe Devloo   *
 *   phil@fec.unicamp.br   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef TPZDOHRPRECOND_H
#define TPZDOHRPRECOND_H

#include "pzmatrix.h"
#include <list>
#include <semaphore.h>
#include "tpzautopointer.h"
#include "tpzdohrsubstruct.h"
#include "tpzdohrmatrix.h"
#include "tpzdohrassembly.h"
#include "tpzdohrassemblelist.h"

#include "pzp_thread.h"

/**
 * \addtogroup substructure
 * @{
 */
/**
 @brief Implements a matrix which computes the preconditioner developed by Dohrmann. \ref substructure "Sub Structure"
 @author Philippe Devloo
 */
template <class TSubStruct = TPZDohrSubstruct> 
class TPZDohrPrecond : public TPZMatrix
{
	/**
	 * @brief The matrix class is a placeholder for a list of substructures
	 */
	std::list<TPZAutoPointer<TSubStruct> > fGlobal;
	/**
	 * @brief The global matrix associated with the coarse degrees of freedom
	 */
	TPZStepSolver * fCoarse; //K(c)
	/**
	 * The global residual vector associated with the coarse degrees of freedom
	 */
	//  TPZFMatrix fCoarseResidual; //r(c)
	/**
	 * The product K(c)_inv*r(c)
	 */
	//  TPZFMatrix fInvKcrc; //r(c)
	/**
	 * @brief Size of the coarse system
	 */
	int fNumCoarse; //n(c)
	
	/** @brief Number of threads used during preconditioning */
	int fNumThreads;
	
	TPZAutoPointer<TPZDohrAssembly> fAssemble;
	
public:
    /** @brief Constructor with matrix */
    TPZDohrPrecond(TPZDohrMatrix<TSubStruct> &origin, TPZAutoPointer<TPZDohrAssembly> assemble);
	/** @brief Copy constructor */
	TPZDohrPrecond(const TPZDohrPrecond &copy);
	
    ~TPZDohrPrecond();
    
    CLONEDEF(TPZDohrPrecond)
    
    /**
	 * @brief The matrix class is a placeholder for a list of substructures
	 */
	std::list<TPZAutoPointer<TSubStruct> > &Global()
    {
        return fGlobal;
    }
	
	/** @brief Initialize the necessary datastructures */
	/** It will compute the coarse matrix, coarse residual and any other necessary data structures */
	void Initialize();
    
        /**
         * Specify the number of threads for preconditioning
         */
        void SetNumThreads(int numthreads)
        {
	  fNumThreads = numthreads;
        }

	//void AddSubstruct(TPZAutoPointer<TPZDohrSubstruct> substruct);
    
	/**
	 * @brief The only method any matrix class needs to implement
	 *
	 * In this case the variable x represents the rhs and z the result of the preconditioning \n
	 * When used as a preconditioner y will be zero
	 * In fact, it will compute v1+v2+v3 \n
	 * It computes z = beta * y + alpha * opt(this)*x but z and x can not overlap in memory.
	 * @param x Is x on the above operation. It must be a vector!
	 * @param y Is y on the above operation
	 * @param z Is z on the above operation
	 * @param alpha Is alpha on the above operation
	 * @param beta Is beta on the above operation
	 * @param opt Indicates if is Transpose or not
	 * @param stride Indicates n/N where n is dimension of the right hand side vector and N is matrix dimension
	 */
	virtual void MultAdd(const TPZFMatrix &x,const TPZFMatrix &y, TPZFMatrix &z,
						 const REAL alpha,const REAL beta,const int opt,const int stride) const;
	
	/** @brief Specify the solution process for the coarse matrix */
	void SetSolver(TPZSolver &solver);
	
	/** @brief Compute the contribution of the coarse matrix */
	void ComputeV1(const TPZFMatrix &x, TPZFMatrix &v1) const;
	/** @brief Compute the contribution of the sub domains */
	void ComputeV2(const TPZFMatrix &x, TPZFMatrix &v2) const;
};

#include <pthread.h>

/**
 * @brief Auxiliar structure with thread to compute the preconditioner developed by Dohrmann. \ref substructure "Sub Structure"
 */
template <class TSubStruct> 
struct TPZDohrPrecondThreadV1Data {
	TPZDohrPrecondThreadV1Data() : fDohrMatrix(0), fInput(0), fOutput(0)
	{
	}
	TPZDohrPrecondThreadV1Data(const TPZDohrPrecond<TSubStruct> *ptr, const TPZFMatrix &input, TPZFMatrix &output) : fDohrMatrix(ptr),
	fInput(&input), fOutput(&output)
	{
	}
	/** @brief Pointer to the dohr matrix */
	const TPZDohrPrecond<TSubStruct> * fDohrMatrix;
	/** @brief Input matrix */
	const TPZFMatrix * fInput;
	/** @brief Matrix where the coarse solution will be contributed */
	TPZFMatrix *fOutput;
	/** @brief Compute the contribution of the coarse matrix */
	static void *ComputeV1(void *dataptr)
	{
		TPZDohrPrecondThreadV1Data<TSubStruct> *ptr = (TPZDohrPrecondThreadV1Data<TSubStruct> *) dataptr;
		ptr->fDohrMatrix->ComputeV1(*(ptr->fInput),*(ptr->fOutput));
		return dataptr;
	}
};

/**
 * @brief Auxiliar structure for v2 vector to compute the preconditioner developed by Dohrmann. \ref substructure "Sub Structure"
 */
template <class TSubStruct> 
struct TPZDohrPrecondV2SubData {
	
	TPZDohrPrecondV2SubData() : fSubStructure(0), fInput_local(0), fv2_local(0)
	{
	}
	
	TPZDohrPrecondV2SubData(int subindex, const TPZAutoPointer<TSubStruct> &substruct, TPZAutoPointer<TPZFMatrix> res_local) : fSubStructure(substruct),
	fInput_local(res_local)
	{
		fv2_local = new TPZDohrAssembleItem(subindex, res_local->Rows());
	}
	/** @note Protect ourselves from default copy constructors */
	TPZDohrPrecondV2SubData(const TPZDohrPrecondV2SubData<TSubStruct> &copy) : fSubStructure(copy.fSubStructure), fInput_local(copy.fInput_local),
	fv2_local(copy.fv2_local)
	{
	}
	
	TPZDohrPrecondV2SubData &operator=(const TPZDohrPrecondV2SubData &copy)
	{
		fSubStructure = copy.fSubStructure;
		fInput_local = copy.fInput_local;
		fv2_local = copy.fv2_local;
		return *this;
	}
	
	bool IsValid()
	{
		return fSubStructure;
	}
	
	~TPZDohrPrecondV2SubData()
	{
	}
	
	/** @brief Pointer to the dohr matrix */
	TPZAutoPointer<TSubStruct> fSubStructure;
	/** @brief Input matrix */
	TPZAutoPointer<TPZFMatrix> fInput_local;
	/** @brief The local contribution to the v2 vector */
	TPZAutoPointer<TPZDohrAssembleItem> fv2_local;
};

struct TPZDohrAssembleList;

/**
 * @brief Auxiliar structure with list for v2 vector data. \ref substructure "Sub Structure"
 */
template <class TSubStruct> 
struct TPZDohrPrecondV2SubDataList {
	TPZDohrPrecondV2SubDataList(TPZAutoPointer<TPZDohrAssembleList> &assemble) : fAssemblyStructure(assemble)
	{
	  PZP_THREAD_MUTEX_INIT(&fAccessLock, 0, "TPZDohrPrecondV2SubDataList::TPZDohrPrecondV2SubDataList()");
	}
	~TPZDohrPrecondV2SubDataList()
	{
	  PZP_THREAD_MUTEX_DESTROY(&fAccessLock, "TPZDohrPrecondV2SubDataList::~TPZDohrPrecondV2SubDataList()");
	}
	/** @brief Mutex which will enable the access protection of the list */
	pthread_mutex_t fAccessLock;
	/** @brief The list of structures which need to be computed */
	std::list<TPZDohrPrecondV2SubData<TSubStruct> > fWork;
	/** @brief Interface to add items in a thread safe way */
	void AddItem(TPZDohrPrecondV2SubData<TSubStruct> &data)
	{
	  PZP_THREAD_MUTEX_LOCK(&fAccessLock, "TPZDohrPrecondV2SubDataList::AddItem()");
	  fWork.push_back(data);
	  PZP_THREAD_MUTEX_UNLOCK(&fAccessLock, "TPZDohrPrecondV2SubDataList::AddItem()");
	}
	/** @brief Interface to pop an item in a thread safe way */
	TPZDohrPrecondV2SubData<TSubStruct> PopItem()
	{
		TPZDohrPrecondV2SubData<TSubStruct> result;
		PZP_THREAD_MUTEX_LOCK(&fAccessLock, "TPZDohrPrecondV2SubDataList::PopItem()");
		if (fWork.size()) {
			result = *fWork.begin();
			fWork.pop_front();
		}
		PZP_THREAD_MUTEX_UNLOCK(&fAccessLock, "TPZDohrPrecondV2SubDataList::PopItem()");
		return result;
	}
	
	/** @brief The local contribution to the v2 vector */
	TPZAutoPointer<TPZDohrAssembleList> fAssemblyStructure;
	
	/** @brief The procedure which executes the lengthy process */
	static void *ThreadWork(void *voidptr);
	
};

/** @} */

#endif
