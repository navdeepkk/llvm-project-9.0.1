#!/bin/bash

if [ $# -eq 1 ]; then
	if [ $1 -eq 0 ]; then
		rm -rd rewrittenfiles
		mkdir rewrittenfiles
		shopt -s nullglob
		#Loop through all .c files
		for f in *.c
			do
			cp $f ~/__temp.c

			../build/bin/prep ~/__temp.c -- -w -ferror-limit=1 > __y.c
			rm ~/__temp.c

			../build/bin/loop-convert __y.c -- -w -ferror-limit=1 > __x.c
			#rm __$f
			rm __y.c

			../build/bin/hoist __x.c -- -w -ferror-limit=1> ./rewrittenfiles/__$f
			rm __x.c
		done
	else
		#compile and run all the rewritten files.	
		OUTPATH=./rewrittenfiles/*
		for f in $OUTPATH
			do		 
			echo "OUTPUT-"$f
			echo "_______________________________________________"
			echo
			gcc -lm -w $f && ./a.out
			echo
		done	
	fi
	
else
	echo "------------------------------------------------------------------------------------Please provide exactly one argument."

fi

