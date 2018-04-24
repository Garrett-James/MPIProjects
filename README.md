# MPIProjects
A collection of various MPI projects. 

## CustomMPI.c
CustomMPI is a collection of simplified implementations of functions already available in MPI.  
Functions include:  
1. mybarrier()  
  mybarrier() functions similarly to MPI_Barrier(). Processors will enter mybarrier() and will not exit the function until all processors in the MPI_COMM_WORLD communication group enter.
2. mybroadcast()  
  mybradcast() functions similarly to MPI_Broadcast() and has a complexity of O(logN), where N is the number of processors. The integer in buffer be sent to all other processors in MPI_COMM_WORLD. 
3. myreduce()  
  myreduce() functions similarly to MPI_Reduce(). The integer in each processors buffer will be summed together and sent to processor 0. 
  
## FloydMPI.c  
FloydMPI is a parallel implementation of the Floyd-Warshall algorithm. Special consideration was given to the communication overhead and amount of memory allocated to each processor. Specifically, for each iteration of k, communication overhead could be at most O(N/sqrt(p)log2(p)). This is generally N broadcasts, where N is the size of the larger matrix and p is the number of processors.

Each processor was allowed access to only a portion of the larger array and was grouped into horizontal and vertical communication groups to receive the necessary information from other processors.
