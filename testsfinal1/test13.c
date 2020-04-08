/*
This test case checks indirect recursion.
fun1 and fun2 are two nested functions.
fun2() is nested inside fun1().

fun1() calls fun2()
fun2() calls fun1()

Both increment and print variable n.

Expected output:
1 2 3 4 5 6 7 8 9 10
*/

#include<stdio.h>

int main()
{
   int n = 1;
   fun1:{
	   if(n<=10)
           {
               printf("%d ", n);
               n++;

               fun2:{
                        if(n <=10)
			{
			   printf("%d ",n);
			   n++;
			   fun1();
			}
			else
			{
			   return;
			}
		     }
		fun2();
	   }
	   else
	   {
		return;
	   }
   }
   fun1();
   return 0;

}

