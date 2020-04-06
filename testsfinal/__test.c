struct s_label8 {
int * * p;
int * x;
};

struct s_foo11 {
struct s_label8 *__s;
};

void label8( struct s_label8*);
void foo11( struct s_foo11*);


#include<stdio.h>

int main(){
	int x = 2;
	int * p[10];
	p[0] = &x;
	
struct s_label8 slabel8;
slabel8.p = &p;
slabel8.x = &x;


	label8(&slabel8);	
	return 0;
}


void label8( struct s_label8* __s ){

		
struct s_foo11 sfoo11;
sfoo11.__s = __s;

		foo11(&sfoo11);
	}

void foo11( struct s_foo11* __s ){

			*(__s->__s->p)[0] = *(__s->__s->p)[0] + 1;
			printf("%d\n", *(__s->__s->p)[0]); 
		}

