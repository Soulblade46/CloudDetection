#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

#define L15_PATH "U-MARF/MSG/Level1.5/"

struct calibration
{
    double slope[12];
    double offset[12];
};

const  char* fname= "img.h5";
hid_t  fid;


int main()
{
    char *ChannelPath[] = {
    	L15_PATH "DATA/Channel 04/IMAGE_DATA",
    	L15_PATH "DATA/Channel 09/IMAGE_DATA",
    };

    #define META_PATH L15_PATH "METADATA/"
    #define HEADER_PATH META_PATH "HEADER/"

    char * AttributePath   = HEADER_PATH "RadiometricProcessing/Level15ImageCalibration_ARRAY";
    char * DescriptionPath = HEADER_PATH "ImageDescription/ImageDescription_DESCR";
    char * SubsetPath = META_PATH "SUBSET";



    hid_t dataset, dataspace;
    hssize_t num_entries;
    herr_t status;
    struct calibration* cal;
    hid_t h5cal_t = H5Tcreate(H5T_COMPOUND, sizeof(struct calibration));

    // Read fulldisk lines and columns from image description table
    fid = H5Fopen(fname , H5F_ACC_RDONLY , H5P_DEFAULT );
    dataset = H5Dopen2(fid, AttributePath, H5P_DEFAULT);

    dataspace = H5Dget_space(dataset);
    H5Sget_simple_extent_ndims(dataspace);

    num_entries = H5Sget_simple_extent_npoints(dataspace);

    cal=(struct calibration*)malloc(sizeof(struct calibration)*num_entries);

    status = H5Dread(dataset, h5cal_t, H5S_ALL, dataspace, H5P_DEFAULT, cal);
   
    H5Dclose(dataset);

    // channel numbering starts at 1, array indices at 0
    //data.ch04.calibration = cal[3];
    //data.ch09.calibration = cal[8];


}