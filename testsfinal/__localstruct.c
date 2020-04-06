struct s_foo52 {
};

struct s_label33 {
};

struct s_bar60 {
struct s_foo52 *__s;
};

struct s_foe68 {
struct s_foo52 *__s;
};

struct point{

		int e, f;
	};

struct point47{

			int x, y;
			struct point47 * p;
		};

struct point56{

			int a, b;
		};

struct point72{

				int c, d;
			};

void label33( struct s_label33*);
void foo52( struct s_foo52*);
void bar60( struct s_bar60*);
void foe68( struct s_foe68*);


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




void hey(){
	
struct s_label33 slabel33;

label33(&slabel33);
}


int main(){
	

	
struct s_foo52 sfoo52;

	foo52(&sfoo52);
			struct point47 p2;													//this should resolve to point defined in main, and should be renamed.
			p2.x = 2;																	//this line will give compiler error if resolved incorrectly.
			p2.y = 2;
			printf("%d %d\n", p2.x, p2.y);
	hey();
	return 0;
}



void label33( struct s_label33* __s ){

	struct point p;														//this should resolve to point defined globally as no definitions is presetn in scope.
		p.e = 3;                                //this line will give compiler error if resolved incorrectly.
		p.f = 3;
		printf("%d %d\n", p.e, p.f);	
}

void foo52( struct s_foo52* __s ){
		
	

		
struct s_bar60 sbar60;
sbar60.__s = __s;

		
struct s_foe68 sfoe68;
sfoe68.__s = __s;

	bar60(&sbar60);
	foe68(&sfoe68);
}

void bar60( struct s_bar60* __s ){

			struct point56 p1;													//this should resolve to point defined in foo, and should be renamed.
			p1.a = 1;																	//this line will give compiler error if resolved incorrectly.
			p1.b = 1;
			printf("%d %d\n", p1.a, p1.b);
		}

void foe68( struct s_foe68* __s ){

			

			struct point72 p2;													//this should resolve to point defined locally, and should be renmaed.
			p2.c = 2;																	//this line will give compiler error if resolved incorrectly. 		
			p2.d = 2;																	
			printf("%d %d\n", p2.c, p2.d);
		}

