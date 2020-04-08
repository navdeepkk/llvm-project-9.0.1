#include<stdio.h>

/*
Test case for:
If I have a local captured variable (i.e. the var is defined inside the function and used inside the closure) and a global var with same name, will your code still work?
*/

//Case 1: Resolution to global variable when none of the outer function contains declaration.

int x = 0;

void case1()
{
	x++;
	printf("value of global x = %d",x); //local x is not declared yet, so this still points to global x.
	int x = 7;
	f1:{
		f2:{
			   x = x + 2;				     //use in f2: refers to local x
		   }
		   
		   printf("\nvalue of local x before call: %d",x);  //use in f1: refers to local x
		   f2();
	   	   printf("\nvalue of local x after call: %d", x);  //use in f1: refers to local x
	   }
	   f1();
}



int main()
{
	case1();
}
