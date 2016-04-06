/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

typedef void (*transpose)(int M, int N, int A[N][M], int B[M][N]);
void transpose_32(int M, int N, int A[N][M], int B[M][N]);
void transpose_64(int M, int N, int A[N][M], int B[M][N]);
void transpose_61(int M, int N, int A[N][M], int B[M][N]);

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    transpose tr;

    if (M == 32) {
	tr = transpose_32;
    } else if (M == 64) {
	tr = transpose_64;
    } else {
	tr = transpose_61;
    }

    tr(M, N, A, B);
}
void transpose_32(int M, int N, int A[N][M], int B[M][N])
{   
    int row_block = 8;
    int col_block = 8;
    int temp;
    for (int i = 0; i < N; i += row_block) {
	for (int j = 0; j < M; j += col_block) {
	    for (int k = j; k < j + col_block && k < M; k++) {
		if (i == j)
		    temp = A[k][k];
		for (int l = i; l < i + row_block && l < N; l++) {
		    if ((l == k) && (i == j)) 
			continue;
		    B[k][l] = A[l][k];
		}
		if (i == j)
		    B[k][k] = temp;
	    }	
	}
    }
}

void transpose_64(int M, int N, int A[N][M], int B[M][N])
{
    int temp1;
    int temp2;
    int temp3;
    int temp4;

    for (int i = 0; i < N; i += 8) {
	for (int j = 0; j < M; j += 8) {
	    for (int k = j; k < j + 4 && k < M; k++) {
		if (i == j) {
		    temp1 = A[k][k];
		    temp2 = A[k][k + 4];
		}
		for (int l = i; l < i + 4 && l < N; l++) {
		    if ((l == k) && (i == j))
			continue;
		    B[k][l] = A[l][k];	
		    B[k][l + 4] = A[l][k + 4];
		}
		if (i == j) {
		    B[k][k] = temp1;
		    B[k][k + 4] = temp2;
		}
	    }

	    for (int k = j; k < j + 4 && k < M; k++) {
		temp1 = B[k][i + 4];
		temp2 = B[k][i + 5];
		temp3 = B[k][i + 6];
		temp4 = B[k][i + 7];
	    	    
	    	B[k][i + 4] = A[i + 4][k];
		B[k][i + 5] = A[i + 5][k];
		B[k][i + 6] = A[i + 6][k];
		B[k][i + 7] = A[i + 7][k];

		B[k + 4][i] = temp1;
		B[k + 4][i + 1] = temp2;	
		B[k + 4][i + 2] = temp3;
		B[k + 4][i + 3] = temp4;
	    }
    
	    for (int k = j + 4; k < j + 8 && k < M; k++) {
		if (i == j)
		temp1 = A[k][k];
		for (int l = i + 4; l < i + 8 && l < N; l++) {
		    if ((l == k) && (i == j))
			continue;
		    B[k][l] = A[l][k];
		}
		if (i == j)
		    B[k][k] = temp1;
	    }
	}
    }
}
void transpose_61(int M, int N, int A[N][M], int B[M][N])
{
    int row_block = 8;
    int col_block = 4;
    for (int i = 0; i < N; i += row_block) {
	for (int j = 0; j < M; j += col_block) {
	    for (int k = j; k < j + col_block && k < M; k++) {
		for (int l = i; l < i + row_block && l < N; l++) {
		    B[k][l] = A[l][k];
		}
	    }
	}
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

}

