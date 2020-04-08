#!/bin/bash

if [ $# -eq 1 ]; then
	rm -rd rewrittenfiles
  mkdir rewrittenfiles	
	cp $1 ~/__temp.c

	#../build/bin/preprep ~/temp.c > __z.c
	#rm ~/temp.c

	../build/bin/prep ~/__temp.c -- -w -ferror-limit=1 > __y.c
	rm ~/__temp.c

	../build/bin/loop-convert __y.c -- -w -ferror-limit=1 > __x.c
	rm __$1
	rm __y.c

	../build/bin/hoist __x.c -- -w -ferror-limit=1> ./rewrittenfiles/__$1
	rm __x.c

	echo "---------------------------------------OUTPUT----------------------------------------OUTPUT--------------------------------------OUTPUT-----------------------------------------"

	gcc -lm  ./rewrittenfiles/__$1 && ./a.out

else
	echo "------------------------------------------------------------------------------------Please provide exactly one input program."

fi

