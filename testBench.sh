#!/bin/bash
#
# This script executes the NBF code and generate statistics for gprof gcov and LibTM
#

#Profilling for the NBF code
DIR_BASE=~/RESEARCH_2013/Repos/LibTM
DIR_SOURCE=$DIR_BASE/apps/nbf
ANALYSE=$DIR_SOURCE/analyse
PROG=nbf
FILE=statistics.$PROG
CODE=nbf.c
TT=$(date +%Y%m%d%H%M)
CD="1 2 3 4"
CR="1 2"
exp_run_t="1 2 3"
_maxp=50
_timesteps=10

#Prepare Infra
Prep_infra()
{
    
    if [ ! -d "$DIR_SOURCE/analyse" ]; then
    mkdir $DIR_SOURCE/analyse
    fi
    
    if [ ! -d "$DIR_SOURCE/historic" ]; then
    mkdir $DIR_SOURCE/historic
    fi
    
}

#Aux Loop
AUX()
{

Func=6
Lines=12
    
cat ./analyse/nbf.gprof.CD_$1_CR_$2 | head -$Lines | tail -$(($Lines-1)) >> $3

}

#Executing gcov
Gcov_analyse()
{
    
#Compile with gcov flags, more info see MakeFile
    make -C $DIR_BASE nbf_gcov
    
#Execute the Program
    $DIR_BASE/$PROG $1 $2 $3 $4 $5
    
#Generate the Gcov Statistics
    gcov -b $DIR_SOURCE/$CODE
    
#Clean diretory
    make -C $DIR_BASE clean
    
#Analyze Statistics and save historic
    grep -v "-" $DIR_SOURCE/$CODE.gcov | grep -i ":" > $ANALYSE/$PROG.gcov.CD_$1_CR_$2
    mv $DIR_SOURCE/$PROG.o $DIR_SOURCE/$PROG.gcda $DIR_SOURCE/$CODE.gcov $DIR_SOURCE/tm.h.gcov $DIR_SOURCE/$PROG.gcno $DIR_SOURCE/historic/
    
}

#Executing gprof
Gprof_analyse()
{
    
#Compile with gprof flags, more info see MakeFile
    make -C $DIR_BASE nbf_gp
    
#Execute the Program
    $DIR_BASE/$PROG $1 $2 $3 $4 $5
    
#Generate Gprof Statistics and move it to Analize Folder
    gprof $DIR_BASE/$PROG > $DIR_SOURCE/$CODE.gprof
    
#Clean diretory
    make -C $DIR_BASE clean
    
#Analyse Statistics and save historic
    grep '[0-9]' $DIR_SOURCE/$CODE.gprof > $ANALYSE/$PROG.gprof.CD_$1_CR_$2
    mv $DIR_SOURCE/gmon.out $DIR_SOURCE/$PROG.o $DIR_SOURCE/$CODE.gprof $DIR_SOURCE/historic/
    
}

#*******  Main ****************

#Make Directories for analyse and historic
Prep_infra

#Compile the C code for analyse the statistics
#** As the code is simple and not LibTM standard, its compilation is not in the Makefile
gcc stat.c -o stat

# Generates Gcov and Gprof Statistics for all the CR and CD 3 times each
for _CR in $CR
do
     for _CD in $CD
     do
        #Clean up last execution
        rm -rf $ANALYSE/*
        for _exp_run_t in $exp_run_t
        do
           #Gcov_analyse $_CD $_CR $_maxp $_timesteps
           Gprof_analyse $_CD $_CR $_maxp $_timesteps $ANALYSE/$FILE
           AUX $_CD $_CR $ANALYSE/$_CD.$_CR.gp
        done
        
        # This code is used to alter the layout of the output file ....Can be split into function with more time....
        sort -t ';' $ANALYSE/$FILE >> $ANALYSE/$_CD.$_CR
        cat $ANALYSE/$_CD.$_CR $ANALYSE/$_CD.$_CR.gp > $_CD.$_CR.result
        grep -v "#" $_CD.$_CR.result |grep -v "Duration" |grep -v " "|grep -i ";" | grep -v "T" > $DIR_SOURCE/historic/c.file_$_CD.$_CR
        cp $DIR_SOURCE/historic/c.file_$_CD.$_CR $DIR_SOURCE/c.file
        echo "****CD:$_CD.CR$_CR****" >> excel
        ./stat >> excel
        echo "*********************" >> excel
     done
done
