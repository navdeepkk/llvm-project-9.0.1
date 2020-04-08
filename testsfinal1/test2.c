/*
In addition, does your implementation handle recursively nested function blocks?

This test case checks if variable is resolved to it's corresponding declaration as per static scoping rules
for arbitrary depth.

More specifically it tests following cases:
Case 1: Variable used at depth 2 from variable declared in label at depth 2.(From any depth to any depth)
Case 2: Variable used at depth 7(randomly chosen to test if large depth is possible) from variable declared in function.

Case 1 output: 
value of a before call to f4: 6
value of a after call to f4: 14
Case 2 output: 
value of a before call to f1 : 5
value of a after call to f1 : 50
*/

#include<stdio.h>

/*commenting following define won't run that test case and also won't generate code for that test case*/
#define C1
#define C2

//case 1 :  At depth 2 from variable declared in label at depth 2.(From any depth to any depth)
#ifdef C1
void case1()
{
	int a = 2;
	printf("\nCase 1 output: ");
	f2:{
		int a = 5;
	
		f3:{
			int a = 6;
			
			f4:{
				a++;         		// resolves to a in f3.
				f5:{
					a *= 2;		// resolves to a in f3.
				   }
				f5();
			}
			
			printf("\nvalue of a before call to f4: %d", a);
			f4();
			printf("\nvalue of a after call to f4: %d", a);
		}
		f3();
	}
	f2();
}
#endif

//case 2: At depth 7(randomly chosen to test if any depth is possible) from variable declared in function.
#ifdef C2
void case2()
{
	int a = 5;
	printf("\nCase 2 output: ");
	
	f1:{
		f2:{
			f3:{
				f4:{
					f5:{
						f6:{
							f7:{
								a = a*10;
							}
							f7();
						   }
						   f6();
					   }
					   f5();
				   }
				   f4();
			   }
			   f3();
		   }
		   f2();
	   }

	   printf("\nvalue of a before call to f1 : %d",a);
	   f1();
	   printf("\nvalue of a after call to f1 : %d",a);
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
