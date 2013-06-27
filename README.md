**************************************************************************
* Profiling for  NBF LibTM  - UofT - July 2013
**************************************************************************

1 - Summary about the codes  (**Some can be re-used to generate statistics for each transaction)  
2 - Instructions and how to execute
3 - Results for NBF
4 - Results for LibTM
5 - Suggestions for the future


1 - Summary about the Codes 

NBF.c  --> This is the main code, since it executes the NBF calculations that are being measured and profiled. The original code(*original.c) was divided into 4 transactions in the LibTM version, and functions to generate statistics has been added.

Obs: The functions STATISTICS() and MATH(), probably can be re-used in other LibTM codes. This Function just use the boundaries values provided by LibTM and subtract the previous one to generate the current values for each transaction. 


TESTBENCH.sh  --> This script executes the nbf code for ALL the CD and CR possibilities, generate gprof and gcov statistics and execute the stat.c for analyses

Obs: this code requires some updates the Makefile due the gprof and gcov compiling flags.


STAT.c --> This code  receives the statistics generated as input, loads them into vectors and calculates the average for ALL the CD/CR for all the threads. The Output is a file in a simple format for excel.

Excel --> This spreadsheet has one tab where the output from stat.c should be pasted. All the other tabs will be generated automatically.



2 - Instructions 

2.1 - Add the Instructions below in the Makefile

nbf: lib_tm.a $(MAKE) -C apps/nbf nbf CFLAGS="$(CFLAGS)"
nbf_gcov: lib_tm.a $(MAKE) -C apps/nbf nbf CFLAGS="$(CFLAGS) -g -fprofile-arcs -ftest-coverage "
nbf_gp: lib_tm.a $(MAKE) -C apps/nbf nbf CFLAGS="$(CFLAGS) -pg"

2.2 - Copy/Download the files nbf.c, testbench.sh and stat.c inside the LibTM/apps/<Folder>

2.3 - Execute the test bench.sh code. 

2.4 - Past the excel file generated onto the spreadsheet Analyses.xlsx

3 - In construction

4 - In construction

5 - In construction 

