/*
 * Name: 민두기
 * Student ID: 2012-11598
 * Design 설명: 
 * set index가 5bit이고, block index가 5bit라 32개의 set과 32byte의
 * block이 존재하는 cache이다. 그러나 Direct mapped Cache이기 때문에
 * set index가 동일하고 tag가 다르면 무조건 cache miss가 나고
 * eviction이 일어나게 된다. 
 * Test set의 size가 모두 다르기 때문에 각각의 경우에 다른 
 * transpose함수를 사용해야 원하는 조건을 만족시킬 수 있어 
 * 함수포인터를 이용하였다.
 * 32byte의 block size를 활용하기 위하여 8 x 8로 tile화 하여
 * transpose하였다.
 * 32 x 32 size 행렬의 경우 B[i][i] = A[i][i]를 할 때 eviction이 생겨서
 * cache miss를 유발한다는 것을 알 수 있다. 이는 block size가 32byte라 
 * set index가 8개의 integer마다 1씩 증가하지만 set의 크기가 32개 뿐이라
 * 어는 순간 set index가 같은 원소를 tranpose 하기 때문이다. 32 x 32의 경우
 * tile을 transpose할 때 set index가 같은 것 끼리 transpose하는
 * 경우는 B[i][i] = A[i][i]일 때 뿐이다. 그러므로 이때 A[i][i]의
 * 값을 미리 저장해 두었다가 루프가 끝나고 B[i][i]에 넣어주는 방식으로
 * cache miss를 300미만으로 만들 수 있었다.
 * 동일한 함수를 사용하여 61 x 67 size 행렬도 cache miss를 2000미만으로
 * 낮출 수 있었다.
 * 64 * 64 size 행렬의 경우 한 tile에서 B[i][i] = A[i][i]일 때 뿐만아니라
 * B[i + 4][i] = A[i][i + 4], B[i][i + 4] = A[i + 4][i]일 때도 cache miss
 * 가 나게 된다. Tile의 크기를 4 x 4로 줄이고, 32 x 32 size 행렬의 경우와
 * 마찬가지로 cache miss가 생기는 경우 미리 register에 넣어 놨다가
 * 다시 넣는 형식으로 해보았지만 1600밑으로는 줄어 들지 않았다.
 * blocksize가 32byte인데 이를 제대로 활용 못했기 때문이다. 
 * 그래서 8 x 8 tile size를 그대로 유지하고 좀더 활용해 보기로 하였다.
 * tile:
 *  a	b
 *  c	d
 * 라고 하면 (a, b, c, d)의 크기는 모두 4 x 4이다. a를 tranpose 할 때
 * 무난하게 cache miss를 피하면서 transpose해준다. 그러나 추가적으로
 * a를 tranpose할 때 사용되었으며 cache되어있는 b부분을 B의 b부분에 
 * 넣어준다. 이때 자연스럽게 b부분이 transpose되면서B의 b부분에
 * 들어가진다.
 * 그후 B의 b부분을 memory에 넣고 A의 c부분을 받아 transpose해서
 * B의 b부분에 넣고 앞서 memory에 담았던 것을 B의 c부분에 넣는다.
 * 마지막으로 d부분을 cache miss를 피해가며 담으면 transpose가
 * 완료된다. 이 방법을 사용해 cache miss를 1300미만으로 만들 수 있었다.
 */
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

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    transpose tr;

    if (M == 64) {
	tr = transpose_64;
    } else {
	tr = transpose_32;
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

