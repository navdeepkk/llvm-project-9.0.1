//------------------------------------------//
//This test case shows that qualifiers types//
//are handeled appropriately for any data		//
//type  and are passed in relevant scope	  //
//structures. To see that the tool actually //
//does this the output file generated should//
//be obeserved, in the scopes structures the//
//qual types will also be present.					// 
// EXPECTED OUTPUT													//
// 1																				//
//------------------------------------------//

#include<stdio.h>

int main(){
	static int x;																					//this is a storage class hence this should not be passed on to the scope structure.
		foo:{
				struct point{
					int x, y;
						struct loc{
						int x;
					}l1,l2[10];	
					struct loc l3,l4;
				}p1;
			const struct point p2[10], p7;										//this is a qualifier type and it should be passes on to the scope structure.
			
			bar:{
				printf("static x is: %d\n", x);
			}
			bar();
		}

		foo();
	return 0;
}
