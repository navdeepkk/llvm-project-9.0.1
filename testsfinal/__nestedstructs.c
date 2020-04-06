struct s_label29 {
};

struct s_setpoints47 {
struct s_label29 *__s;
struct point34 * points;
int * i;
};

struct s_print58 {
struct s_label29 *__s;
struct point34 * points;
int * i;
};

struct loc{
																	//this global struct 'loc' will clash with 'loc' defined inside struct 'point' 
	int a, b;																	//when it is hoisted to a global level. so the struct 'loc' in points is to be
};

struct point34{

			int distance;
			
struct loc38{
														//this defintition may clash so rename.
				int x, y;
			};	
			struct loc38 l;													//if struct 'loc' is renamed this declaration is also to be updated.
		};

void label29( struct s_label29*);
void setpoints47( struct s_setpoints47*);
void print58( struct s_print58*);


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


																					//renamed and all its uses should also be renamed.


int main(){

struct s_label29 slabel29;


	label29(&slabel29);

return 0;
}


void label29( struct s_label29* __s ){

		int i;
		


		struct point34 points[5];									//array of structs are also supported.

	
struct s_setpoints47 ssetpoints47;
ssetpoints47.__s = __s;
ssetpoints47.points = &points;
ssetpoints47.i = &i;

	setpoints47(&ssetpoints47);
	
	
struct s_print58 sprint58;
sprint58.__s = __s;
sprint58.points = &points;
sprint58.i = &i;
	
	print58(&sprint58);
	}

void setpoints47( struct s_setpoints47* __s ){

			for((*(__s->i)) = 0; (*(__s->i)) < 5;(*(__s->i)) ++){		
				(__s->points)[(*(__s->i))].distance = 0;							//this loop sets the values in the array of structs points. 
				(__s->points)[(*(__s->i))].l.x = (*(__s->i)); 
				(__s->points)[(*(__s->i))].l.y = (*(__s->i)); 
			}
		}

void print58( struct s_print58* __s ){

			for((*(__s->i)) = 0; (*(__s->i)) < 5;(*(__s->i)) ++){		
				printf("distance: %d ",(__s->points)[(*(__s->i))].distance);					//prints out the values which were set earlier.
				printf("x: %d ",(__s->points)[(*(__s->i))].l.x);
				printf("y: %d ",(__s->points)[(*(__s->i))].l.y);
				printf("\n");
			}	
		}

