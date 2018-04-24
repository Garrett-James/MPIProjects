/*Garret James, Antonio Bujanda, Felix Perez
  Professor Zhuang
  CS 4379 - Parallel and Concurrent
  April 9, 2018
  
  Code Explanation:
   Each processor belongs to 3 communication groups, global, horizontal, and vertical
   Each processor in the same column belongs to a communication group consisting of all processors in the column.
   Each processor in the same row belongs to a communication group consisting of all processors in the row.
   For each k, each processor in the same row or column communication group needs the same set of values from the same processor.
   For each k, each processor in the same row group needs the same column of W values, W[i,k], from the same processor.
   For each k, each processor in the same col group needs the same row of W values, W[k,j], from the same processor.
   The processor that has the needed row or col is found with k/(n/sqrt(p)).
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define N 16

/* Function Declarations=================================== */
int ** initialize(int n);
void populate(int ** matrix0, int ** matrix1, int n);
void deallocate(int ** matrix, int n);
void print2DArray(int ** matrix, int n);
int min(int a, int b);

/*==========================================================*/

int main(int argc, char ** argv){
	
	/* MPI standard variables */
	int myrank;						// rank of process
	int worldSize;					// # of processes
	int tag = 99;					// tag for messages
	MPI_Status status;				// status buffer for receive
	MPI_Request request;			// request object for receive
	
	/* MPI subgroup variables */
	int horizColor;					// key 'horizontal' color
	int vertColor;					// key 'vertical' color
	int myhoriz_rank;				// rank of process in MPI_COMM_HORIZONTAL
	int myvert_rank;				// rank of process in MPI_COMM_VERTICAL
	MPI_Comm MPI_COMM_VERTICAL;		// Vertical Communicator
	MPI_Comm MPI_COMM_HORIZONTAL;	// Horizontal Communicator

	// Initialize the MPI Environment
	MPI_Init(&argc, &argv);
	
	// Get the number of processes
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
	
	// Find the process rank
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	srand(time(NULL) + myrank);
  
	horizColor = myrank / (int)sqrt(worldSize);
	vertColor = myrank % (int)sqrt(worldSize);

	// Split MPI_COMM_WORLD into horizontal and vertical subsections
	MPI_Comm_split(MPI_COMM_WORLD, horizColor, myrank, &MPI_COMM_HORIZONTAL);
	MPI_Comm_split(MPI_COMM_WORLD, vertColor, myrank, &MPI_COMM_VERTICAL);
	
	// Get the new process ranks from communicators
	MPI_Comm_rank(MPI_COMM_HORIZONTAL, &myhoriz_rank);
	MPI_Comm_rank(MPI_COMM_VERTICAL, &myvert_rank);
	
	/* Our Variables */
	int i, j, k;
	int subN = N / (int)sqrt(worldSize);
	int ** W = initialize(subN);  // Initialize W
	int ** W0 = initialize(subN); // Initialize W0
	populate(W,W0,subN);
	int global_k_row[subN];
	int global_k_col[subN];
 
	for(k = 0; k < N; k++){
		//Populate global_k_row or col
			for(j = 0; j < subN; j++){
				global_k_row[j] = W0[k%subN][j]; 
			}
			for(i = 0; i < subN; i++){
				global_k_col[i] = W0[i][k%subN]; 
			}
		//The correct root processor, found with k/subN, broadcasts its global_k_col or row
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Bcast(global_k_row, subN, MPI_INT, k/subN, MPI_COMM_VERTICAL);
		MPI_Bcast(global_k_col, subN, MPI_INT, k/subN, MPI_COMM_HORIZONTAL);
		MPI_Barrier(MPI_COMM_WORLD);
		
    //the inner for loops function the same as the original given function but only loop n/sqrt(p), or subN.
		for(i = 0; i < subN; i++){
			for(j = 0; j < subN; j++){
        //the w0[k,j] + w0[i,k] read from the global_k_row or col instead.
				W[i][j] = min(W0[i][j], global_k_row[j] + global_k_col[i]);
			}
		}
		
		for(i = 0; i < subN; i++){
			for(j = 0; j < subN; j++){
				W0[i][j] = W[i][j];
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
	
 //Testing purpose
	for(i=0;i<worldSize;i++){
		if(myrank==i){
			print2DArray(W0,subN);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
 
	// Free memory
	deallocate(W, subN);
	deallocate(W0, subN);
	
	
	
	// Finalize MPI Environment
	MPI_Finalize();
	
	return 0;
	
}//End of Main

void populate(int ** matrix0, int ** matrix1, int n){
	int i, j;
	
	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++){
			matrix0[i][j] = (rand() % 10)+1;
			matrix1[i][j] = matrix0[i][j];
		}
	}
}

void deallocate(int ** matrix, int n){
	int i;
	
	for(i = 0; i < n; i++){
		free(matrix[i]);
	}
	
	free(matrix);
	matrix = NULL;
}

int ** initialize(int n){
	int i, j, ** matrix;
	
	matrix = (int **)malloc(n * sizeof(int*));
	
	for(i = 0; i < n; i++){
		matrix[i] = (int *)malloc(n * sizeof(int));
	}
	
	return matrix;
}

void print2DArray(int ** matrix, int n){
	int i, j;
	
	for(i = 0; i < n; i++){
		printf("row %d | ", i);
		for(j = 0; j < n; j++){
			if(j == n - 1){
				printf("%d\n", matrix[i][j]);
			}
			else{
				printf("%d ", matrix[i][j]);
			}
		}
	}
	printf("\n");
}

int min(int a, int b){
	if(a < b){
		return a;
	}
	return b;
}
