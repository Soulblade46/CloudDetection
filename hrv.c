#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <grib_api.h>

#define SIZE_IMAGE 2000

unsigned short* mask_final_eumet;
//int matrix_scaling[3][3]={{cx,0,0},{0,cy,0},{0,0,1}};
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

/*void setup_final_mask_eumet()
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
}*/

/*unsigned short* zoom(unsigned short data[],int dim,int dim_hd_x,int dim_hd_y)
{
    int px, py;
    int w1=dim;
    int h1=dim;
    int w2=dim_hd_x;
    int h2=dim_hd_y;
    mask_final_eumet=malloc(sizeof(unsigned short)*dim_hd_x*dim_hd_y);
    for (int i = 0; i<h2; i++) {
        for (int j = 0; j<w2; j++) {
            px = floor(j*3);
            py = floor(i*3);
            mask_final_eumet[(i*w2) + j] = data[(py*w1) + px];
        }
    }
    return mask_final_eumet;         
}*/


unsigned short * zoom(unsigned short * pixels,int w1,int h1,int w2,int h2) {
    unsigned short * temp = malloc(sizeof(unsigned short)*w2*h2);
    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;
    int x2, y2 ;
    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            temp[(i*w2)+j] = pixels[(y2*w1)+x2] ;
        }                
    }                
    return temp;
}

int count_file=0;
size_t values_len= 0;

void set_eumetsat_mask(num_tot_files)
{

}

unsigned short* get_eumetsat_mask(char* filename,int num_tot_files)
{
	int err = 0,i;
  double *values = NULL;
  double max,min,average;
  //size_t values_len= 0;

  #ifndef eumetsat_mask
  unsigned short* eumetsat_mask[num_tot_files];
	#endif
 
  FILE* in = NULL;
  //char* filename ;
  grib_handle *h = NULL;
 
  printf("ok\n");
  //filename="mask.grb";
 
  in = fopen(filename,"r");
  if(!in) {
    printf("ERROR: unable to open file %s\n",filename);
  }
 
  /* create new handle from a message in a file*/
  h = grib_handle_new_from_file(0,in,&err);
  if (h == NULL) {
    printf("Error: unable to create handle from file %s\n",filename);
  }
 
 
  /* get the size of the values array*/
  GRIB_CHECK(grib_get_size(h,"values",&values_len),0);
 
  values = malloc(values_len*sizeof(double));
 
  /* get data values*/
  GRIB_CHECK(grib_get_double_array(h,"values",values,&values_len),0);

  /*for (i = 0; i < num_tot_files; ++i)
  {
  	eumetsat_mask[i]=malloc(sizeof(int)*values_len);
  }*/
 
  eumetsat_mask[count_file]=malloc(sizeof(unsigned short)*values_len);
  for(int j=0;j< values_len;++j)
  {
    //printf("%d  %.10e\n",i+1,values[i]);
    eumetsat_mask[count_file][j]=(unsigned short)values[j];
    //printf("value=%d\n",(int)values[j]);
  }
  //++count_file;
  fclose(in);
  return eumetsat_mask[count_file++];
      
 
  /*free(values);
 
 
  GRIB_CHECK(grib_get_double(h,"max",&max),0);
  GRIB_CHECK(grib_get_double(h,"min",&min),0);
  GRIB_CHECK(grib_get_double(h,"average",&average),0);
 
  printf("%d values found in %s\n",(int)values_len,filename);
  printf("max=%.10e min=%.10e average=%.10e\n",max,min,average);
 
  grib_handle_delete(h);
 
  fclose(in);*/
}