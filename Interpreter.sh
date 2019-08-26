#!/bin/bash

#include <string.h>

clear
printf "\n\tProgram 04 Command Line Interpreter\n\n"

printf "Which version would you like to run? (input 1 or 2): "
read version

printf "\nEnter desired number of Rows: "
read rows

printf "\nEnter desired number of Columns: "
read columns

printf "\nEnter desired number of Threads: "
read threads

echo -e "\nRows: ${rows}  Columns: ${columns}  Threads: ${threads} \n"


# check user input for version number selection and change directory accordingly
if [ "$version" == "1" ] ;
	then
	cd "Version 1"
elif [ "$version" == "2" ] ;
	then
	cd "Version 2"

fi
progDirectory=$(pwd)


# attempt to compile the program in the current directory (either version 1 or version 2)
# then execute the program if successful
if gcc main.c gl_frontEnd.c -lGL -lglut -lpthread -o cell; then
	echo "Successfully compiled program."
	./cell $rows $columns $threads &
else
	echo "Failed to compile program."
fi


cd /tmp/

# main while loop to take command input from user
while : ; do
	printf ">> "
	read varInput

	if [ "$varInput" == "end" ] ; 
	then
		printf "\nClosing Cellular Automation..."
		echo "end">prog04pipe
		printf "Done.\n"
		printf "\nClosing Interpreter..."
		break

	elif [ "$varInput" == "rule 1" ] ;
	then
		printf "rule is now 1: Game of Life\n"
		echo "rule 1">prog04pipe

	elif [ "$varInput" == "rule 2" ] ;
	then
		printf "rule is now 2: Coral Growth\n"
		echo "rule 2">prog04pipe

	elif [ "$varInput" == "rule 3" ] ;
	then
		printf "rule is now 3: Amoeba Growth\n"
		echo "rule 3">prog04pipe

	elif [ "$varInput" == "rule 4" ] ;
	then
		printf "rule is now 4: Maze Generation\n"
		echo "rule 4">prog04pipe

	elif [ "$varInput" == "color on" ] ;
	then
		printf "Color: ON\n"
		echo "color on">prog04pipe

	elif [ "$varInput" == "color off" ] ;
	then
		printf "Color: OFF\n"
		echo "color off">prog04pipe

	elif [ "$varInput" == "speedup" ] ;
	then
		printf "Speeding up generation by 5000 mu\n"
		echo "speedup">prog04pipe

	elif [ "$varInput" == "slowdown" ] ;
	then
		printf "Slowing down generation by 5000 mu\n"
		echo "slowdown">prog04pipe

	else
		printf "Invalid Command.\n"
	fi

done

printf "Closed.\n"
