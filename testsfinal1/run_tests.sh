# Run tool on all test cases.
rm test1_op.c
bin/loop-convert test1.c>>test1_op.c

rm test2_op.c
bin/loop-convert test2.c>>test2_op.c

rm test3_op.c
bin/loop-convert test3.c>>test3_op.c

rm test4_op.c
bin/loop-convert test4.c>>test4_op.c

rm test5_op.c
bin/loop-convert test5.c>>test5_op.c

rm test6_op.c
bin/loop-convert test6.c>>test6_op.c

rm test7_op.c
bin/loop-convert test7.c>>test7_op.c

rm test8_op.c
bin/loop-convert test8.c>>test8_op.c

rm test9_op.c
bin/loop-convert test9.c>>test9_op.c

rm test10_op.c
bin/loop-convert test10.c>>test10_op.c

rm test11_op.c
bin/loop-convert test11.c>>test11_op.c

rm test12_op.c
bin/loop-convert test12.c>>test12_op.c

rm test13_op.c
bin/loop-convert test13.c>>test13_op.c

rm test14_op.c
bin/loop-convert test14.c>>test14_op.c

#compile output of all test cases
echo "_________________________________________________________________________"
echo "Compiling Test1"
gcc test1_op.c -o test1
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test2"
gcc test2_op.c -o test2
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test3"
gcc test3_op.c -o test3
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test4"
gcc test4_op.c -o test4
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test5"
gcc test5_op.c -o test5
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test6"
gcc test6_op.c -o test6
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test7"
gcc test7_op.c -o test7
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test8"
gcc test8_op.c -o test8
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test9"
gcc test9_op.c -o test9
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test10"
gcc test10_op.c -o test10
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test11"
gcc test11_op.c -o test11
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test12"
gcc test12_op.c -o test12
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test13"
gcc test13_op.c -o test13
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Compiling Test14"
gcc test14_op.c -o test14
echo "_________________________________________________________________________"

#Run all test cases
echo "_________________________________________________________________________"
echo "Running  Test1: Test sent with code assignment."
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "Value of x before calling nested function foo = 3
Value of x inside nested function foo = 3
Value of x after calling nested function foo = 4"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test1
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test2:"
echo "In addition, does your implementation handle recursively nested function blocks?

This test case checks if variable is resolved to it's corresponding declaration as per static scoping rules
for arbitrary depth.

More specifically it tests following cases:
Case 1: Variable used at depth 2 from variable declared in label at depth 2.(From any depth to any depth)
Case 2: Variable used at depth 7(randomly chosen to test if large depth is possible) from variable declared in function."
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "Case 1 output: 
value of a before call to f4: 6
value of a after call to f4: 14
Case 2 output: 
value of a before call to f1 : 5
value of a after call to f1 : 50
"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test2
echo
echo "_________________________________________________________________________"


echo "_________________________________________________________________________"
echo "Running  Test3:"
echo "Does your asst handle structs that are locally defined (i.e. defined inside the function)? 

This test case checks when structures are declared inside function/labels.

Case 1: Structure declared in function and it's structure variable used in a nested function.
Case 2: Structure declared in function has same name as structure in other function/ global structure. (Structure Renaming) 

Expected output:
1. Structure declarations are hoisted outside so as to enable access to nested functions.
2. Structures are renamed to avoid compilation error when same name structures are declared in other nested functions.
3. Structure variable declarations are resolved and rewritten to match corresponding structure.
4. Structure variable declarations inside the structures are renamed."

echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "Output can seen in test3_op.c and labels_test3.c.h.
Basically, if output is correct then compiling test3_op.c should not generate any compiler errors."
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
echo "Check if Compiling Test3 (above) had any compiler errors"
./test3
echo
echo "_________________________________________________________________________"


echo "_________________________________________________________________________"
echo "Running  Test4:"
echo "If I have a local captured variable (i.e. the var is defined inside the function and used inside the closure) and a global var with same name, will your code still work?"
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "value of global x = 1
value of local x before call: 7
value of local x after call: 9"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test4
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test5:"
echo "Can you handle multiple closures in same/different functions (not necessarily nested)?

Case 1: Nested labels : Already tested in Test2.
Case 2: Two labels at same depth (f1 and f2) i.e not nested.
        Second label(f2) redeclares variable with same name as parent.
        variable in first label(f1) gets resolved to parents declaration.
	variable in second label gets(f2) resolved to its own declaration.
"
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "Case 2 output: 
a at f1 = 5
a at f2 = 7"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test5
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test6:"
echo "This test case checks if rewriting for array variable for following cases 
work correctly inside nested function.
a. Handling of multidimensional arrays. 
b. array as index of array  e.g. arr[b[i]][b[j]] = 8
c. expressions as index of array e.g a[i + 2j][i*j] = 4"
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "0 0 0 0 0 0 
0 1 0 0 0 0 
0 0 2 0 0 0 
0 0 0 0 0 0 
0 0 0 0 4 0 
0 0 0 0 0 0 

arr[j-i][j*i] = 7"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test6
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test7:"
echo "This test case checks if different combination of struct and array can be resolved by nested functions.

Case 1: variable as 3D-array of structure 
Case 2: variable of structure with 3D-array as a member
Case 3: array of structure having 2D-array as member
"
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "case1 output:
value of c[1][0][1].a before call to f2: 7
value of c[1][0][1].a after call to f2: 8

case2 output:
t.a[3][2][1] = 80

case3 output:
s[2][1].a[5][1] = 4"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test7
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test8:"
echo "This test case checks implementation of linked list.
It implements insert() and traverse() as nested functions.
Head of linked list i.e. struct node* head is declared in main function and used in insert and traverse.

Basically, this test case verifies one of the case of support of tool for pointers.
"
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "Linked List: 0->10->20->30->40->50->60->70->80->90->NULL"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test8
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test9:"
echo "This test case checks implementation of 2D array with the help of double pointer and malloc.
int **arr is declared in main function. 
While memory allocation and initialization of arr is done in nested function init().
And it is printed using nested function print.

This is other test case to check support of tool for pointer."
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "0 0 0 0 0 
0 1 2 3 4 
0 2 4 6 8
"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test9
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test10:"
echo "This test case checks if captured variable can passed as arguments to other functions.

More specifically it tests following cases:
Case 1: Passing value of variable to a function call at depth 2 from variable declaration. => _square(x)
Case 2: Passing address of variable to a function call at depth 2 from variable declaration.  => square(&x)"
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "Case 1 output: 
value of a before call to f1: 5
value of a after call to f1: 25
Case 2 output: 
value of a before call to f1: 5
value of a after call to f1: 25
"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test10
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test11:"
echo "Function Renaming
   a. Call expression resolution to appropriate function definition, when nested function has same name as 
   global function or other nested function.
   b. Appropriate call resolution based on static scope."
echo "-------------------------------------------------------------------------"
echo "Expected Output:"
echo "Line no 34 Called global foo()
Line no 27 called foo/f1 
Line no 47 called main/foo .
Line no 43 called main/foo/f1 
Line no 52Called global foo()
Line no 27 called foo/f1
"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test11
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test12:"
echo "This test case check recursion for nested functions.
fact is nested function which calls itself.
It calculates factorial of 6."
echo "Expected Output:"
echo "Factorial is 720."
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test12
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test13:"
echo "This test case checks indirect recursion.
fun1 and fun2 are two nested functions.
fun2() is nested inside fun1().

fun1() calls fun2()
fun2() calls fun1()

Both increment and print variable n."
echo "Expected Output:"
echo "1 2 3 4 5 6 7 8 9 10"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test13
echo
echo "_________________________________________________________________________"

echo "_________________________________________________________________________"
echo "Running  Test14:"
echo "This test case implements recursive insertion sort.
i_sort() is a nested function which sorts array."
echo "Expected Output:"
echo "5 11 12 13 327"
echo "-------------------------------------------------------------------------"
echo "Actual Output:"
./test14
echo
echo "_________________________________________________________________________"

