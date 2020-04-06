//------------------------------------------//
// THis exapmle demonstrates local definition/
// fo structures. structures may be locally //
// defined within labels and they may be		//
// having same names. all of them are renamed/
// and hoisted to a global level. both local//
// and global structures are supported. the //
// test demonstrates both the cases.				//
// compiler error will be generated in the  //
// rewritten code if resolution happens     //
//     incorrectly just because the variable//
// s inside the structs are of different nam//
// es and if name resolution is incorrect   //
// then the varible being used will not be  //
// found.																		//
// EXPECTED OUTPUT													// 
// 1 1              												//
// 2 2              												//
// 2 2              												//
// 3 3              												//
//------------------------------------------//

#include<stdio.h>

struct point{
		int e, f;
	};

void hey(){
	label:{
	struct point p;														//this should resolve to point defined globally as no definitions is presetn in scope.
		p.e = 3;                                //this line will give compiler error if resolved incorrectly.
		p.f = 3;
		printf("%d %d\n", p.e, p.f);	
}
label();
}


int main(){
	struct point{
			int x, y;
			struct point * p;
		};
	foo:{		
	struct point{
			int a, b;
		};
		bar:{
			struct point p1;													//this should resolve to point defined in foo, and should be renamed.
			p1.a = 1;																	//this line will give compiler error if resolved incorrectly.
			p1.b = 1;
			printf("%d %d\n", p1.a, p1.b);
		}
		foe:{
			struct point{
				int c, d;
			};
			struct point p2;													//this should resolve to point defined locally, and should be renmaed.
			p2.c = 2;																	//this line will give compiler error if resolved incorrectly. 		
			p2.d = 2;																	
			printf("%d %d\n", p2.c, p2.d);
		}
	bar();
	foe();
}
	foo();
			struct point p2;													//this should resolve to point defined in main, and should be renamed.
			p2.x = 2;																	//this line will give compiler error if resolved incorrectly.
			p2.y = 2;
			printf("%d %d\n", p2.x, p2.y);
	hey();
	return 0;
}

