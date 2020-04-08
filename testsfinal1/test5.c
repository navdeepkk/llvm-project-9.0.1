/* 
Can you handle multiple closures in same/different functions (not necessarily nested)?

Case 1: Nested labels : Already tested in Test2.
Case 2: Two labels at same depth (f1 and f2)
        Second label(f2) redeclares variable with same name as parent.
        variable in first label(f1) gets resolved to parents declaration.
	variable in second label gets(f2) resolved to its own declaration.

Case 2 output: 
a at f1 = 5
a at f2 = 7

*/

void case2()
{
	int a = 5;
	printf("\nCase 2 output: ");

	f1:{
		printf("\na at f1 = %d",a); 	// Should get resolved to a in case2()
	   }

	f2:{
		   int a = 7;
		   printf("\na at f2 = %d", a); // Should get resolved to a in f2
	   }

	   f1();
	   f2();
}

int main()
{
	case2();
}

