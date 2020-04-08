#include<stdio.h>

int main()
{
	L1:{
			L2:{
					L1(); //resolves to outer L1
		
					L1:{
							printf("inner L1i\n");
						}

					L1(); //resolves to inner L1
	         }
      printf("Outer L1\n");

   L1(); //resolves to outer L1
}

					
