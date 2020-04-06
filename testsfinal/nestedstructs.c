//------------------------------------------//
// This test case demonstrates that structs //
// can be defined within other structs and  //
// can be handeled by the tool by renmaing. //
// renmaing has to be done of the variable	//
// declarations also of the renamed type. If//
// renaming is not done than there may be cl//
// ashes with some other structure present at/
// global level.														//
// EXPECTED OUTPUT													//
// distance: 0 x: 0 y: 0										//
// distance: 0 x: 1 y: 1										//
// distance: 0 x: 2 y: 2										//
// distance: 0 x: 3 y: 3										//
// distance: 0 x: 4 y: 4										//
//------------------------------------------//

#include<stdio.h>

struct loc{																	//this global struct 'loc' will clash with 'loc' defined inside struct 'point' 
	int a, b;																	//when it is hoisted to a global level. so the struct 'loc' in points is to be
};																					//renamed and all its uses should also be renamed.


int main(){
label:{
		int i;
		struct point{
			int distance;
			struct loc{														//this defintition may clash so rename.
				int x, y;
			};	
			struct loc l;													//if struct 'loc' is renamed this declaration is also to be updated.
		};

		struct point points[5];									//array of structs are also supported.

	setpoints:{
			for(i = 0; i < 5;i ++){		
				points[i].distance = 0;							//this loop sets the values in the array of structs points. 
				points[i].l.x = i; 
				points[i].l.y = i; 
			}
		}
	setpoints();
	
	print:{
			for(i = 0; i < 5;i ++){		
				printf("distance: %d ",points[i].distance);					//prints out the values which were set earlier.
				printf("x: %d ",points[i].l.x);
				printf("y: %d ",points[i].l.y);
				printf("\n");
			}	
		}	
	print();
	}

	label();

return 0;
}
