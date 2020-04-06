struct s_foo26 {
int * a;
};

struct s_fact37 {
struct s_foo26 *__s;
int * x;
int * b;
int * res;
};

void foo26( struct s_foo26*);
void fact37( struct s_fact37*);


//------------------------------------------//
// This test case is an extension of the    //
// fact test case and demonsrates that scop //
// are being captures correctly and also th //
// e values of variables is retained when   //
// a label is called again. fact() when call//
// the second time does nothing, as the val //
// ue of b and res is now one. the third ca //
// ll should work perfectly fine and print  //
// out 720.																	//
// EXPECTED OUTPUT													//
// x inside foo is: 1                       //
// x inside foo inside if is: 1             //
// factorial is: 120                        //
// factorial is: 120                        //
// factorial is: 720                        //
//------------------------------------------//

#include<stdio.h>

int x = 0;

int main() {
	int a;
	
struct s_foo26 sfoo26;
sfoo26.a = &a;


	a = 1;											//this varible should refer to the local a defined in main.
	if(a == 1){
		foo26(&sfoo26);
	}

	return 0;
}


void foo26( struct s_foo26* __s )  {

		x = x + 1;								//this x should resolve to the global var x.
		printf("x inside foo if is: %d\n", x);
		int b = 5;
		int res = 1;
		if (res == 1) {
			int x;
			x = 3;									//this x should resolve to the local var x.
			printf("x inside foo inside if is: %d\n", x);
		
struct s_fact37 sfact37;
sfact37.__s = __s;
sfact37.x = &x;
sfact37.b = &b;
sfact37.res = &res;

			fact37(&sfact37);									//this call will calculate the factorial.
			printf("factorial is :%d \n", res);
			fact37(&sfact37);									//this call should return without doing anyhting as b has already been reduced to 1 and should print 120.
			printf("factorial is :%d \n", res);
			b = 6;
			res = 1;  
			fact37(&sfact37);									//this call should return the factorial of 6 as b and res have been restored once again.
			printf("factorial is :%d \n", res);
		}
	}

void fact37( struct s_fact37* __s )  {

			if ((*(__s->b)) == 1) {
				return;
			} else {
				(*(__s->res)) *= (*(__s->b));
				(*(__s->b))--;
				fact37(__s);
			}
		}

