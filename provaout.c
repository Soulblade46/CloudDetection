#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM 3712

char* file="out.h5";
int main() {
   hid_t       file_id, dataset_id,dataspace_id;  /* identifiers */
   herr_t      status;
   int         i, j;

   /* Initialize the dataset. */

   char* dset_data;

   dset_data=(char*)malloc(sizeof(char)*DIM*DIM);
   for (i = 0; i < DIM*DIM; i++)
         dset_data[i] = 250;
   /* Open an existing file. */
   file_id = H5Fopen(file, H5F_ACC_RDWR, H5P_DEFAULT);

   /* Open an existing dataset. */
   dataset_id = H5Dopen2(file_id, "/out", H5P_DEFAULT);

   dataspace_id = H5Dget_space(dataset_id);

   /* Write the dataset. */
   status = H5Dwrite(dataset_id, H5T_NATIVE_CHAR, H5S_ALL, dataspace_id, H5P_DEFAULT,dset_data);

   /*status = H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
                    dset_data);*/

   printf("ok\n");
   /* Close the dataset. */
   status = H5Dclose(dataset_id);

   /* Close the file. */
   status = H5Fclose(file_id);
}