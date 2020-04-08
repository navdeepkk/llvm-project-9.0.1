/*
This test case checks if captured variable can passed as arguments to other functions.

More specifically it tests following cases:
Case 1: Passing value of variable to a function call at depth 2 from variable declaration.  _square(x)
Case 2: Passing address of variable to a function call at depth 2 from variable declaration.        square(&x)

Expected output:
Case 1 output: 
value of a before call to f1: 5
value of a after call to f1: 25
Case 2 output: 
value of a before call to f1: 5
value of a after call to f1: 25

*/

#include<stdio.h>

/*commenting following define won't run that test case and also won't generate code for that test case*/
#define C1
#define C2

//Case 1: Passing variable at depth 2 from variable declaration as call by value.
#ifdef C1
int _square(int x)
{
	return (x*x);
}

void case1()
{
	int a = 5;
	printf("\nCase 1 output: ");
	f1:{
		   f2:{
			      a = _square(a);
		      }
	   	f2();
	   }

	   printf("\nvalue of a before call to f1: %d", a);
	   f1();
	   printf("\nvalue of a after call to f1: %d", a);
}
#endif

//Case 2: Passing address of variable at depth 2 from variable declaration.
#ifdef C2
void square(int *x)
{
	*x = ((*x)*(*x));
}

void case2()
{
	int a = 5;
	printf("\nCase 2 output: ");
	f1:{
		   f2:{
			      square(&a);
		      }
	   	f2();
	   }

	   printf("\nvalue of a before call to f1: %d", a);
	   f1();
	   printf("\nvalue of a after call to f1: %d\n", a);
}
#endif

int main()
{
	#ifdef C1
	case1();
	#endif

	#ifdef C2
	case2();
	#endif
}
