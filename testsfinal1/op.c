#include"labels_test13.c.h"
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
struct s_main my_s;

   int n = 1;
my_s.n = &n;

   
   main_fun1(&my_s);
   return 0;

}
void main_fun1_fun2(struct s_main_fun1 *par_s)
{
struct s_main_fun1_fun2 my_s;
my_s.s = par_s;

                        if((*(par_s->s->n)) <=10)
			{
			   printf("%d ",(*(par_s->s->n)));
			   (*(par_s->s->n))++;
			   main_fun1(par_s->s);
			}
			else
			{
			   return;
			}
		     }

void main_fun1(struct s_main *par_s)
{
struct s_main_fun1 my_s;
my_s.s = par_s;

	   if((*(par_s->n))<=10)
           {
               printf("%d ", (*(par_s->n)));
               (*(par_s->n))++;

               
		main_fun1_fun2(&my_s);
	   }
	   else
	   {
		return;
	   }
   }


