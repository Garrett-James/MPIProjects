/* Garret James, Felix Perez, Antonio Bujanda
   Professor Zhuang
   CS 4379-Parallel Programming
   Assignment 2
   March 2, 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

/* Function Declarations */
void myreduce(int * buffer, int rank, int worldSize);
void mybarrier(int rank, int worldSize);
void mybroadcast(int buffer, int rank, int worldSize);

int main(int argc, char ** argv){
    
    /* MPI Standard Variables*/
    int myrank;                    //rank of process
    int worldSize;                //number of processes
    MPI_Status status;            //status buffer for receive
    MPI_Request request;        //request buffer for receive

    srand(time(NULL));             //random number generator
    int buffer = 1;                //data buffer
    
    // Initialize the MPI Environment
    MPI_Init(&argc, &argv);
    
    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    
    // Find the process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    
    printf("MPI task %d has started.\n", myrank);
    
    myreduce(&buffer, myrank, worldSize);
    mybarrier(myrank, worldSize);
    mybroadcast(buffer, myrank, worldSize);

    printf("The program at rank %d is complete.\n", myrank);

    // Finalize the MPI environment
    MPI_Finalize();
    
    return 0;
}// End of Main

/*
    myreduce() will sum the values stored in all buffers of all processors.
    This is done similarly to mybroadcast(). First, odd processors n will send their buffer values to processor
    n-1 which will receive the value and add it to their buffer. 

    Second, the even processors in the first group of sends, that is all processors that fulfill 2^1+z(2^2), where z is any integer, will send the buffer to processors in the second group of sends. 

This will repeat with each group of sends being defined as processors that fulfill 2^x+z(2^(x+1)), where x is the step that it is sent. 
*/
void myreduce(int * buffer, int rank, int worldSize){
    int buffer2, tag = 99;
    int x, z;
    MPI_Status status;
    
    MPI_Status status;
    //If the processor is odd, send to the processor before it
    if(rank%2 == 1){
        MPI_Send(buffer, 1, MPI_INT, rank - 1, tag, MPI_COMM_WORLD);
    }
    else{
        MPI_Recv(&buffer2, 1, MPI_INT, rank + 1, tag, MPI_COMM_WORLD, &status);
        buffer+=buffer2;
    }
    /*Processors are grouped according to 2^x+z(2^(x+1)) where x is the step number. Each step, the buffer is sent to processors of the next group. This repeats until processor 0 has the sum of all buffers.*/
    for(x = 1; x < log(worldSize); x++){
        for(z = 0; z < log(worldSize) - x; z++){
            if(rank == (pow(2,x) + z * pow(2,x+1))){
                MPI_Send(buffer, 1, MPI_INT, rank - pow(2,x), tag, MPI_COMM_WORLD);
            }
            
            if(rank==(z*pow(2,x+1))){
                MPI_Recv(&buffer2,1,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &status);
                *buffer = *buffer + buffer2;
            }
        }
    }
}

/*
    mybarrier() will stop all processors from proceeding past until every processor has reached it. 
    This is accomplished by all non-root processors sending a communication to the root processor and 
    then making a blocking receive call waiting for a response. Until a receive is made, it will remain 
    in place waiting.

    The root processor makes n-1 blocking receives, where n is the number of processors. Once it has 
    received n-1 messages, meaning every processor has reached mybarrier, the root processor will begin 
    sending messages to all other processors to allow them to leave the barrier.
*/
void mybarrier(int rank, int worldSize){
    int i, buffer = 1;
    MPI_Status status;

    if(rank == 0){
        for(i = 1; i < worldSize; i++){ //number of ranks - 1
            MPI_Recv(&buffer, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
        }
        
        for( int i = 1; i < worldSize; i++){ //number of ranks - 1
            MPI_Send(&buffer, 1, MPI_INT, i, i+100, MPI_COMM_WORLD);
        }
    }
    else{
        MPI_Send(&buffer, 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
        MPI_Recv(&buffer, 1, MPI_INT, 0, rank + 100, MPI_COMM_WORLD, &status);
    }
    
    return;
}

/*
    mybroadcast() will send information from the root processor to all other processors.

    If the processor is not the root, it will make a blocking receive and wait for the data from rank 0
    or another processor that has already received the data. Once the data is received, the processor will
     send the data to another processor until all processors have received the data.
*/
void mybroadcast(int buffer, int rank, int worldSize){
    int step, tag = 99;
MPI_Status status;    

    if(rank != 0){
        MPI_Recv(&buffer,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD, &status);
    }
    for(step = 1; step <= worldSize/2; step *= 2){
        
        if(rank < step){
            MPI_Send(&buffer, 1, MPI_INT,step + rank, tag, MPI_COMM_WORLD);
        }
    }
}
