/** standard C I/O functions */
#include <cstdio>

/** pz full matrix class */
#include "pzfmatrix.h"

#include <stdlib.h>

#include <sys/time.h>
/** util function - return wall clock time in seconds */
double mysecond()
{
    struct timeval tp;
    struct timezone tzp;
    gettimeofday(&tp,&tzp);
    return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
} /* mysecond */

using namespace std;

int main()
{
#ifdef USING_BLAS
    setenv("VECLIB_MAXIMUM_THREADS", "1", true);
#endif
    
    int n = 9;
    int m = 1;
    TPZFMatrix<double> A(m, n, 0.0);
    TPZFMatrix<double> B(n, n, 0.0);
    
    /** putting non-zeros values in the matrices */
    for (int i=0; i<m; i++) {
        for (int j=0; j<n; j++) {
            A(i, j) = ( i * n + j ) + 1.0;
        }
    }
    
    for (int i=0; i<n; i++) {
        for (int j=0; j<n; j++) {
            if (i == j) B(i, j) = 1.0; //( i * n + j ) * 0.2;
        }
    }
    
    
    cout << " ---- " << endl;

    double begin = mysecond();
    TPZFMatrix<double> C = A * B;
    double end = mysecond();
    

    A.Print(std::cout);
    B.Print(std::cout);
    C.Print(std::cout);
    
    for (int i=0; i<m; i++) {
        for (int j=0; j<n; j++) {
            if ( C(i,j) != A(i, j) ) {
                std::cout <<  C(i,j) << "\t" << A(i, j) << std::endl;
                std::cout << "wrong!" << std::endl;
                exit(1);
            }
        }
    }
    
    printf("Multiply Time: %.3lf\n", end-begin);
    
} /* main */