//------------------------------------------//
// This exapmle shows  structures containing//
// arrays, and also arrays of structures.   //
// the array in struct s arr is being assi  //
// gned values in the first for loop and in	//
// second iteration it prints out the assign//
// values. in the third segment the array of//
// structures is assigned values and then th//
// ey are printed. the example also show use//
// of storage classes such as static for var//
// iables in particular the array b here.	  //
// EXPECTED OUTPUT													// 
// 0 20 1 20 2 20 3 20 4 20									//
// 1																				//
//------------------------------------------//

#include<stdio.h>

struct s{
	int i;
	int a[5][5];
};


int main(){
	struct s ss, arr[5];
	static int b[20][20];																					//use with storage class static.
	foo:{
			ss.i = 0;
			for(ss.i = 0; ss.i < 5; ss.i++){
				ss.a[ss.i][ss.i] = ss.i;																//initializes array 'a' in 'ss'.
				b[ss.i][ss.i] = 20;
			}
			
			for(ss.i = 0; ss.i < 5; ss.i++){
				printf("%d ",ss.a[ss.i][ss.i]);													//prints out array 'a' in 'ss'.
				printf("%d ",b[ss.i][ss.i]);
			}
			printf("\n");
			
			arr[0].i = 1;																							//initializes arr which is an array of struct s.
			arr[0].a[0][0] = arr[0].i;
			printf("%d\n ", arr[0].a[0][0]);													//array arr is an array of structures which is
	}																															//initialized and then printed.
	foo();	
}
