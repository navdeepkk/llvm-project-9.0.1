struct s_foo31 {
int  * b;
struct s * ss;
struct s  * arr;
};

struct s{

	int i;
	int a[5][5];
};

void foo31( struct s_foo31*);


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





int main(){
	struct s ss, arr[5];
	static int b[20][20];																					//use with storage class static.
	
struct s_foo31 sfoo31;
sfoo31.b = &b;
sfoo31.ss = &ss;
sfoo31.arr = &arr;
																															//initialized and then printed.
	foo31(&sfoo31);	
}


void foo31( struct s_foo31* __s ){

			(*(__s->ss)).i = 0;
			for((*(__s->ss)).i = 0; (*(__s->ss)).i < 5; (*(__s->ss)).i++){
				(*(__s->ss)).a[(*(__s->ss)).i][(*(__s->ss)).i] = (*(__s->ss)).i;																//initializes array 'a' in 'ss'.
				(__s->b)[(*(__s->ss)).i * 20 + (*(__s->ss)).i] = 20;
			}
			
			for((*(__s->ss)).i = 0; (*(__s->ss)).i < 5; (*(__s->ss)).i++){
				printf("%d ",(*(__s->ss)).a[(*(__s->ss)).i][(*(__s->ss)).i]);													//prints out array 'a' in 'ss'.
				printf("%d ",(__s->b)[(*(__s->ss)).i * 20 + (*(__s->ss)).i]);
			}
			printf("\n");
			
			(__s->arr)[0].i = 1;																							//initializes arr which is an array of struct s.
			(__s->arr)[0].a[0][0] = (__s->arr)[0].i;
			printf("%d\n ", (__s->arr)[0].a[0][0]);													//array arr is an array of structures which is
	}

