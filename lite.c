#include <hdf5.h>
#include <hdf5_hl.h>
//#include <H5LTpublic.h>


int main( void )
{
 hid_t       file_id;
 int         data[6];
 hsize_t     dims[2];
 size_t     i, j, nrow, n_values;

 /* open file from ex_lite1.c */
 file_id = H5Fopen ("img.h5", H5F_ACC_RDONLY, H5P_DEFAULT);

 /* read dataset */
 H5LTread_dataset(file_id,"U-MARF/MSG/Level1.5/METADATA/HEADER/RadiometricProcessing/Level15ImageCalibration_ARRAY",H5T_NATIVE_INT,data);

 /* get the dimensions of the dataset */
 H5LTget_dataset_info(file_id,"U-MARF/MSG/Level1.5/METADATA/HEADER/RadiometricProcessing/Level15ImageCalibration_ARRAY",dims,NULL,NULL);

 /* print it by rows */
 n_values = (size_t)(dims[0] * dims[1]);
 nrow = (size_t)dims[1];
 for (i=0; i<n_values/nrow; i++ )
 {
  for (j=0; j<nrow; j++)
   printf ("  %d", data[i*nrow + j]);
  printf ("\n");
 }

 /* close file */
 H5Fclose (file_id);

 return 0;


}