//---------------------------------------------//
// This test case shows the ability of code to //
// handle pointers(representing arrays). here a//
// 2-d array is allocated using malloc and then//
// they are assigned values. in the subsequent //
// funcion print the initilaized array is print//
// ed.																				 //
// EXPECTED OUTPUT														 //
// 0 1 2																			 //
// 1 2 3																			 //
// 2 3 4																			 //
// 3 4 5																			 //
//---------------------------------------------//  

#include<stdio.h>
#include<stdlib.h>

int main(){
	int i, j, m = 4, n = 3;
	int **arr = (int **)malloc(m * sizeof(int *));
	label:{		
		for(i = 0; i < m; i++){
			arr[i] = (int *) malloc(sizeof(int) * n);										//creating a 2-d array using array of pointer.
		}																															// the varible has been captured from the enclosing scope.	
		for(i = 0;i<m;i++)
		{
			for(j=0;j<n;j++)
			{
				arr[i + 0][j + 0] = i+j;																	//randomly assigning values to array elements.
			}
		}
	}


	label();


	print:{	
		for( i = 0; i < m ; i++){
			for( j = 0; j< n; j++  ){
				printf("%d ", arr[i][j]);																	//printing out values assigned to 
			}
			printf("\n");
		}
	}
	print();	
	return 0;
}
