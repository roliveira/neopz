
/*
  // Author: MISAEL LUIS SANTANA MANDUJANO.
  //
  // File:   tblock.hh
  //
  // Class:  TPZBlock
  //
  // Obs.:   Permite a visualizacao de matrizes atraves de blocos.
  //         So' podem ser inseridos, removidos ou modificados os
  //         blocos da diagonal da matriz. Os blocos da diagonal
  //         devem ser quadrados.
  //
  //
  // Versao: 12 / 1994.
*/


#ifndef _TBLOCKHH_
#define _TBLOCKHH_


#include "pzmatrix.h"
#include "pzmanvector.h"
#include "pzreal.h"
#include "pzsave.h"

/**
 * @brief Implements block matrices
 * @ingroup matrixutility
 */
class TPZBlock : public TPZSaveable
{
public:
	/**
	 * @brief For each elements on matrix a size 1 block is created
	 * @param matrix_to_represent Indicates which matrix is to be represented
	 * @param num_of_blocks Indicates number of blocks
	 * @param initial_blocks_size Indicates initial block size, default value is 1
	 */
	TPZBlock(TPZMatrix *const matrix_to_represent = 0,const int num_of_blocks = 0,
			 const int initial_blocks_size = 1 );
	
	/**
	 * @brief Copy constructor
	 * @param bl New object is created based on bl
	 */
	TPZBlock(const TPZBlock &bl);
	
	/**
     @brief Simple Destrutor
	 */
	virtual ~TPZBlock();
	
	///muda o ponteiro para a matriz other
	/**
     @brief Changes pointer to other
     @param other New matrix to be pointed to
	 */
	virtual void SetMatrix(TPZMatrix *const other);
	
	/**
     @brief Returns a pointer to current matrix
	 */
	TPZMatrix *Matrix(){ return fpMatrix;}
	
	/**
     @brief Sets number of blocks on diagonal matrix
     @param num_of_blocks Number of blocks
	 */
	int SetNBlocks(const int num_of_blocks );
	
	/// Modifica as dimensoes de um bloco existente ou cria um novo bloco caso nao existir bloco com o indice fornecido.
	/**
     @brief Modifies existing block dimensions or creates a new block with given index
     @param index Given index to be redimensioned or created
     @param dim New dimension
     @param pos New position
	 */
	int Set(const int index,const int dim,const int pos = -1 );
	
	// Metodo para calcular a sequencia dos blocos
	/**
     @brief Computes blocks sequence
     @param dimensions Contains blocks sequence
	 */
	int SetAll( TPZVec<int> & dimensions );
	
	//** Refaz a seq��ncia do posicionamento dos blocos*/
	/**
     @brief Resequences blocks positioning
     @param start Starting position
	 */
	int Resequence(const int start=0);
	
	// Remove um bloco
	/**
     @brief Removes a block
     @param index Index of the block to be removed
	 */
	int Remove(const int index );
	
	//Misael, Verifica se os blocos sao sequenciais e nao ultrapassan o tamanho da matrix  20/3/95
	/**
     @brief Verifies if blocks are sequential and does not overcome matrix size
	 */
	int Verify() const;
	
	REAL & operator()(const int block_row,const int block_col,const int r,const int c ) const;
	
	// Le e escreve um elemento na matriz, fazendo verificacoes.
	/// Get or put a element from or to matrix verifiying
	const REAL & Get(const int block_row,const int block_col,const int r,const int c ) const;
	int Put(const int block_row,const int block_col,const int r,const int c,const REAL& value );
	
	// Le e escreve um elemento na matriz, fazendo verificacoes.
	/// Get or put a element from or to matrix verifiying
	const REAL & Get(const int block_row,const int r,const int c ) const;
	int Put(const int block_row,const int r,const int c,const REAL& value );
	
	// Le e escreve um elemento na matriz, sem fazer verificacoes. GetVal devolva referencia
	/// Get or put a element from or to matrix, don't make verifiying
	const REAL & GetVal(const int bRow,const int bCol,const int r,const int c ) const;
	int PutVal(const int bRow,const int bCol,const int r,const int c,const REAL& value );
	
	// Escreve, le e soma um bloco na matriz. 'block_row' e
	//  'block_col' sao dados em unidades de blocos.
	/**
     @brief Puts a block on current matrix
     @param block_row Contains block row
     @param block_col Contains block column
     @param block Block to be inserted
	 */
	int PutBlock(const int block_row,const int block_col,const TPZFMatrix & block );
	/**
     @brief Gets a block on current matrix
     @param block_row Contains block row
     @param block_col Contains block column
     @param block Block to be inserted
	 */
	int GetBlock(const int block_row,const int block_col, TPZFMatrix *const block ) const;
	/**
     @brief Adds a block on current matrix
     @param block_row Contains block row
     @param block_col Contains block column
     @param block Block to be inserted
	 */
	int AddBlock(const int block_row,const int block_col, const TPZFMatrix & block );
	
	//Coloca o bloco (block_row , block_col) dentro da matriz &target desde a posicao (row,col)
	
	/**
     @brief Inserts a block (block_row , block_col) on current matrix target
     @param block_row Contains block row
     @param block_col Contains block column
     @param block Block to be inserted
     @param row Starting row position
     @param col Starting column position
	 */
	int InsertBlock(const int block_row,const int block_col,
					const int row,const int col, TPZMatrix &target) const;
	
	TPZBlock &operator=(const TPZBlock & );
	
	/**
     @brief Prints a matrix block
     @param block_row Contains block row
     @param block_col Contains block column
     @param title Title on printed output device
     @param out Output device
	 */
	int  PrintBlock(const int block_row,const int block_col,const char *title = "",TPZostream &out = std::cout ) const;
	
	//Imprime todos os blocos da matriz
	/// Prints all the blocks of the matrix
	void Print(const char *title = "",TPZostream &out = std::cout,TPZMatrix *mat=NULL);
	
	void PrintSolution(const char *title, TPZostream &out);
	
	//retorna o numero maximo de blocos na diagonal
	/**
     @brief Returns the max number of blocks on diagonal
	 */
	int MaxBlockSize() const {return fBlock.NElements();}
	/**
     @brief Returns number of blocks on diagonal
	 */
	int NBlocks() const {return fBlock.NElements();}
	
	//retorna a dimensao do bloco
	/**
     @brief Returns block dimension
     @param block_diagonal Inquired block_diagonal
	 */
	int Size(const int block_diagonal) const { return fBlock[block_diagonal].dim; }
	
	//**retorna a posicao do primeiro elemento bloco, relativo � diagonal da matriz*/
	/**
     @brief Returns the position of first element block dependent on matrix diagonal
     @param block_diagonal Inquired block_diagonal
	 */
	int Position(const int block_diagonal) const { return fBlock[block_diagonal].pos;}
	
	//Retorna a dimensao da matriz que o bloco esta apontando
	/**
     @brief Returns matrix dimension pointed by block
	 */
	int Dim() const {return fBlock.NElements() ? fBlock[fBlock.NElements()-1].pos+fBlock[fBlock.NElements()-1].dim : 0; }
	
	/**
	 * @brief returns the unique identifier for reading/writing objects to streams
	 */
	virtual int ClassId() const;
	/**
	 @brief Save the element data to a stream
	 */
	virtual void Write(TPZStream &buf, int withclassid);
	
	/**
	 @brief Read the element data from a stream
	 */
	virtual void Read(TPZStream &buf, void *context);
	
	
private:
	
	//static int Error(const char *msg );  //, char *msg2
	
	/**
     @struct TNode
     @brief Defines a node
	 */
	struct TNode
	{
		int pos; /**< Position of node */
		int dim; /**< Dimension of node */
		
		TNode() {
			pos=0;
			dim=0;
		}
		void Read(TPZStream &buf, void *context);
		void Write(TPZStream &buf, void *context);
	};
	
	/**
     @brief Nodes vector
	 */
	TPZManVector<TNode>    fBlock;
	/**
     @brief Pointer to TPZMatrix
	 */
	TPZMatrix *fpMatrix;
	static REAL gZero;//zero
	
};

#endif

