#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

struct calibration
{
    double slope[12];
    double offset[12];

};

int main ()
{
	char* fname = "img.h5";
	hid_t  file_id , dataset_id , dataspace_id , file_dataspace_id;
	hsize_t* dims;
	hssize_t  num_elem;
	herr_t status;
	int  rank;
	int  ndims;
	int rows , cols , stride;
	struct calibration* A;

	hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(struct calibration));

	/* Open  existing  HDF5  file */
	file_id = H5Fopen(fname , H5F_ACC_RDONLY , H5P_DEFAULT );
	/* Open  existing  dataset  */
	dataset_id = H5Dopen(file_id , "U-MARF/MSG/Level1.5/METADATA/HEADER/RadiometricProcessing/Level15ImageCalibration_ARRAY", H5P_DEFAULT );

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
	A = (struct calibration *)  malloc(num_elem * sizeof(struct calibration));
	rows = dims [0];
	cols = dims [1];
	stride = cols;
	free(dims);

	/*  Create  dataspace  */
	dataspace_id = H5Screate_simple(rank , dims , NULL);
	/* Read  matrix  data  from  file */
	//status = H5Dread(dataset_id , H5T_NATIVE_DOUBLE , dataspace_id ,file_dataspace_id , H5P_DEFAULT , A); originale

	status = H5Dread(dataset_id , h5cal_t, H5S_ALL ,dataspace_id , H5P_DEFAULT , A);

	//status = H5Dread(dataset, h5cal_t, H5S_ALL, dataspace, H5P_DEFAULT, cal.data()); prof

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