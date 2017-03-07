#include <stdio.h>
#include <stdlib.h>
#include <hdf5.h>

int main()
{
	/*  Declare  necessary  variables  */
const  char* fname = "img.h5";
hid_t  file_id , dataset_id , dataspace_id , file_dataspace_id,status;
hsize_t* dims;
hssize_t  num_elem;
int  rank;
int  ndims;
int rows , cols , stride;
double* A;
/* Open  existing  HDF5  file */
//file_id = H5Fopen(fname , H5F_ACC_RDONLY , H5P_DEFAULT );
file_id = H5Fopen("img.h5", H5F_ACC_RDONLY , H5P_DEFAULT );
/* Open  existing  dataset  */
dataset_id = H5Dopen(file_id , "/Matrix", H5P_DEFAULT );


/*  Determine  dataset  parameters  */
file_dataspace_id = H5Dget_space(dataset_id );
rank = H5Sget_simple_extent_ndims(file_dataspace_id );
dims = (hsize_t *)  malloc(rank * sizeof(hsize_t ));
ndims = H5Sget_simple_extent_dims(file_dataspace_id , dims ,
NULL);
if (ndims != rank)
{
fprintf(stderr , "Expected  dataspace  to be  dimension ");
fprintf(stderr , "%d but  appears  to be %d\n", rank , ndims );
}
/*  Allocate  matrix  */
num_elem = H5Sget_simple_extent_npoints(file_dataspace_id );
A = (double *)  malloc(num_elem * sizeof(double ));
rows = dims [0];
cols = dims [1];
stride = cols;
free(dims);

/*  Create  dataspace  */
dataspace_id = H5Screate_simple(rank , dims , NULL);
/* Read  matrix  data  from  file */
status = H5Dread(dataset_id , H5T_NATIVE_DOUBLE , dataspace_id ,
file_dataspace_id , H5P_DEFAULT , A);
/*  Release  resources  and  close  file */
status = H5Dclose(dataset_id );
status = H5Sclose(dataspace_id );
status = H5Sclose(file_dataspace_id );
status = H5Fclose(file_id );
/*
* ....
* Do  something  with  matrix
* ...
*/
free(A);
}