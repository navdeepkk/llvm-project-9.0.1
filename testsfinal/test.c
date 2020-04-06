
#include<stdio.h>
#include<stdlib.h>

int main(){
	int i, j, m = 4, n = 3;
	int **arr = (int **)malloc(m * sizeof(int *));
	label:{		
		for(i = 0; i < m; i++){
			arr[i] = (int *) malloc(sizeof(int) * n);
		}		
		for(i = 0;i<m;i++)
		{
			for(j=0;j<n;j++)
			{
				arr[i + 0][j + 0] = i+j;
			}
		}
	}


	label();


	print:{	
		for( i = 0; i < m ; i++){
			for( j = 0; j< n; j++  ){
				printf("%d ", arr[i][j]);
			}
			printf("\n");
		}
	}
	print();	
	return 0;
}
