/**
 * @file
 * @brief Contains the implementation of the TPZRefCube methods. 
 */
#include "TPZGeoCube.h"
#include "TPZRefCube.h"
#include "pzshapecube.h"
#include "TPZGeoElement.h"
#include "pzgeoelside.h"
#include "pzgeoel.h"

using namespace pzshape;
using namespace std;

namespace pzrefine {
	static int nsubeldata[27] = {1,1,1,1,1,1,1,1,3,3,3,3,3,3,3,3,3,3,3,3,9,9,9,9,9,9,26};
	
	static int subeldata[27][26][2] = {
		/*00*/{{0,0}},
		/*01*/{{1,1}},
		/*02*/{{2,2}},
		/*03*/{{3,3}},
		/*04*/{{4,4}},
		/*05*/{{5,5}},
		/*06*/{{6,6}},
		/*07*/{{7,7}},
		/*08*/{{0,1},{0,8},{1,8}},
		/*09*/{{1,2},{1,9},{2,9}},
		/*10*/{{2,3},{2,10},{3,10}},
		/*11*/{{0,3},{0,11},{3,11}},
		/*12*/{{0,4},{0,12},{4,12}},
		/*13*/{{1,5},{1,13},{5,13}},
		/*14*/{{2,6},{2,14},{6,14}},
		/*15*/{{3,7},{3,15},{7,15}},
		/*16*/{{4,5},{4,16},{5,16}},
		/*17*/{{5,6},{5,17},{6,17}},
		/*18*/{{6,7},{6,18},{7,18}},
		/*19*/{{4,7},{4,19},{7,19}},
		/*20*/{{0,2},{0,9},{0,10},{2,8},{2,11},{0,20},{1,20},{2,20},{3,20}},
		/*21*/{{0,5},{0,13},{0,16},{5,8},{5,12},{0,21},{1,21},{4,21},{5,21}},
		/*22*/{{1,6},{1,14},{1,17},{6,9},{6,13},{1,22},{2,22},{5,22},{6,22}},
		/*23*/{{2,7},{2,15},{2,18},{7,10},{7,14},{2,23},{3,23},{6,23},{7,23}},
		/*24*/{{0,7},{0,15},{0,19},{7,11},{7,12},{0,24},{3,24},{4,24},{7,24}},
		/*25*/{{4,6},{4,17},{4,18},{6,16},{6,19},{4,25},{5,25},{6,25},{7,25}},
		/*26*/{{0,14},{0,17},{0,18},{6,8},{6,11},{6,12},{0,26},{1,26},{2,26},{3,26},{4,26},{5,26},{6,26},
			{7,26},{0,22},{0,23},{0,25},{1,25},{2,21},{2,24},{2,25},{3,25},{4,22},{4,23},{6,21},{6,24}}
	};
	
	static int MidSideNodes[19][2]  = {
		{0,1},{1,2},{2,3},{3,0},{4,0},{5,1},
		{6,2},{7,3},{4,5},{5,6},{6,7},{7,4},  //lados
		{0,2},{0,5},{1,6},{2,7},{0,7},{4,6},  //faces
		{0,6} };//centro
	
	static REAL MidCoord[19][3] = { {0.,-1.,-1.},{1.,0.,-1.},{0.,1.,-1.},{-1.,0.,-1.},{-1.,-1.,0.},{1.,-1.,0.},
		{1.,1.,0.},{-1.,1.,0.},{0.,-1.,1.},{1.,0.,1.},{0.,1.,1.},{-1.,0.,1.},
		{0.,0.,-1.},{0.,-1.,0.},{1.,0.,0.},{0.,1.,0.},{-1.,0.,0.},{0.,0.,1.},{0.,0.,0.} };
	
	/**
	 * define as conectividades entre sub-elementos
	 * linha i � filho i, {a,b,c} = {lado do filho atual,
	 * irm�o vizinho,lado do vizinho}
	 */
	const int NumInNeigh = 19;
	static int InNeigh[8][NumInNeigh][3] = {
		{{1,1,0},{2,1,3},{3,3,0},{4,4,0},{5,1,4},{6,1,7},{7,3,4},{9,1,11},{10,3,8},{13,1,12},{14,1,15},{15,3,12},{16,4,8},{17,1,19},{18,3,16},{19,4,11},{22,1,24},{23,3,21},{25,4,20}},
		{{0,0,1},{2,2,1},{3,2,0},{4,5,0},{5,5,1},{6,2,5},{7,2,4},{10,2,8},{11,0,9},{12,0,13},{14,2,13},{15,2,12},{16,5,8},{17,5,9},{18,2,16},{19,5,11},{23,2,21},{24,0,22},{25,5,20}},
		{{0,3,1},{1,1,2},{3,3,2},{4,3,5},{5,6,1},{6,6,2},{7,6,3},{8,1,10},{11,3,9},{12,3,13},{13,1,14},{15,3,14},{16,6,8},{17,6,9},{18,6,10},{19,6,11},{21,1,23},{24,3,22},{25,6,20}},
		{{0,0,3},{1,0,2},{2,2,3},{4,7,0},{5,4,2},{6,2,7},{7,7,3},{8,0,10},{9,2,11},{12,0,15},{13,0,14},{14,2,15},{16,7,8},{17,2,19},{18,7,10},{19,7,11},{21,0,23},{22,2,24},{25,7,20}},
		{{0,0,4},{1,0,5},{2,5,3},{3,0,7},{5,5,4},{6,5,7},{7,7,4},{8,0,16},{9,0,17},{10,0,18},{11,0,19},{13,5,12},{14,5,15},{15,7,12},{17,5,19},{18,7,16},{20,0,25},{22,5,24},{23,7,21}},
		{{0,4,1},{1,1,5},{2,1,6},{3,6,0},{4,4,5},{6,6,5},{7,6,4},{8,1,16},{9,1,17},{10,1,18},{11,4,9},{12,4,13},{14,6,13},{15,6,12},{18,6,16},{19,4,17},{20,1,25},{23,6,21},{24,4,22}},
		{{0,7,1},{1,5,2},{2,2,6},{3,7,2},{4,7,5},{5,5,6},{7,7,6},{8,5,10},{9,2,17},{10,2,18},{11,7,9},{12,7,13},{13,5,14},{15,7,14},{16,5,18},{19,7,17},{20,2,25},{21,5,23},{24,7,22}},
		{{0,4,3},{1,0,6},{2,3,6},{3,3,7},{4,4,7},{5,4,6},{6,6,7},{8,4,10},{9,3,17},{10,3,18},{11,3,19},{12,4,15},{13,4,14},{14,6,15},{16,4,18},{17,6,19},{20,3,25},{21,4,23},{22,6,24}} };
	
	/**
	 * define os cantos locais dos fihos
	 */
	static int CornerSons[8][8] = {
		{0,8,20,11,12,21,26,24},{8,1,9,20,21,13,22,26},{20,9,2,10,26,22,14,23},
		{11,20,10,3,24,26,23,15},{12,21,26,24,4,16,25,19},{21,13,22,26,16,5,17,25},
		{26,22,14,23,25,17,6,18},{24,26,23,15,19,25,18,7} };
	
	static REAL buildt[8][27][4][3] = {//por colunas
		/*S0*/{
			/*00*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{-1.,-1.,-1.}},
			/*01*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*02*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*03*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*04*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*05*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*06*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*07*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*08*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*09*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*10*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*11*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*12*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*13*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*14*/{{0.,0.,0.5},{0.,0.,0.},{0.,0.,0.},{0.,0.,-0.5}},
			/*15*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*16*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*17*/{{0.,0.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-0.5,0.}},
			/*18*/{{-0.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-0.5,0.,0.}},
			/*19*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*20*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,-.5,0.}},
			/*21*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,-.5,0.}},
			/*22*/{{0.,0.5,0.},{0.,0.,0.5},{0.,0.,0.},{0.,-0.5,-0.5}},
			/*23*/{{0.5,0.,0.},{0.,0.,0.5},{0.,0.,0.},{-0.5,0.,-0.5}},
			/*24*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,-.5,0.}},
			/*25*/{{0.5,0.,0.},{0.,0.5,0.},{0.,0.,0.},{-0.5,-0.5,0.}},
			/*26*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,.5},{-.5,-.5,-.5}}},
		/*S1*/{
			/*00*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*01*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{1,-1,-1}},
			/*02*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*03*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*04*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*05*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*06*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*07*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*08*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*09*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*10*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*11*/{{0.,-.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*12*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*13*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*14*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*15*/{{0.,0.,0.5},{0.,0.,0.},{0.,0.,0.},{0.,0.,-0.5}},
			/*16*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*17*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*18*/{{-0.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.5,0.,0.}},
			/*19*/{{0.,-0.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-0.5,0.}},
			/*20*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,-.5,0.}},
			/*21*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,-.5,0.}},
			/*22*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,-.5,0.}},
			/*23*/{{0.5,0.,0.},{0.,0.,0.5},{0.,0.,0.},{0.5,0.,-0.5}},
			/*24*/{{0.,0.5,0.},{0.,0.,0.5},{0.,0.,0.},{0.,-0.5,-0.5}},
			/*25*/{{0.5,0.,0.},{0.,0.5,0.},{0.,0.,0.},{0.5,-0.5,0.}},
			/*26*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,.5},{.5,-.5,-.5}}},
		/*S2*/{
			/*00*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*01*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*02*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{1,1,-1}},
			/*03*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*04*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*05*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*06*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*07*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*08*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*09*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*10*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*11*/{{0.,-.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*12*/{{0.,0.,0.5},{0.,0.,0.},{0.,0.,0.},{0.,0.,-0.5}},
			/*13*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*14*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*15*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*16*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*17*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*18*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*19*/{{0.,-0.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.5,0.}},
			/*20*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,.5,0.}},
			/*21*/{{0.5,0.,0.},{0.,0.,0.5},{0.,0.,0.},{0.5,0.,-0.5}},
			/*22*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,-.5,0.}},
			/*23*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,-.5,0.}},
			/*24*/{{0.,0.5,0.},{0.,0.,0.5},{0.,0.,0.},{0.,0.5,-0.5}},
			/*25*/{{0.5,0.,0.},{0.,0.5,0.},{0.,0.,0.},{0.5,0.5,0.}},
			/*26*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,.5},{.5,.5,-.5}}},
		/*S3*/{
			/*00*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*01*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*02*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*03*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{-1,1,-1}},
			/*04*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*05*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*06*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*07*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*08*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*09*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*10*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*11*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*12*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*13*/{{0.,0.,.5},{0.,0.,0.},{0.,0.,0.},{0.,0.,-.5}},
			/*14*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*15*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*16*/{{0.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-0.5,0.,0.}},
			/*17*/{{0.,0.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.5,0.}},
			/*18*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*19*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*20*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,.5,0.}},
			/*21*/{{0.5,0.,0.},{0.,0.,0.5},{0.,0.,0.},{-0.5,0.,-0.5}},
			/*22*/{{0.,0.5,0.},{0.,0.,0.5},{0.,0.,0.},{0.,0.5,-0.5}},
			/*23*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,-.5,0.}},
			/*24*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,-.5,0.}},
			/*25*/{{0.5,0.,0.},{0.,0.5,0.},{0.,0.,0.},{-0.5,0.5,0.}},
			/*26*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,.5},{-.5,.5,-.5}}},
		/*S4*/{
			/*00*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*01*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*02*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*03*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*04*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{-1,-1,1}},
			/*05*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*06*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*07*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*08*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*09*/{{0.,0.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-0.5,0.}},
			/*10*/{{-0.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-0.5,0.,0.}},
			/*11*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*12*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*13*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*14*/{{0.,0.,0.5},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.5}},
			/*15*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*16*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*17*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*18*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*19*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*20*/{{0.5,0.,0.},{0.,0.5,0.},{0.,0.,0.},{-0.5,-0.5,0.}},
			/*21*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,.5,0.}},
			/*22*/{{0.,0.5,0.},{0.,0.,0.5},{0.,0.,0.},{0.,-0.5,0.5}},
			/*23*/{{0.5,0.,0.},{0.,0.,0.5},{0.,0.,0.},{-0.5,0.,0.5}},
			/*24*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,.5,0.}},
			/*25*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,-.5,0.}},
			/*26*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,.5},{-.5,-.5,.5}}},
		/*S5*/{
			/*00*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*01*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*02*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*03*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*04*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*05*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{1,-1,1}},
			/*06*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*07*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*08*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*09*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*10*/{{-0.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.5,0.,0.}},
			/*11*/{{0.,-0.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-0.5,0.}},
			/*12*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*13*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*14*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*15*/{{0.,0.,0.5},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.5}},
			/*16*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*17*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*18*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*19*/{{0.,-.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,-.5,0.}},
			/*20*/{{0.5,0.,0.},{0.,0.5,0.},{0.,0.,0.},{0.5,-0.5,0.}},
			/*21*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,.5,0.}},
			/*22*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,.5,0.}},
			/*23*/{{0.5,0.,0.},{0.,0.,0.5},{0.,0.,0.},{0.5,0.,0.5}},
			/*24*/{{0.,0.5,0.},{0.,0.,0.5},{0.,0.,0.},{0.,-0.5,0.5}},
			/*25*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,-.5,0.}},
			/*26*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,.5},{.5,-.5,.5}}},
		/*S6*/{
			/*00*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*01*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*02*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*03*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*04*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*05*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*06*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{1,1,1}},
			/*07*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*08*/{{0.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.5,0.,0.}},
			/*09*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*10*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*11*/{{0.,-0.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.5,0.}},
			/*12*/{{0.,0.,0.5},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.5}},
			/*13*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*14*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*15*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*16*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*17*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*18*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*19*/{{0.,-.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*20*/{{0.5,0.,0.},{0.,0.5,0.},{0.,0.,0.},{0.5,0.5,0.}},
			/*21*/{{0.5,0.,0.},{0.,0.,0.5},{0.,0.,0.},{0.5,0.,0.5}},
			/*22*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,.5,0.}},
			/*23*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,.5,0.}},
			/*24*/{{0.,0.5,0.},{0.,0.,0.5},{0.,0.,0.},{0.,0.5,0.5}},
			/*25*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,.5,0.}},
			/*26*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,.5},{.5,.5,.5}}},
		/*S7*/{
			/*00*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*01*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*02*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*03*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*04*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*05*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*06*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}},
			/*07*/{{0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{-1,1,1}},
			/*08*/{{0.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-0.5,0.,0.}},
			/*09*/{{0.,0.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.5,0.}},
			/*10*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*11*/{{-.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*12*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*13*/{{0.,0.,0.5},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.5}},
			/*14*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*15*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*16*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*17*/{{0.,.5,0.},{0.,0.,0.},{0.,0.,0.},{0.,.5,0.}},
			/*18*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{.5,0.,0.}},
			/*19*/{{.5,0.,0.},{0.,0.,0.},{0.,0.,0.},{-.5,0.,0.}},
			/*20*/{{0.5,0.,0.},{0.,0.5,0.},{0.,0.,0.},{-0.5,0.5,0.}},
			/*21*/{{0.5,0.,0.},{0.,0.,0.5},{0.,0.,0.},{-0.5,0.,0.5}},
			/*22*/{{0.,0.5,0.},{0.,0.,0.5},{0.,0.,0.},{0.,0.5,0.5}},
			/*23*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,.5,0.}},
			/*24*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{.5,.5,0.}},
			/*25*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,0.},{-.5,.5,0.}},
			/*26*/{{.5,0.,0.},{0.,.5,0.},{0.,0.,.5},{-.5,.5,.5}}}
	};
	
	static int fatherside[8][27] = {
		/*00*/{0,8,20,11,12,21,26,24,8,20,20,11,12,21,26,24,21,26,26,24,20,21,26,26,24,26,26},
		/*01*/{8,1,9,20,21,13,22,26,8,9,20,20,21,13,22,26,21,22,26,26,20,21,22,26,26,26,26},
		/*02*/{20,9,2,10,26,22,14,23,20,9,10,20,26,22,14,23,26,22,23,26,20,26,22,23,26,26,26},
		/*03*/{11,20,10,3,24,26,23,15,20,20,10,11,24,26,23,15,26,26,23,24,20,26,26,23,24,26,26},
		/*04*/{12,21,26,24,4,16,25,19,21,26,26,24,12,21,26,24,16,25,25,19,26,21,26,26,24,25,26},
		/*05*/{21,13,22,26,16,5,17,25,21,22,26,26,21,13,22,26,16,17,25,25,26,21,22,26,26,25,26},
		/*06*/{26,22,14,23,25,17,6,18,26,22,23,26,26,22,14,23,25,17,18,25,26,26,22,23,26,25,26},
		/*07*/{24,26,23,15,19,25,18,7,26,26,23,24,24,26,23,15,25,25,18,19,26,26,26,23,24,25,26},
	};
	
	//Into Divides is necesary to consider the connectivity with the all neighboards
	void TPZRefCube::Divide(TPZGeoEl *geo,TPZVec<TPZGeoEl *> &SubElVec) {
		
		int i;
		if(geo->HasSubElement()) {
			SubElVec.Resize(NSubEl);
			for(i=0;i<NSubEl;i++) SubElVec[i] = geo->SubElement(i);
			return;//If exist fSubEl return this sons
		}
		int j,sub,matid=geo->MaterialId();
		long index;
		int np[TPZShapeCube::NSides];//guarda conectividades dos 8 subelementos
		
		for(j=0;j<TPZShapeCube::NCornerNodes;j++) np[j] = geo->NodeIndex(j);
		for(sub=TPZShapeCube::NCornerNodes;sub<TPZShapeCube::NSides;sub++) {
			NewMidSideNode(geo,sub,index);
			np[sub] = index;
		}
		// creating new subelements
		for(i=0;i<TPZShapeCube::NCornerNodes;i++) {
			TPZManVector<long>cornerindexes(TPZShapeCube::NCornerNodes);
			for(int j=0;j<TPZShapeCube::NCornerNodes;j++) cornerindexes[j] = np[CornerSons[i][j]];
			long index;
			TPZGeoEl *subel = geo->Mesh()->CreateGeoElement(ECube,cornerindexes,matid,index);
			geo->SetSubElement(i , subel);
		}
		
		SubElVec.Resize(NSubEl);
		for(sub=0;sub<NSubEl;sub++) {
			SubElVec[sub] = geo->SubElement(sub);
			SubElVec[sub]->SetFather(geo);
			SubElVec[sub]->SetFather(geo->Index());
		}
		for(i=0;i<NSubEl;i++) {//conectividades entre os filhos : viz interna
			for(j=0;j<NumInNeigh;j++) {        //lado do subel                                          numero do filho viz.             lado do viz.
				geo->SubElement(i)->SetNeighbour(InNeigh[i][j][0],TPZGeoElSide(geo->SubElement(InNeigh[i][j][1]),InNeigh[i][j][2]));
			}
		}
		
		geo->SetSubElementConnectivities();
	}
	
	void TPZRefCube::NewMidSideNode(TPZGeoEl *gel,int side,long &index) {
		
		MidSideNodeIndex(gel,side,index);
		if(index < 0) {
			TPZGeoElSide gelside = gel->Neighbour(side);
			if(gelside.Element()) {
				while(gelside.Element() != gel) {
					gelside.Element()->MidSideNodeIndex(gelside.Side(),index);
					if(index!=-1) return;
					gelside = gelside.Neighbour();
				}
			}
			TPZVec<REAL> par(3,0.);
			TPZVec<REAL> coord(3,0.);
			if(side < TPZShapeCube::NCornerNodes) {
				index = gel->NodeIndex(side); 
				return;
			}
			//aqui side = 8 a 26
			side-=TPZShapeCube::NCornerNodes;//0,1,..,18
			par[0] = MidCoord[side][0];
			par[1] = MidCoord[side][1];
			par[2] = MidCoord[side][2];
			gel->X(par,coord);
			index = gel->Mesh()->NodeVec().AllocateNewElement();
			gel->Mesh()->NodeVec()[index].Initialize(coord,*gel->Mesh());
		}
	}
	
	void TPZRefCube::MidSideNodeIndex(const TPZGeoEl *gel,int side,long &index) {
		index = -1;
		if(side<0 || side>TPZShapeCube::NSides-1) {
			PZError << "TPZRefCube::MidSideNodeIndex. Bad parameter side = " << side << endl;
			return;
		}
		//sides 0 a 7
		if(side<TPZShapeCube::NCornerNodes) {//o n� medio do lado 0 � o 0 etc.
			index = (gel)->NodeIndex(side);
			return; 
		}
		//o n� medio da face � o centro e o n� medio do centro � o centro
		//como n� de algum filho se este existir
		//caso tenha filhos � o canto de algum filho, se n�o tiver filhos retorna -1
		if(gel->HasSubElement()) {
			side-=TPZShapeCube::NCornerNodes;
			index=(gel->SubElement(MidSideNodes[side][0]))->NodeIndex(MidSideNodes[side][1]);
		}
	}
	
	void TPZRefCube::GetSubElements(const TPZGeoEl *father,int side, TPZStack<TPZGeoElSide> &subel){
		
		subel.Resize(0);
		if(side<0 || side>TPZShapeCube::NSides || !father->HasSubElement()){
			PZError << "TPZRefCube::GetSubelements2 called with error arguments\n";
			return;
		}
		int nsub = NSideSubElements(side);//nsubeldata[side];
		for(int i=0;i<nsub;i++)
			subel.Push(TPZGeoElSide(father->SubElement(subeldata[side][i][0]),
									subeldata[side][i][1]));
	}
	
	int TPZRefCube::NSideSubElements(int side) {  
		if(side<0 || side>TPZShapeCube::NSides-1){
			PZError << "TPZRefCube::NSideSubelements2 called with error arguments\n";
			return -1;
		}
		return nsubeldata[side];
	}
	
	//int TPZRefCube::NSideSubElements(int side) {
	//  if(side < 0 || side > 26) {
	//    PZError << "TPZRefCube::NSideSubElements called for side " << side << endl;
	//    return 0;
	//  }
	//  if(side==26) return 8;//centro
	//  if(side>19 && side<26) return 4;//faces
	//  if(side>7) return 2;//lados
	//  return 1;//cantos
	//}
	
	
	TPZTransform TPZRefCube::GetTransform(int side,int whichsubel){
		
		if(side<0 || side>TPZShapeCube::NSides-1){
			PZError << "TPZRefCube::GetTransform side out of range or father null\n";
			return TPZTransform(0,0);
		}
		int smalldim = TPZShapeCube::SideDimension(side);
		int fatherside = FatherSide(side,whichsubel);
		int largedim = TPZShapeCube::SideDimension(fatherside);
		TPZTransform trans(largedim,smalldim);
		int i,j;
		for(i=0; i<largedim; i++) {
			for(j=0; j<smalldim; j++) {
				trans.Mult()(i,j) = buildt[whichsubel][side][j][i];
			}
			trans.Sum() (i,0) = buildt[whichsubel][side][3][i];
		}
		return trans;
	}
	
	int TPZRefCube::FatherSide(int side,int whichsubel){
		
		if(side<0 || side>TPZShapeCube::NSides-1){
			PZError << "TPZRefCube::Father2 called error" << endl;
			return -1;
		}
		return fatherside[whichsubel][side];
	}
	
};
