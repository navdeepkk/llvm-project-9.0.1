struct s_foo27 {
int  * a;
int * ij;
int * i;
};

struct s_bar30 {
struct s_foo27 *__s;
};

void foo27( struct s_foo27*);
void bar30( struct s_bar30*);


//------------------------------------------//
// Arrays require different type of handeling/
// as they are passed as single pointers to //
// label statements, so the array subscripts//
// are now to be re written keeping in mind //
// the dimensionality and size of the array.//
// This test case shows that arrays of any  //
// dimensions are supported						 and  //
// may be captured from any scope and also  //
// they may be indexed with any variable    //
// within its scope or from scopes captured //
// from enclosing blocks. in first call to  //
// foo it sets the elements of the array 'a'//
// and in second calls it prints out those  //
// elements.																//
// EXPECTED OUTPUT													//
// 5 5																			//
//------------------------------------------//
#include<stdio.h>

int b[5];
int main(){
				int ij;
				int i;
				int a[10][5][2];
        
struct s_foo27 sfoo27;
sfoo27.a = &a;
sfoo27.ij = &ij;
sfoo27.i = &i;

i = 0;
foo27(&sfoo27);																																							//call to set elements in array 'a'.
i = 1;
foo27(&sfoo27);																																							//call to print array 'a'.

        return 0;
}


void foo27( struct s_foo27* __s ){

							
struct s_bar30 sbar30;
sbar30.__s = __s;

	bar30(&sbar30);
}

void bar30( struct s_bar30* __s ){

							if((*(__s->__s->i)) == 0){
                for((*(__s->__s->ij)) = 0; (*(__s->__s->ij)) < 2; (*(__s->__s->ij))++){
                       (__s->__s->a)[(*(__s->__s->ij)) + (*(__s->__s->ij)) + (*(__s->__s->ij)) * 5 * 2 +  (*(__s->__s->ij)) + (*(__s->__s->ij)) + (*(__s->__s->ij)) * 2 +  (*(__s->__s->ij)) +(*(__s->__s->ij)) +(*(__s->__s->ij))] = 5;							//array 'a' is captured from the enclosing scope. array 'b' is global
												b[(*(__s->__s->ij)) + (*(__s->__s->ij))] = (*(__s->__s->ij));																						//and is used for indexing array 'a' further. Index 'ij' of 'b' is 
                }																																		//is also captured from scope of main.
							}
							else if((*(__s->__s->i)) == 1){
									for((*(__s->__s->ij)) = 0; (*(__s->__s->ij)) < 2; (*(__s->__s->ij))++){
									printf("%d ", (__s->__s->a)[b[(*(__s->__s->ij)) + (*(__s->__s->ij))] + (*(__s->__s->ij)) + (*(__s->__s->ij)) * 5 * 2 +  b[(*(__s->__s->ij)) + (*(__s->__s->ij))] + (*(__s->__s->ij)) + (*(__s->__s->ij)) * 2 +  b[(*(__s->__s->ij)) + (*(__s->__s->ij))] + (*(__s->__s->ij)) + (*(__s->__s->ij))]);
								}
								printf("\n");	
							}
            }

