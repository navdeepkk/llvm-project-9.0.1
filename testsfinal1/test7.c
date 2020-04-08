/*
This test case checks if different combination of struct and array can be resolved by nested functions.

Case 1: variable as 3D-array of structure 
Case 2: variable of structure with 3D-array as a member
Case 3: array of structure having 2D-array as member

case1 output:
value of s[1][0][1].a before call to f2: 7
value of s[1][0][1].a after call to f2: 8

case2 output:
t.a[3][2][1] = 80

case3 output:
s[2][1].a[5][1] = 4

*/

/*commenting following define won't run that test case and also won't generate code for that test case*/
#include<stdio.h>

#define C1
#define C2
#define C3

#ifdef C1
struct case1_s{
	int a;
	int b;
};

//Case 1: variable as 3D-array of structure 
void case1()
{
	f1:{
		struct case1_s c[4][3][2];
		int i = 1;
		int j = 0;
		c[1][0][1].a = 7;
		f2:{
			c[i][j][i].a++;
		}
		printf("\ncase1 output:");
		printf("\nvalue of c[1][0][1].a before call to f2: %d", c[1][0][1].a);
		f2();
		printf("\nvalue of c[1][0][1].a after call to f2: %d", c[1][0][1].a);
	}
	f1();
}
#endif	

#ifdef C2
struct case2_s{
	int a[9][8][7];
};

//Case 2: variable of structure with 3D-array as a member
void case2()
{
	struct case2_s s;
	s.a[4][2][1] = 8;

	f1:{
		struct case2_s t;

		t.a[3][2][1] = s.a[4][2][1] * 10;

		printf("\ncase2 output:\n");
		printf("t.a[3][2][1] = %d\n",t.a[3][2][1]);
	}
	f1();
}
#endif

#ifdef C3
struct case3_s{
	int a[20][3];
};

//Case 3: array of structure having 2D-array as member
void case3()
{
	struct case3_s s[4][3];

	s[2][1].a[5][1] = 4;

	f1:{

		printf("\n case3 output:");
		printf("\n s[2][1].a[5][1] = %d\n",s[2][1].a[5][1]);
	}
	f1();
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

	#ifdef C3
	case3();
	#endif
}
