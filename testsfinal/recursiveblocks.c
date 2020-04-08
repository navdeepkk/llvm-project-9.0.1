//------------------------------------------//
// This test case demostrates the ability of//
// code to handel recursively nested blocks.//
// This test case shows a case of Indirect  //
// recursion between functions at arbitrary //
// depths. The innermost function can call  //
// any of the functions that are in its     //
// scope. It also shows that recursively nes//
// ted blocks are also supported.						//
// EXPECTED OUTPUT													//
// b:4 a:0                                  //
// b:3 a:0                                  //
// b:2 a:0                                  //
// b:1 a:0                                  //
// b:0 a:0                                  //
//------------------------------------------//

#include<stdio.h>

int main(){
    r:{
        int b = 5;
            p:{
								int a = 0;
                b--;
                printf("b:%d a:%d\n",b ,a);
                if(b==0) return;
                int c;
                q:{
                    int d;
                    s:{
                        p();								//calls p which is defined at a palce outside s, well within the scope of the call.
                    }
                    s();
                }
                q();
            }
        p();    
    }
    r();
return 0;
}

