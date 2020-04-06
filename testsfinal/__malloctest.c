struct s_label17 {
int* n;
int *** arr;
int* m;
int* i;
int* j;
};

struct s_print36 {
int* n;
int *** arr;
int* m;
int* i;
int* j;
};

void label17( struct s_label17*);
void print36( struct s_print36*);


//---------------------------------------------//
// This test case shows the ability of code to //
// handle pointers(representing arrays). here a//
// 2-d array is allocated using malloc and then//
// they are assigned values. in the subsequent //
// funcion print the initilaized array is print//
// ed.																				 //
// EXPECTED OUTPUT														 //
// 
#include<stdio.h>
#include<stdlib.h>

int main(){
	int i, j, m = 4, n = 3;
	int **arr = (int **)malloc(m * sizeof(int *));
	
struct s_label17 slabel17;
slabel17.n = &n;
slabel17.arr = &arr;
slabel17.m = &m;
slabel17.i = &i;
slabel17.j = &j;



	label17(&slabel17);


	
struct s_print36 sprint36;
sprint36.n = &n;
sprint36.arr = &arr;
sprint36.m = &m;
sprint36.i = &i;
sprint36.j = &j;

	print36(&sprint36);	
	return 0;
}


void label17( struct s_label17* __s ){
		
		for((*(__s->i)) = 0; (*(__s->i)) < (*(__s->m)); (*(__s->i))++){
			(*(__s->arr))[(*(__s->i))] = (int *) malloc(sizeof(int) * (*(__s->n)));
		}		
		for((*(__s->i)) = 0;(*(__s->i))<(*(__s->m));(*(__s->i))++)
		{
			for((*(__s->j))=0;(*(__s->j))<(*(__s->n));(*(__s->j))++)
			{
				(*(__s->arr))[(*(__s->i)) + 0][(*(__s->j)) + 0] = (*(__s->i))+(*(__s->j));
			}
		}
	}

void print36( struct s_print36* __s ){
	
		for( (*(__s->i)) = 0; (*(__s->i)) < (*(__s->m)) ; (*(__s->i))++){
			for( (*(__s->j)) = 0; (*(__s->j))< (*(__s->n)); (*(__s->j))++  ){
				printf("%d ", (*(__s->arr))[(*(__s->i))][(*(__s->j))]);
			}
			printf("\n");
		}
	}

