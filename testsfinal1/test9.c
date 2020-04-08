/*
This test case checks implementation of 2D array with the help of double pointer and malloc.
int **arr is declared in main function. 
While memory allocation and initialization of arr is done in nested function init().
And it is printed using nested function print.

This is other test case to check support of tool for pointer.

Expected output:
0 0 0 0 0 
0 1 2 3 4 
0 2 4 6 8

*/

#include<stdio.h>
#include<stdlib.h>

int main()
{
	  int i, j, row = 3 , col = 5;
	  int **arr = (int **)malloc(row * sizeof(int *));

		init:{		
				  for(i = 0; i < row; i++)
				  {
						 arr[i] = (int *) malloc(sizeof(int) * col);
				  }		

				  for(i = 0;i<row;i++)
				  {
							 for(j=0;j<col;j++)
							 {
										arr[i + 0][j + 0] = i*j;
							 }
				  }
		}

		init();
		print:{	
				  for( i = 0; i < row ; i++){
							 for( j = 0; j< col; j++  ){
										printf("%d ", arr[i][j]);
							 }
							 printf("\n");
				  }
		}
		print();	
		return 0;
}

