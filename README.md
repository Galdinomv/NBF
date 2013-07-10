**************************************************************************
* Profiling for  NBF LibTM  - UofT - July 2013 ***************************
**************************************************************************

1 - Summary about the codes  (**Some can be re-used to generate statistics for each transaction)  
2 - Instructions and how to execute.
3 - Results for NBF.


**********************************************************************
1 - Summary about the Codes 

NBF.c  --> This is the main code, since it executes the NBF calculations that are being measured and profiled. The original code(*original.c) was divided into 4 transactions in the LibTM version, and functions to generate statistics has been added.

Obs: The functions STATISTICS() and MATH(), probably can be re-used in other LibTM codes. This Function just use the boundaries values provided by LibTM and subtract the previous one to generate the current values for each transaction. 


TESTBENCH.sh  --> This script executes the nbf code for ALL the CD and CR possibilities, generate gprof and gcov statistics and execute the stat.c for analyses

Obs: this code requires some updates the Makefile due the gprof and gcov compiling flags.


STAT.c --> This code  receives the statistics generated as input, loads them into vectors and calculates the average for ALL the CD/CR for all the threads. The Output is a file in a simple format for excel.

Excel --> This spreadsheet has one tab where the output from stat.c should be pasted. Most of the other tabs will be generated automatically.


**********************************************************************
2 - Instructions 

2.1 - Add the Instructions below in the Makefile

nbf: lib_tm.a $(MAKE) -C apps/nbf nbf CFLAGS="$(CFLAGS)"
nbf_gcov: lib_tm.a $(MAKE) -C apps/nbf nbf CFLAGS="$(CFLAGS) -g -fprofile-arcs -ftest-coverage "
nbf_gp: lib_tm.a $(MAKE) -C apps/nbf nbf CFLAGS="$(CFLAGS) -pg"

2.2 - Copy/Download the files nbf.c, testbench.sh and stat.c inside the LibTM/apps/<Folder>

2.3 - Execute the test bench.sh code. 

2.4 - Past the excel file generated onto the spreadsheet Analyses.xlsx

**********************************************************************
3 - Result for the NBF  - Updated July, 10th, 2013 *Marcus Galdino

As a conclusion, the parallelized version has shown speedup for the transactions, however, this speedup was overlapped by the overhead (Slow down) of the Functions Read and Write Management(LibTM).


The following is the Summary of the Results.

** The Best Policy was: Abort Readers + Read Optimistic, having best results for all the transactions.

** The worst Policies were: Wait for Readers and Pessimistic combinations.

* However the Pessimistics Policies had low LibTM overhead(*see slide 14) wich might generate a better execution Time (*see slide 15)

###### Experiments:

1 - Considering:
Number of Atoms=16384
Timesteps=10

Speedup for the functions: Insert_Random_values; Map_Neighbors ; Push_Atoms.
Slight Slow Down: Calculates_Forces

* Function Write Management has around 80% of Total Execution. 

2 - Considering:
Number of Atoms=316384   (20X Bigger)
Timesteps=100            (10X Bigger)

Speedup for ALL the functions, however slow down in the Read Write Functions.

* DIstribution: 30% Write, 43% Read,  11% Transactions(10% Calculates_Forces).  (See slide 16)

For analyse purposes, The Number of Atoms was increase to a (2416384)140X Bigger, but the percentage of time inside Transactions decreased to 6.5%, showing that the increase of Atoms not necessary reflects a increase of % inside Transactions. 


