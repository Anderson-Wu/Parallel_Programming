/* circuitSatifiability.c solves the Circuit Satisfiability
 *
 *   The particular circuit being tested is "wired" into the
 *   logic of function 'checkCircuit'. All combinations of
 *   inputs that satisfy the circuit are printed.
 *
 *   16-bit version by Michael J. Quinn, Sept 2002.
 *   Extended to 32 bits by Joel C. Adams, Sept 2013.
 */

#include <stdio.h>     // printf()
#include <stdlib.h>
#include <limits.h>    // UINT_MAX
#include <mpi.h>
#include <time.h>

long long calculate_part(long long );
long long tree_sum(int ,int ,long long);

void tree_bcast(int my_rank,int comm_sz,long long int total_toss,long long int* my_toss){
    if(my_rank == 0){
        long long int toss = total_toss/comm_sz;
        *my_toss = toss;
        for(int i = 1;i < comm_sz;i++){
            if(i != comm_sz-1){
                MPI_Send(&toss,1,MPI_LONG_LONG,i,0,MPI_COMM_WORLD);
            }
            else{
                toss = total_toss-toss*(comm_sz-1);
                MPI_Send(&toss,1,MPI_LONG_LONG,i,0,MPI_COMM_WORLD);
            }
        }
    }
    else{
        MPI_Recv(my_toss,1,MPI_LONG_LONG,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    }

}


int main (int argc, char *argv[]) {

   int comm_sz;
   int my_rank;
   MPI_Init(NULL,NULL);
   MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
   MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

   long long int total_tosses = 100000000;
   long long int my_toss;

   srand(time(NULL));
   double startTime = 0.0, totalTime = 0.0;


   startTime = MPI_Wtime();

   tree_bcast(my_rank,comm_sz,total_tosses,&my_toss);
   

   long long val = calculate_part(my_toss);
   long long result = tree_sum(my_rank,comm_sz,val);
   
   totalTime = MPI_Wtime()-startTime;
   if (my_rank == 0)
   {
        printf("Process %d finished in time %f secs.\n", my_rank, totalTime);
        printf("pi is approximately %.16lf\n", 4 *((double) result / total_tosses));
        fflush(stdout);
    }

   MPI_Finalize();

   return 0;
}



long long calculate_part(long long time){
    long long  count = 0;
    double x, y, distance_squared;
    for (long long i = 0; i < time; i++)
    {
        x = (double)rand()/RAND_MAX;
        y = (double)rand()/RAND_MAX;
        distance_squared = x *x + y * y;
        if (distance_squared <= 1.0)
            count++;
    }
   return count;

}


long long tree_sum(int my_rank,int total_process,long long val){
   long recvval;
   long my_val = val;
   int half;
   int odd;
   while(total_process != 1){
      odd = total_process%2;
      half = total_process/2;
      if(my_rank < half){
         MPI_Recv(&recvval,1,MPI_LONG,my_rank+half+odd,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
         my_val += recvval;
      }
      else if(my_rank >= (half + odd)){
         MPI_Send(&my_val,1,MPI_LONG,my_rank-half-odd,0,MPI_COMM_WORLD);
      }
      total_process = half + odd;      
   }
   return my_val;
}

