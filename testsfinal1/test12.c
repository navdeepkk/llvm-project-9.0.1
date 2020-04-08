/*
This test case check recursion for nested functions.
fact is nested function which calls itself.
It calculates factorial of 6.

Expected output:
Factorial is 720.

*/

#include<stdio.h>

int main()
{
	int input = 6;
        int op = 1;

	fact:{
		 if(input <=1)
		 {
	            return;
		 }
		 else
		 {
		    op *= input;
		    input--;
		    fact();
		 }
	  }

	  fact();
	  printf("\nFactorial is %d.\n",op);
         
          return 0;
}

