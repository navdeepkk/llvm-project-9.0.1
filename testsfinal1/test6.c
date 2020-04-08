/*
This test case checks if rewriting for array variable for following cases 
work correctly inside nested function.
a. Handling of multidimensional arrays. 
b. array as index of array  e.g. arr[b[i]][b[j]] = 8
c. expressions as index of array e.g a[i + 2j][i*j] = 4

Expected Output:
0 0 0 0 0 0 
0 1 0 0 0 0 
0 0 2 0 0 0 
0 0 0 0 0 0 
0 0 0 0 4 0 
0 0 0 0 0 0 

arr[j-i][j*i] = 7
*/


#include<stdio.h>

int main()
{
  int a[5] = {0,1,2,0,4};
  int i,j,k,l;
  f1:{
        int b[3][6][6];
 	int p;

	print:{
		for(j=0;j<1;j++)
		{
		   for(k=0;k<6;k++)
		   {
		      for(l=0;l<6;l++)
		      {
		         if(p == 1)
			 printf("%d ",b[j][k][l]);
			 else
			 b[j][k][l] = 0;
		      }
		      printf("\n");
           	   }
		printf("\n \n");
		}
	 }
		
	//Initialize
	p = 0;
	print();
	
        for(i=0;i<5;i++)
	{
	   b[0][a[i]][a[i]] = a[i]; 
	}
	
	//print		
	p = 1;	
	print();

   }

   f1();

   int arr[2][3];
   i = 1;
   j = 2;
   arr[1][2] = 7;
   f2:{
	 printf("arr[j-i][j*i] = %d\n",arr[j-i][j*i]);
      }
   f2();
}

