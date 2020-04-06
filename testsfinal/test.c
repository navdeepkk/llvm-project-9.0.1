#include<stdio.h>

int main(){
	int x = 2;
	int * p[10];
	p[0] = &x;
	label:{
		foo:{
			*p[0] = *p[0] + 1;
			printf("%d\n", *p[0]); 
		}
		foo();
	}

	label();	
	return 0;
}
