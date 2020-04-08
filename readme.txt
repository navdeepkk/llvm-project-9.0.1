**********INSTRUCTIONS FOR COMPILING THE CODE**********
************clang/llvm version used - 9.0.1************

1.) Copy the three folders 'prep', loop-convert', 'hoist' into the clang-tools-extra folder present in /llvm-project-9.0.1
2.) In llvm-project-9.0.1/clang-tools-extra/CMakeLists.txt insert the following in seperate lines:-
			add_subdirectory(loop-convert)
			add_subdirectory(hoist) 
			add_subdirectory(prep)
3.) Run - 'mkdir build' and then Run - 'cd build' from /llvm-project-9.0.1
4.) Run - cmake -G "Unix Makefiles" -DLLVM_TARGETS_TO_BUILD=X86 -DBUILD_SHARED_LIBS=ON -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" ../llvm
5.) Run make
6.) After make is successfull copy the folder 'testsfinal' into /llvm-project-9.0.1



**********INSTRUCTIONS FOR RUNNING THE TEST CASES**********
1.) Copy 'testsfinal' into /llvm-project-9.0.1
2.) Run 'cd testsfinal'.
3.) There are 2 scripts in the folder 'run.sh', 'runsingle.sh'.
4.) 'run.sh' runs all the test cases. 'runsingle.sh' runs a single test case of choice. 
5.) 'run.sh' takes a single argument, either a '0' or '1'.
		::to run the tool on all the files and generate rewritten files, run 'bash run.sh 0'.
		::to compile the rewritten files and see their outputs run 'bash run.sh 1'
		::the output is written to stdout.
		::the rewritten files are present in /llvm-project-9.0.1/testsfinal/rewrittenfiles/
		::all rewritten files have "__" prefix with their original name as suffix.

6.)	'runsingle.sh' takes a single argument, the name of the input file. It rewrites and compiles the file in oneshot.
		::the output is written to stdout.
		::the rewritten file is present in /llvm-project-9.0.1/testsfinal/rewrittenfiles/
		::the rewritten file have "__" prefix with their original name as suffix.
		::for example 'bash runsingle.sh orig.c'



												**********TEST CASES Q/A**********
*****THE TEST CASE FILES CONTAIN MORE ELABORATE EXPLANATIONS ALONG WITH COMMENTS AT APPROPRIATE PLACES*****
*****TEST CASES ARE PRESENT IN llvm-project-9.0.1/testsfinal FOLDER*****

Q.1) Does your implementation handle recursively nested function blocks?
Ans.1) Yes it does. Test case 'recursiveblocks.c' demonstrates it. The test case has 4 recursively nested labels. It also demonstrates 
indirect recursion. The innermost label calls the label which is nested at depth 2. The expected output of the tese case is:- 
																EXPECTED OUTPUT                          
																b:4 a:0                                  
																b:3 a:0                                  
																b:2 a:0                                  
																b:1 a:0                                  
																b:0 a:0                                  

Q.2) Does your asst handle structs that are locally defined (i.e. defined inside the function)?
Ans.2) Yes it does. Test case 'localstructs.c' demonstrates local definition of structures. Structures may be locally defined within labels 
and they may behaving same names. All of them are renamed and hoisted to a global level. Both local and global structures are supported. 
The test demonstrates both the cases. Compiler error will be generated when compiling the rewritten code if resolution happens incorrectly, 
just becausethe variables inside the structs are of different names and if name resolution is incorrect then the varible being used 
will not be found. The expected output is:-
																EXPECTED OUTPUT													 
																1 1              												
																2 2              												
																2 2              												
																3 3  
                        
Q.3) If I have a local captured variable (i.e. the var is defined inside the function and used inside the closure) and a global var with same name, will your code still work?
Ans.3) Yes it will. Test case 'varresolution.c' shows the case of varible resolution when the variables are present at diffenret levels. The use of a varible 
resolves to the nearest available definition. In case no definitions is present it resolves to the global defintion. Incase a varible with same name is present 
in some enclosing scope and also in global scope, the use resolves to the definition in the nearest enclosing scope. The expected output is:-
                                EXPECTED OUTPUT													
                                local 'a' before call to 'r' is: 0		
                                global 'x' before call to 's' is: 2			
                                'a' inside 'r'is: 1										
                                'a' insieade 'r' after redefinition is: 0  
                                'x' inside 'r' is: 2											
                                'x' inside 's' is: 3										
                                local 'a' after call to 'r' is: 1			
                                global 'x' after call to 's' is: 2	                                        

Q.4) Do you pass all the variables to every closure or only the captured vars?
Ans.4) No, Only captured variables captured from the immediate parent are passed. Moreover a reference to the variables passed to the parent is 
also passed. The captured varibales are enclosed into a strucutre varible and the reference to this structure varible is passed to the closure.

Q.5) Can you handle multiple closures in same/different functions (not necessarily nested)?
Ans.5) Yes it can. Test case 'multipleclosures.c' demonstrates the  capibility of the tool to handle multiple labels even with same name. 
in different functions. The test case has multiple labels with the same name in different scopes, namely labels foo and foe. Moreover there is
a function name foo also. The calls are resolved appropriately to the closest label/function in scope. If no such label is present it resolves 
to a global functtion. The tool assumes that all calls are valid. The expected output is:-

                                EXPECTED OUTPUT									
                                in global foo!!!				   				
                                in foo inside main!!!						
                                'a' in foo inside main is: 0					
                                'a' in foo inside main is: 0						
                                'z' in foo inside main is: 3			
                                in foo inside foe!!!							
                                In foebar!!!											
                                'x' in foe inside bar is: 1			
                                'y' in foe inside bar is: 0		
Q.6) Do you explicitly pass global vars to the closure as well? Note that you don't need that.
Ans.6) No, Only the local vriables which are capturedd from the parent are passed. When a reference is not resolve to any of the variables
passed it resolves to a global varible. The testcase 'varresolution.c' demonstrates this.

*********ADDITIONAL FEATURES**********
Feature 1.) SUPPORT FOR QUAL TYPES - Qualifiers such as const and storage classes for varibles captured rom enclosing scopes are supported.
Test case 'qualtest.c' shows the capability. To observe correctness, The structure of scopes generated should be observed. The storage classes
such as 'static' should not be passed on to the scope structures. But qulaifiers such as const should be passed. The output from 
the test case is:-
                                struct s_foo18 {
                                    int* x;
                                };

                                struct s_bar36 {
                                    struct s_foo18 *__s;
                                    struct point22 * p1;
                                    const  struct point22 * p7;
                                    const struct point * p2;
                                };

Feature 2.) SUPPORT FOR POINTERS - The tool supports pointer data types for int, float and strucutures also. The test case 'nodetest.c' shows the 
capability of tool to rewrite a code for insertion and printing of a linked list. One more test case 'malloctest.c' test the capability of performing
malloc on a variable that has been captured form the enclosing scope and then printing it out.

Feature 3.) SUPPORT FOR NESTED STRUCTURE DEFINITIONS - Nested definitions of structures is also fully supported. The test case 'nestedstructs.c'
demonstrates this capability. The nested definitions are renamed and their corresponding uses are also renmaed. As all structures are hoisted to global
level the renaming is done to avoid conflicts with any variables or functions that are present at the global level. 

Feature 4.) SUPPORT FOR ARRAY OF STRUCTURES - Arrays of structures are also supported. The test case 'arrayofstructs.c' shows this. The test case shows
that arrays may be defined inside structures and also array of structures are compatible.

FEATURE 5.) SUPPORT FOR RECURSION - The tool fully supports recursion. The test cases 'factmix.c', 'quicksort.c' show this capability. Direct and
Indirect both type of recursion are supported.

FEATURE 6.) SUPPORT FOR MULTIDIMENSIONAL ARRAYS - Multidimensional array with subscripts as arrays adn so on is also supported. The arrays and their subscripts
can be captured from any enclosing scope. The test case 'arrtest.c' shows this. The 3-d array 'arr' presetn in the test case is indexed using another array
'b' which is 1-d. the index for array b is captured from enclosing scope.
