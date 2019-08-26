#!/bin/bash

#include <string.h>


progDirectory=$(pwd)

outErr=" "
processCounter=0
declare -A pipes
declare -A procInfo

function createProcess()
{
	local rows=$1
	local cols=$2
	local threads=$3
	local pid=$4
	# attempt to compile the program in the current directory (either version 1 or version 2)
	# then execute the program if successful
	if gcc main.c gl_frontEnd.c -lGL -lglut -lpthread -o cell; then
		./cell $rows $cols $threads $pid &
		pipes[$pid]="/tmp/pipe${pid}"
		if [ $rows < 100 ] ; then
			$rows="${rows} "
		fi
		procInfo[$pid]="${rows}\t|\t${cols}\t|\t${threads}\t|"
		outErr="Successfully created Program"
	else
		outErr="Failed to create Program"
	fi
}

function sendCommand()
{
	local pipePath=$1
	local cmd=$2

	echo $cmd>$pipePath
}

function checkCommand()
{
	local pid=$1
	local pipePath=$2
	local cmd=$3
	printf "${cmd}"
	if [ "${cmd}" == " end" ] ; then
		sendCommand $pipePath "end"
		unset pipes[$pid]
		unset procInfo[$pid]
		processCounter=$[$processCounter -1]
		outErr="Process ${pid} ended"
	elif [ "${cmd}" == " rule 1" ] ; then
		sendCommand $pipePath "rule 1"
		outErr="Rule(${pid}): Game of Life"

	elif [ "${cmd}" == " rule 2" ] ; then
		sendCommand $pipePath "rule 2"
		outErr="Rule(${pid}): Coral Growth"
	
	elif [ "${cmd}" == " rule 3" ] ; then
		sendCommand $pipePath "rule 3"
		outErr="Rule(${pid}): Amoeba Growth"
	
	elif [ "${cmd}" == " rule 4" ] ; then
		sendCommand $pipePath "rule 4"
		outErr="Rule(${pid}): Maze Generation"
	
	elif [ "${cmd}" == " color on" ] ; then
		sendCommand $pipePath "color on"
		outErr="Color(${pid}): ON"
	
	elif [ "${cmd}" == " color off" ] ; then
		sendCommand $pipePath "color off"
		outErr="Color(${pid}): OFF"
	
	elif [ "${cmd}" == " speedup" ] ; then
		sendCommand $pipePath "speedup"
		outErr="Speed(${pid}): FASTER"
	
	elif [ "${cmd}" == " slowdown" ] ; then 
		sendCommand $pipePath "slowdown"
		outErr="Speed(${pid}): SLOWER"
	else
		outErr="Invalid Command"
	fi
}

# main while loop to take command input from user
while : ; do
	clear
	printf "\n\tProgram 04 Extra Credit Command Line Interpreter\n"
	printf "=================================================================================\n"
	printf "| Launch Program:\t 'cell HEIGHT WIDTH THREADS'\t\t\t\t|\n"
	printf "| End Program:\t\t 'pid: end'\t\t\t\t\t\t|\n"
	printf "| Change Rule:\t\t 'pid: rule RULE'\t\t\t\t\t|\n"
	printf "| \t\tGame of Life:\t RULE=1 \t\t\t\t\t|\n|\t\tCoral Growth:\t RULE=2\t\t\t\t\t\t|\n|\t\tAmoeba Growth:\t RULE=3\t\t\t\t\t\t|\n|\t\tMaze Growth:\t RULE=4\t\t\t\t\t\t|\n"
	printf "| Enable Color Mode:\t 'pid: color on'\t\t\t\t\t|\n"
	printf "| Disable Color Mode:\t 'pid: color off'\t\t\t\t\t|\n"
	printf "| Speed up:\t\t 'pid: speedup'\t\t\t\t\t\t|\n"
	printf "| Slow down:\t\t 'pid: slowdown'\t\t\t\t\t|\n|\t\t\t\t\t\t\t\t\t\t|\n"
	printf "|===============================Running Procceses: ${processCounter} ===========================|\n"
	printf "|\tPID\t|\tPipe\t|\tRows\t|\tCols\t|\tThreads\t|\n"
	printf "|===============================================================================|\n"
	for i in "${!pipes[@]}"
	do
  		echo -e "|\t$i\t| ${pipes[$i]}\t|\t${procInfo[$i]}"
	done
	printf "|===============================================================================|\n"
	printf "|return: ${outErr}\n"
	printf "|>>"
	outErr="Invalid Command"
	read varInput
	i=0
	for cmd in $varInput
	do
		inputArr[i]=$cmd
		i=$[$i +1]
	done

	# handle command input
	if [ "${inputArr[0]}" == "cell" ] ; 
		then
		processCounter=$[$processCounter +1]
		createProcess ${inputArr[1]} ${inputArr[2]} ${inputArr[3]} ${processCounter}
	fi

	# check first character for PID reference
	firstChar=${varInput:0:1}
	for i in "${!pipes[@]}"
	do
		if [ ${firstChar} == $i ] ;
			then
			checkCommand $i ${pipes[$i]} "${varInput##*:}"
		fi
	done

done

printf "Closed.\n"
