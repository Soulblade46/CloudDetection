#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <stdbool.h>
#include <math.h>

#define SIZE_IMAGE 2000


int* eumetsat;
//int matrix_scaling[3][3]={{cx,0,0},{0,cy,0},{0,0,1}};
int* mask_final_eumet;
//int pixel_matrix[3]={v,w,1};

/*void matrix_prod(int matA[],int dim_A,int matB[],int dim_B,int matC[],int dim_C)
{
	for(i=0;i<dim_A;i++){
    	for(j=0;j<dim_B;j++){
        	matC[i + j*dim_C]=0;
        	for(k=0;k<SIZE_IMAGE;k++){
            	matC[i + j*SIZE_IMAGE]+=matA[i + k*SIZE_IMAGE]*matB[k + j*SIZE_IMAGE];
        	}
    	}
	}
}*/

void setup_final_mask_eumet()
{
	mask_final_eumet=malloc(sizeof(int)*DIM*DIM);
	for (int i = 0; i < DIM*DIM; ++i)
	{
		mask_final_eumet[i]=-1;
	}
}

void inter()
{
	for (int i = 0; i < DIM; ++i)
	{
		for (int j = 0; j < DIM; ++j)
		{
			if ((mask_final_eumet[i + j*DIM]==-1) && (((i + j*DIM)+1)<DIM))
			{
				mask_final_eumet[i + j*DIM]=mask_final_eumet[1+i + j*DIM];
			}
		}
	}
}

void zoom()
{
	int* mask_final_eumet;
    int px, py;
    int w1=DIM;
    int h1=DIM;
    int w2=1000;
    int h2=1000;
    mask_final_eumet=malloc(sizeof(int)*DIM*DIM);
    for (int i = 0; i<h2; i++) {
        for (int j = 0; j<w2; j++) {
            px = floor(j*3);
            py = floor(i*3);
            mask_final_eumet[(i*w2) + j] = data[(py*w1) + px];
        }
    }            
}