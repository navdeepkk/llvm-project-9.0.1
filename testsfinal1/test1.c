#include<stdio.h>

void main()
{
  int x=3;
  /*foo is a nested function
  i.e. the below syntax is equivalent to :
  void foo(){
  printf("Value of x inside nested function foo = %d\n",x);
  x=4;
  }
  */
	 foo:{
				printf("Value of x inside nested function foo = %d\n",x);
				x=4;
	 }
	 printf("Value of x before calling nested function foo = %d\n",x);/*should print 3 after transformation*/
	 foo();
	 printf("Value of x after calling nested function foo = %d\n",x);/*should print 4 after transformation*/

 }
