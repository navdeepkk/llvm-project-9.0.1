	/* Expected output:

Line no 34 Called global foo()
Line no 27 called foo/f1 
Line no 47 called main/foo .
Line no 43  called main/foo/f1
Line no 52 Called global foo()

This test case checks if call gets resolved to their appropriate function depending on their scope.
Case 1: Label with same name as another function.
Case 2: Label with same name as other label. 
*/


#include<stdio.h>

void foo()
{
	printf("Called global foo()\n");

	/*Label with same name as other label.*/
	f1:{
		   printf("called foo/f1 \n");
	   }

	printf("Line no 27 ");
	f1();
}


int main()
{
	printf("Line no 34 ");
	foo(); /*Gets resolved to global function.*/

	/*Label with same name as another function.*/
	{
           foo:{
		printf("called main/foo .\n");
		f1:{
			printf("called main/foo/f1 \n");
		}
		printf("Line no 43 ");
		f1();
	    }
	
	    printf("Line no 47 ");
	    foo();/*Gets resolved to local function.*/
        } /*scope of nested function ends here*/

        printf("Line no 52");		
	foo(); /*Should be resolved to global function.*/
}
