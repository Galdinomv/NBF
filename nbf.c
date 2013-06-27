/*
 
 Molecular Dynamics Non Bonded Force Kernel
 
 Structure:
 Function 1 - Insert_Random_values
 Function 2 - Map_Neighbors
 Function 3 - Calculates_Forces
 Function 4 - Push_Atoms
 Function 5 - Calculates_xsum
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../../src/tm.h"
#include "../../src/utils/hrtime.h"

#define N_TH  4                //Number of Threads
#define N_TXN 4                //Number of Transactions in the code
#define N_ELEM 16384

/* TM Types */
typedef  tm_type<int>  tm_int;
typedef	tm_type<double>  tm_double;

/* NBF Variables */
int natoms = 8192 * 2;             //Number of Atoms
int timesteps;                     //Number of Iterations       **ORIGINAL VALUE = 10
double xsum;                   //checksum
double c = 0.1;                    //Force Constant

/* TM Variables */
tm_int inb[N_ELEM];                //Bonded atoms (will NOT be calculated)
tm_double x[N_ELEM],f[N_ELEM];     //x=coordinate ; f=force
tm_int **partners;                 //Non-bonded atoms with a cutoff distance from others (will be calculated)
tm_int j;                          //Used in Calculates_Forces to store the partners
tm_double force;                   //Auxiliar for f while calculating force
tm_int maxp;                      //Maximum number of Partners **ORIGINAL VALUE = 50
tm_double temp;
tm_int auxm=0;
//tm_int min_value;

/* Performance Variables */
unsigned int t_tx[N_TH];                     // Total # of transactions started by each thread
unsigned int tx[N_TH];                       // Total # of transactions commited by each thread
volatile int _tx = 0;                        // # of transactions successfully committed by current thread
volatile int _t_tx = 0;                      // # of transactions started by current thread
double durations[N_TH][N_TXN];               // Durations for each transaction ** Matrix is used to split it by thread
volatile unsigned long long start[N_TXN];    // Aux variable for Durations
unsigned int tx_cnt[N_TXN], t_tx_cnt[N_TXN]; // Aux Variables for Statistics
tstats_t thread_stats[N_TH][N_TXN];          //Save the statistics by Thread
tstats_t tot_stats[N_TXN];                   //Save the statistics by Function
tstats_t t_aux[N_TH][N_TXN],t_aux2[N_TXN];   //Aux Variables for statistics


// Writing in X Insert_Random_values
int Insert_Random_values(void *arg,int id)
{
    int i;
    
    start[0] = get_c();                                // Gets time before enter the TXN
    
    /* Some bogus data for testing, to get irregularity and off-processor accesses */
    for (i = id; i < natoms; i+=N_TH)
    {
        BEGIN_TRANSACTION();
        _t_tx++;
        
        x[i] = ((i * 17) % natoms) * 1.0;
        
        COMMIT_TRANSACTION();
        _tx++;
    }
    
    durations[id][0] = c_to_t( get_c() - start[0] );    // Calculates the Time spent by the TXN above
    
    t_tx[id] = _t_tx;
    tx[id] = _tx;
    
    return 0;
}

//write in inb, partners.... reads X
int Map_Neighbors(void *arg,int id)
{
    int i,p;
    tm_int min_value;
    
    start[1] = get_c();                                 // Gets time before enter the TXN
    
    /* Calculates the number partners */
    for (i = id; i < natoms; i+=N_TH)
    {
        BEGIN_TRANSACTION();
        _t_tx++;
        
        min_value = (i/200 + 1) - maxp;
        
        inb[i] = maxp + ((min_value > 0)? auxm: min_value);
        
        for (p = 0; p < inb[i]; p++)
        {
            partners[i][p] =  (i + p*natoms/11 + 1) % natoms;
            if (partners[i][p] == i) printf("%d %d\n", i, p);
            if (x[i] == x[partners[i][p]]) printf("x %f - %d %d %d\n", x[i], i, p, partners[i][p]);
        }
        
        COMMIT_TRANSACTION();
        _tx++;
        
    }
    
    durations[id][1] = c_to_t( get_c() - start[1] );    // Calculates the Time spent by the TXN above
    
    t_tx[id] = _t_tx;
    tx[id] = _tx;
    
    return 0;
}


int Calculates_Forces(void *arg,int id)
{
    int t,i,p;
    
    start[2] = get_c();
    
    /* Calculates Force */
    for (i = id; i < natoms; i+=N_TH)
    {
        BEGIN_TRANSACTION();
        _t_tx++;
        
        for (p = 0; p < inb[i]; p++)
        { //double temp;
            j = partners[i][p];
            temp = x[i] - x[j];
            force = pow(temp , (double)(-4)) * 1000000000.0 ;
            f[i] = f[i] + force;
            f[j] = f[j] - force;
        }
        
        COMMIT_TRANSACTION();
        _tx++;
    }
    
    durations[id][2] = c_to_t( get_c() - start[2] );      // Calculates the Time spent by the TXN above
    
    
    
    //    printf("Force done.\n");
    
    t_tx[id] = _t_tx;
    tx[id] = _tx;
    
    return 0;
}


int Push_Atoms(void *arg,int id)
{
    int i;
    
    start[3] = get_c();
    
    /* Push atoms */
    for (i = id; i < natoms; i+=N_TH)
    {
        BEGIN_TRANSACTION();
        _t_tx++;
        
#if 0
        printf("force[%d] = %e\n", i, f[i]);
#endif
        
        x[i] = x[i] + c*f[i];
        f[i] = 0.0;
        
        COMMIT_TRANSACTION();
        _tx++;
    }
    
    durations[id][3] = c_to_t( get_c() - start[3] );     // Calculates the Time spent by the TXN above
    
    //printf("X done\n");
    
    t_tx[id] = _t_tx;
    tx[id] = _tx;
    
    return 0;
    
}


int Calculates_xsum(void *arg,int id)
{
    int i;
    
    xsum = 0.0;
    //start[4] = get_c();
    
    /* Compute checksum */
    for (i = 0; i < natoms; i++) {
        xsum = xsum + x[i]*0.1;
    }
    
    //durations[id][4] = c_to_t( get_c() - start[4] );     // Calculates the Time spent by the TXN above
    
    return 0;
}


int Math(int txn,int i,int t,tstats_t Source,tstats_t Result,FILE *out_f)
{
    //This variable is used to treat the exceptions for the Math in the Loop interactions
    int Txn_variable=0;
    
    if((txn==0 || txn==2) && t>0)
        Txn_variable= N_TXN -1;      // This solve the exception for Loop Timestep
    else
        Txn_variable= txn-1;
    
    if(txn==0 && t==0)             // This solves the exception for the first execution in the first timestep
    {
        Result.n_commits = Source.n_commits ;
        Result.n_deadlocks_raw = Source.n_deadlocks_raw ;
        Result.n_deadlocks_war = Source.n_deadlocks_war ;
        Result.n_deadlocks_waw = Source.n_deadlocks_waw ;
        Result.n_invalidations = Source.n_invalidations ;
        Result.n_aborts = Source.n_aborts ;
        Result.n_writes = Source.n_writes ;
        Result.n_reads = Source.n_reads  ;
        Result.t_waiting_raw = Source.t_waiting_raw;
        Result.t_waiting_war = Source.t_waiting_war;
        Result.t_waiting_waw =Source.t_waiting_waw ;
    }// As Txn_variable already has its value, no else needed.
    
    
    //Variable i is used to define if the statistics are for Thread or for Function.
    // i > 0 -> Statistics for Thread
    // i < 0 -> Statistics by Function
    if(i >= 0)
    {
        Result.n_commits = Source.n_commits - t_aux[i][Txn_variable].n_commits ;
        Result.n_deadlocks_raw = Source.n_deadlocks_raw - t_aux[i][Txn_variable].n_deadlocks_raw ;
        Result.n_deadlocks_war = Source.n_deadlocks_war - t_aux[i][Txn_variable].n_deadlocks_war ;
        Result.n_deadlocks_waw = Source.n_deadlocks_waw - t_aux[i][Txn_variable].n_deadlocks_waw ;
        Result.n_invalidations = Source.n_invalidations - t_aux[i][Txn_variable].n_invalidations ;
        Result.n_aborts = Source.n_aborts - t_aux[i][Txn_variable].n_aborts ;
        Result.n_writes = Source.n_writes - t_aux[i][Txn_variable].n_writes ;
        Result.n_reads = Source.n_reads - t_aux[i][Txn_variable].n_reads ;
        Result.t_waiting_raw = Source.t_waiting_raw - t_aux[i][Txn_variable].t_waiting_raw;
        Result.t_waiting_war = Source.t_waiting_war - t_aux[i][Txn_variable].t_waiting_war;
        Result.t_waiting_waw =Source.t_waiting_waw - t_aux[i][Txn_variable].t_waiting_waw;
        
    }
    else
    {
        Result.n_commits = Source.n_commits - t_aux2[Txn_variable].n_commits ;
        Result.n_deadlocks_raw = Source.n_deadlocks_raw - t_aux2[Txn_variable].n_deadlocks_raw ;
        Result.n_deadlocks_war = Source.n_deadlocks_war - t_aux2[Txn_variable].n_deadlocks_war ;
        Result.n_deadlocks_waw = Source.n_deadlocks_waw - t_aux2[Txn_variable].n_deadlocks_waw ;
        Result.n_invalidations = Source.n_invalidations - t_aux2[Txn_variable].n_invalidations ;
        Result.n_aborts = Source.n_aborts - t_aux2[Txn_variable].n_aborts ;
        Result.n_writes = Source.n_writes - t_aux2[Txn_variable].n_writes ;
        Result.n_reads = Source.n_reads - t_aux2[Txn_variable].n_reads ;
        Result.t_waiting_raw = Source.t_waiting_raw - t_aux2[Txn_variable].t_waiting_raw;
        Result.t_waiting_war = Source.t_waiting_war - t_aux2[Txn_variable].t_waiting_war;
        Result.t_waiting_waw =Source.t_waiting_waw - t_aux2[Txn_variable].t_waiting_waw;
    }
    
    
    /* The code below prints the statistics in Output File  */
    
    fprintf( out_f, "%llu;", Result.n_commits );
    fprintf( out_f, "%llu;", Result.n_deadlocks_raw );
    fprintf( out_f, "%llu;", Result.n_deadlocks_war );
    fprintf( out_f, "%llu;", Result.n_deadlocks_waw );
    fprintf( out_f, "%llu;", Result.n_invalidations );
    fprintf( out_f, "%llu;", Result.n_aborts );
    double t_waiting = Result.t_waiting_raw + Result.t_waiting_war + Result.t_waiting_waw;
    fprintf( out_f, "%.4lf;", t_waiting );
    fprintf( out_f, "%.4lf;", ((double)(Result.n_aborts))/((double)(Result.n_commits + Result.n_aborts)) );
    fprintf( out_f, "%.4lf;", ((double)(Result.n_writes))/((double)(Result.n_reads + Result.n_writes)) );
    
    return 0;
}


int statistics(int txn,FILE *out_f,int t)
{
    int i;
    double txn_duration=0;
    
    /* The Header is printed just in the first execution */
    if (txn==0 && t==0)
        fprintf(out_f, "\nTxn;Thread;Duration;N_CMT;N_DLK_RAW;N_DLK_WAR;N_DLK_WAW;N_INV;N_ABR;T_WT_ALL;AbRatio;WrRatio;\n");
    
    //fprintf(out_f, "\nTxn  Thread   Duration   N_CMT  N_DLK_RAW  N_DLK_WAR  N_DLK_WAW  N_INV  N_ABR  T_WT_ALL   AbRatio   WrRatio \n");
    
    t_tx_cnt[txn]=tx_cnt[txn]=0;
    
    int Txn_variable;
    
    for (i = 0; i < N_TH; i++) {
        t_tx_cnt[txn] += t_tx[i];
        tx_cnt[txn] += tx[i];
        tx[i] = t_tx[i] = 0;
        
        /* Save Durations for the Thread in the Out File */
        fprintf(out_f, "\n%d;%d;%.4lf;",txn,i,durations[i][txn]);
        
        /* Save LibTM Statistics for the Thread in the Aux Variable */
        t_aux[i][txn] = stats_get(i);
        
        /* Calculates the values for each Thread and print in the File */
        Math(txn,i,t,t_aux[i][txn],thread_stats[i][txn],out_f) ;
        
        /* Acumulates the durations, used for the Function statistics */
        txn_duration+=durations[i][txn];
        
    }
    
    /* Save Durations for the Function in the Out File */
    fprintf(out_f, "\nT;%d;%.4lf;",txn,txn_duration);
    
    /* Save LibTM Statistics for the Function in the Aux Variable */
    t_aux2[txn] = stats_get_total();
    
    /* Calculates the values for the function and print in the File */
    Math(txn,-1,t,t_aux2[txn],tot_stats[txn],out_f) ;
    
    /* Reset the counter variables for the next LibTM Function */
    _t_tx = _tx = 0 ;
    
    
    return 0;
}


main(int argc, char **argv)
{
    int i,id,t ;
    void *arg;
    
    /* Test Number of Arguments */
    if(argc < 6){
        printf("usage : CD CR Max_Partners Timesteps filename\n");
        return 0;
    }
    
    /* Define Output file for statistics */
    FILE* out_f = NULL;
    out_f = fopen(argv[5], "a" );
    
    /* Define Method of Conflict Detection and Conflict Resolution */
    set_version (atoi(argv[1]), atoi(argv[2]));
    
    /* Define Arguments */
    maxp = atoi(argv[3]);
    timesteps = atoi(argv[4]);
    
    /* The special characters are used as pattern for the script that analyse the statistics */
    fprintf( out_f, "\n######### - CD: %d  CR: %d - N_TH %d - maxp: %d timesteps: %d - %s %s  - #######\n",atoi(argv[1]),atoi(argv[2]),N_TH,atoi(argv[3]),atoi(argv[4]),__DATE__,__TIME__);
    
    /* Allocate variables */
    partners = new tm_int *[N_ELEM];
    
    for (i = 0; i < natoms; i++)
        partners[i] = new tm_int [maxp];
    
    
    CREATE_TM_THREADS (N_TH);
    
    /*  Function 01 */
    PARALLEL_EXECUTE (N_TH, Insert_Random_values, arg);
    statistics(0,out_f,t);
    
    /*  Function 02 */
    PARALLEL_EXECUTE (N_TH, Map_Neighbors, arg);
    statistics(1,out_f,t);
    
    for (t = 0; t < timesteps; t++)
    {
        /*  Function 03 */
        PARALLEL_EXECUTE (N_TH, Calculates_Forces, arg);
        statistics(2,out_f,t);
        
        /*  Function 04 */
        PARALLEL_EXECUTE (N_TH, Push_Atoms, arg);
        statistics(3,out_f,t);
        
        /*  Function 05 */
        Calculates_xsum(arg,id);
        
         printf("xsum = %10.4lf\n", (double)xsum);
        //fprintf( out_f, "\n");
    }
    
    DESTROY_TM_THREADS (N_TH);
    
    return 0;
}
