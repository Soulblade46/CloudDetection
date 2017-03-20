#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM 3712

#define FILE        "subset.h5"
#define DATASETNAME "IntArray" 
#define RANK  2

#define DIM0_SUB  3                         /* subset dimensions */ 
#define DIM1_SUB  4 


#define DIM0     3712                         /* size of dataset */       
#define DIM1     3712

void add_attribute_string(int length)
{
	hid_t ss = H5Tcopy(H5T_C_S1);
	H5Tset_size(ss, length);
}

#if 0
void printh5(int* matrix)
{
    hsize_t     dims[2], dimsm[2],dim_attr1[1],dim_attr2[1];   
    //int         data[DIM0][DIM1];           /* data to write */
    //int** data;
    //int         sdata[DIM0_SUB][DIM1_SUB];  /* subset to write */
    //int         rdata[DIM0][DIM1];          /* buffer for read */
    //int** rdata;
 
    hid_t       file_id, dataset_id,attribute1_id,attribute2_id;        /* handles */
    hid_t       dataspace_id,dataspace_attr1_id,dataspace_attr2_id ,memspace_id; 

    herr_t      status;                             
   
    hsize_t     count[2];              /* size of subset in the file */
    hsize_t     offset[2];             /* subset offset in the file */
    hsize_t     stride[2];
    hsize_t     block[2];
    int         i, j;

    
    /*****************************************************************
     * Create a new file with default creation and access properties.*
     * Then create a dataset and write data to it and close the file *
     * and dataset.                                                  *
     *****************************************************************/
    int (*data)[DIM] = malloc(sizeof(int[DIM][DIM]));
    int (*rdata)[DIM] = malloc(sizeof(int[DIM][DIM]));
    file_id = H5Fcreate (FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    dims[0] = DIM0;
    dims[1] = DIM1;
    dim_attr1[0]=1;
    dim_attr2[0]=1;
    dataspace_id = H5Screate_simple (RANK, dims, NULL); 

    dataset_id = H5Dcreate2 (file_id, DATASETNAME, H5T_STD_I32BE, dataspace_id,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    dataspace_attr1_id=H5Screate_simple(1, dim_attr1, NULL);

    hid_t ss5 = H5Tcopy(H5T_C_S1);
    H5Tset_size(ss5, 5);
    #if 1
    attribute1_id=H5Acreate2(dataset_id, "CLASS", ss5, dataspace_attr1_id, H5P_DEFAULT,H5P_DEFAULT);
   H5Awrite(attribute1_id, ss5, "IMAGE");
   #else
    attribute1_id=H5Acreate2(dataset_id, "CLASS",H5T_STRING, dataspace_attr1_id, H5P_DEFAULT,H5P_DEFAULT);
   H5Awrite(attribute1_id, H5T_STRING, "IMAGE");
   #endif

   hid_t ss3 = H5Tcopy(H5T_C_S1);
   H5Tset_size(ss3, 3);
   #if 1
   attribute2_id=H5Acreate2(dataset_id, "IMAGE_VERSION",ss3, dataspace_attr1_id, H5P_DEFAULT,H5P_DEFAULT);
   H5Awrite(attribute2_id, ss3, "1.2");
   #else
   attribute2_id=H5Acreate2(dataset_id, "IMAGE_VERSION",H5T_STRING, dataspace_attr2_id, H5P_DEFAULT,H5P_DEFAULT);
   H5Awrite(attribute2_id, H5T_STRING, "1.2");
   #endif

   /* for (j = 0; j < DIM0; j++) {
   for (i = 0; i < DIM1; i++)
            if (i< (DIM1/2))
          data[j][i] = 1;
            else
               data[j][i] = 0;
    }*/     

    #if 0
    status = H5Dwrite (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, data);
    #else
    status = H5Dwrite (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, matrix);
    #endif

    printf ("\nData Written to File:\n");
    #if 0
    for (i = 0; i<DIM0; i++){
       for (j = 0; j<DIM1; j++)
           printf (" %i", data[i][j]);
       printf ("\n");
    }
    #endif
    status = H5Aclose (attribute1_id);
    status = H5Aclose (attribute2_id);
    status = H5Sclose (dataspace_id);
    status = H5Dclose (dataset_id);
    status = H5Fclose (file_id);
}
#else
	void printh5(unsigned short* matrix)
{
    hsize_t     dims[2], dimsm[2],dim_attr1[1],dim_attr2[1];   
    //int         data[DIM0][DIM1];           /* data to write */
    //int** data;
    //int         sdata[DIM0_SUB][DIM1_SUB];  /* subset to write */
    //int         rdata[DIM0][DIM1];          /* buffer for read */
    //int** rdata;
 
    hid_t       file_id, dataset_id,attribute1_id,attribute2_id;        /* handles */
    hid_t       dataspace_id,dataspace_attr1_id,dataspace_attr2_id ,memspace_id; 

    herr_t      status;                             
   
    hsize_t     count[2];              /* size of subset in the file */
    hsize_t     offset[2];             /* subset offset in the file */
    hsize_t     stride[2];
    hsize_t     block[2];
    int         i, j;

    
    /*****************************************************************
     * Create a new file with default creation and access properties.*
     * Then create a dataset and write data to it and close the file *
     * and dataset.                                                  *
     *****************************************************************/
    double (*data)[DIM] = malloc(sizeof(int[DIM][DIM]));
    double (*rdata)[DIM] = malloc(sizeof(int[DIM][DIM]));
    file_id = H5Fcreate (FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    dims[0] = DIM0;
    dims[1] = DIM1;
    dim_attr1[0]=1;
    dim_attr2[0]=1;
    dataspace_id = H5Screate_simple (RANK, dims, NULL); 

    dataset_id = H5Dcreate2 (file_id, DATASETNAME, H5T_NATIVE_USHORT, dataspace_id,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    dataspace_attr1_id=H5Screate_simple(1, dim_attr1, NULL);

    hid_t ss5 = H5Tcopy(H5T_C_S1);
    H5Tset_size(ss5, 5);
    #if 1
    attribute1_id=H5Acreate2(dataset_id, "CLASS", ss5, dataspace_attr1_id, H5P_DEFAULT,H5P_DEFAULT);
   H5Awrite(attribute1_id, ss5, "IMAGE");
   #else
    attribute1_id=H5Acreate2(dataset_id, "CLASS",H5T_STRING, dataspace_attr1_id, H5P_DEFAULT,H5P_DEFAULT);
   H5Awrite(attribute1_id, H5T_STRING, "IMAGE");
   #endif

   hid_t ss3 = H5Tcopy(H5T_C_S1);
   H5Tset_size(ss3, 3);
   #if 1
   attribute2_id=H5Acreate2(dataset_id, "IMAGE_VERSION",ss3, dataspace_attr1_id, H5P_DEFAULT,H5P_DEFAULT);
   H5Awrite(attribute2_id, ss3, "1.2");
   #else
   attribute2_id=H5Acreate2(dataset_id, "IMAGE_VERSION",H5T_STRING, dataspace_attr2_id, H5P_DEFAULT,H5P_DEFAULT);
   H5Awrite(attribute2_id, H5T_STRING, "1.2");
   #endif

   /* for (j = 0; j < DIM0; j++) {
   for (i = 0; i < DIM1; i++)
            if (i< (DIM1/2))
          data[j][i] = 1;
            else
               data[j][i] = 0;
    }*/     

    #if 0
    status = H5Dwrite (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, data);
    #else
    status = H5Dwrite (dataset_id, H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, matrix);
    #endif

    printf ("\nData Written to File:\n");
    #if 0
    for (i = 0; i<DIM0; i++){
       for (j = 0; j<DIM1; j++)
           printf (" %i", data[i][j]);
       printf ("\n");
    }
    #endif
    status = H5Aclose (attribute1_id);
    status = H5Aclose (attribute2_id);
    status = H5Sclose (dataspace_id);
    status = H5Dclose (dataset_id);
    status = H5Fclose (file_id);
}
#endif