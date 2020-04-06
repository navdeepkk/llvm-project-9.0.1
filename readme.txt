**********INSTRUCTIONS FOR COMPILING THE CODE**********
************clang/llvm version used - 9.0.1************

1.) Copy the three folders into the clang-tools-extra folder present in /llvm-project-9.0.1
2.) In llvm-project-9.0.1/clang-tools-extra/CMakeLists.txt insert the following in seperate lines:-
			add_subdirectory(loop-convert)
			add_subdirectory(hoist) 
			add_subdirectory(prep)
3.) Run - 'mkdir build' and then Run - 'cd build' from /llvm-project-9.0.1
4.) Run - cmake -G Unix Makefiles -DLLVM_TARGETS_TO_BUILD=X86 -DBUILD_SHARED_LIBS=ON -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra"
5.) Run make


**********TEST CASES Q/A**********

